
#include <sstream>

#include <thread>
#include <vector>
#include <functional>
#include <assert.h>
#include <mutex>

#include <stdio.h>

#include <fstream>
#include <iostream>
#include <fcntl.h>

#include "coms.hh"
#include <unistd.h>

#define ROS_DEBUG printf
#define ROS_ERROR printf

namespace DogBotN
{

  SerialComsC::SerialComsC(const char *portAddr)
  {
    Open(portAddr);
  }

  //! default
  SerialComsC::SerialComsC()
  {}

  // Disconnects and closes file descriptors
  SerialComsC::~SerialComsC()
  {
    if(m_fd >= 0)
      Close();
  }

  //! Close connection
  void SerialComsC::Close()
  {
    if(m_fd >= 0)
      close(m_fd);
    m_fd = -1;
  }

  bool SerialComsC::Open(const char *portAddr)
  {
    if(m_fd >= 0) {
      std::cerr << "Coms already open. " << std::endl;
      return false;
    }

    std::cerr << "Opening: '" << portAddr << "' " << std::endl;
    m_fd = open(portAddr,O_RDWR | O_NONBLOCK);
    std::cerr << "Ret " << m_fd << std::endl;
    if(m_fd < 0) {
      std::cerr << "Failed to open file " << portAddr << " " << std::endl;
      perror("Failed to open file ");
      return false;
    }
    std::cerr << "Port opened ok  '" << portAddr << "' " << std::endl;

    m_threadRecieve = std::move(std::thread { [this]{ RunRecieve(); } });
    return true;
  }


  void SerialComsC::SetHandler(ComsPacketTypeT packetId,const std::function<void (uint8_t *data,int )> &handler)
  {
    std::lock_guard<std::mutex> lock(m_accessPacketHandler);
    assert(packetId < 256);

    while(m_packetHandler.size() <= (int) packetId) {
      m_packetHandler.push_back(std::function<void (uint8_t *data,int )>());
    }
    m_packetHandler[(int) packetId] = handler;
  }

  void SerialComsC::AcceptByte(uint8_t sendByte)
  {
    switch(m_state)
    {
    case 0: // Wait for STX.
      if(sendByte == m_charSTX)
        m_state = 1;
      // Else remain in state 0.
      break;
    case 1: // Packet length.
      m_packetLen = sendByte;
      if(m_packetLen > 64) {
        printf("Packet to long! %d  ",m_packetLen);
        if(sendByte != m_charSTX)
          m_state = 0;
        break;
      }

      m_at = 0;
      m_checkSum = 0x55 + m_packetLen;
      m_state = 2;
      break;
    case 2: // Data
      m_checkSum += sendByte;
      m_data[m_at] = sendByte;
      m_at++;
      if(m_at >= m_packetLen)
        m_state = 3;
      break;
    case 3: { // CRC 1
      uint8_t cs1 = (m_checkSum & 0xff);
      //ROS_DEBUG("Checksum1 : %d %d ",(int)  cs1 , (int) sendByte);
      if(cs1 != sendByte) {
        //ROS_DEBUG("Checksum 1 failed. ");
        if(sendByte == m_charSTX)
          m_state = 1;
        else
          m_state = 0;
        break;
      }

      m_state = 4;
    } break;
    case 4: { // CRC 1
      uint8_t cs2 = ((m_checkSum >> 8) & 0xff);
      //ROS_DEBUG("Checksum2 : %d %d ",(int) ((m_checkSum >> 8) & 0xff) , (int) sendByte);
      if(cs2 != sendByte) {
        //std::cerr << "Checksum 2 failed. \n";
        if(sendByte == m_charSTX)
          m_state = 1;
        else
          m_state = 0;
        break;
      }

      m_state = 5;
    } break;
    case 5: // ETX.
      if(sendByte == m_charETX) {
        ProcessPacket();
      } else {
        std::cerr << "Packet corrupted. Len " << m_packetLen << " Type " <<  (int) m_data[0] << "\n";
        if(sendByte == m_charSTX) {
          m_state = 1;
          break;
        }
      }
      m_state = 0;
      break;
    }
  }

  //! Process received packet.
  void SerialComsC::ProcessPacket()
  {
    std::string data;
#if 0
    for(int i = 0;i < m_packetLen;i++) {
      data += std::to_string(m_data[i]) + " ";
    }
    ROS_DEBUG("Got packet [%d] %s ",m_packetLen,data.c_str());
#endif

    // m_data[0] //
    int packetId = m_data[0];
    switch(packetId)
    {
      case CPT_Ping: {
        uint8_t data[64];
        int at = 0;
        data[at++] = CPT_Pong;
        SendPacket(data,at);
      } break;
      case CPT_Sync:
        std::cerr << "Got sync. \n";
        break;
      default: {
        std::lock_guard<std::mutex> lock(m_accessPacketHandler);
        if(packetId < m_packetHandler.size()) {
          const std::function<void (uint8_t *data,int )> &handler = m_packetHandler[packetId];
          if(handler) {
            handler(m_data,m_packetLen);
            return ;
          }
        }
        // Fall back to the default handlers.
        switch(packetId) {
          case CPT_Pong:
            std::cerr << "Got pong. \n";
            break;
          case CPT_Error:
            ROS_ERROR("Received error code: %d  Arg:%d ",(int) m_data[1],(int) m_data[2]);
            break;
          default:
            ROS_DEBUG("Don't know how to handle packet %d (Of %d) ",packetId,(int) m_packetHandler.size());
        }
      } break;
  #if 0
    case 2: { // ADC Data.
      int current = ((int) m_data[2])  + (((int) m_data[3]) << 8);
      int volt = ((int) m_data[4])  + (((int) m_data[5]) << 8);
      ROS_DEBUG("I:%4d V:%4d  Phase:%d  PWM:%d ",current,volt,(int) m_data[6],(int) m_data[7]);
    } break;
    default:
      ROS_DEBUG("Unexpected packet type %d ",(int) m_data[1]);
      break;
  #endif
    }
  }

  bool SerialComsC::RunRecieve()
  {
    std::cerr << "Running receiver" << std::endl;
    assert(m_fd >= 0);
    uint8_t readBuff[1024];
    fd_set localFds;
    FD_ZERO(&localFds);

    while(!m_terminate && m_fd >= 0) {
      FD_SET(m_fd,&localFds);
      int x = select(m_fd+1,&localFds,0,0,0);
      if(x < 0) {
        perror("Failed to select");
        continue;
      }
      // FIXME:- Read into a larger buffer.
      int n = read(m_fd,readBuff,sizeof readBuff);
      if(n == 0)
        continue;
      if(n < 0) {
        if(errno == EAGAIN ||
            errno == EINTR
            )
          continue;
        perror("Failed to read data.");
        continue;
      }
#if 0
      ROS_DEBUG("Got %d bytes: ",n);
      for(int i = 0;i < n;i++) {
        ROS_DEBUG("%02X ",(int) readBuff[i]);
        AcceptByte(readBuff[i]);
      }
      ROS_DEBUG("\n");
#else
      for(int i = 0;i < n;i++) {
        AcceptByte(readBuff[i]);
      }
#endif
    }
    std::cerr << "Exiting receiver " << m_fd << std::endl;

    return true;
  }

  //! Send packet
  void SerialComsC::SendPacket(const uint8_t *buff,int len)
  {
    assert(m_fd >= 0);

    std::vector<uint8_t> packet;
    packet.reserve(len + 6);


    packet.push_back(m_charSTX);
    int crc = len + 0x55;
    packet.push_back(len);
    for(int i =0;i < len;i++) {
      uint8_t data = buff[i];
      packet.push_back(data);
      crc += data;
    }
    packet.push_back(crc);
    packet.push_back(crc >> 8);
    packet.push_back(m_charETX);

    std::lock_guard<std::mutex> lock(m_accessTx);
    int at = 0;
    fd_set localFds;
    FD_ZERO(&localFds);
    FD_SET(m_fd,&localFds);

    while(at < packet.size()) {
      int x = select(m_fd+1,0,&localFds,0,0);
      if(x < 0) {
        perror("write select");
        break;
      }
      int n = write(m_fd,&packet[at],packet.size());
      if(n < 0) {
        if(errno == EAGAIN || errno == EINTR)
          continue;
      }
      at += n;
    }
  }


  //! Send a move command
  void SerialComsC::SendMove(int servoId,int pos)
  {
    uint8_t data[64];

    int at = 0;
    data[at++] = CPT_Servo;
    data[at++] = servoId;
    data[at++] = (int) pos;

    SendPacket(data,at);
  }


  //! Set a parameter
  void SerialComsC::SendSetParam(ComsParameterIndexT param,uint16_t value)
  {
    PacketSetParamC msg;
    msg.m_packetType = CPT_SetParam;
    msg.m_index = (uint16_t) param;
    msg.m_data = value;
    SendPacket((uint8_t*) &msg,sizeof(msg));
  }


  //! Send a move command
  void SerialComsC::SendMoveWithEffort(float pos,float effort)
  {
    int at = 0;

    struct PacketServoC servoPkt;
    servoPkt.m_packetType = CPT_Servo;

    while(pos < 0)
      pos += 3.14159265359;
    servoPkt.m_position = pos * 65535.0 / (2.0 * 3.14159265359);
    servoPkt.m_torqueLimit = effort * 65535.0 / (10.0);

    SendPacket((uint8_t *)&servoPkt,sizeof servoPkt);
  }


  //! Send a move command
  void SerialComsC::SendPing()
  {
    uint8_t data[64];

    int at = 0;
    data[at++] = CPT_Ping;

    SendPacket(data,at);
  }
}