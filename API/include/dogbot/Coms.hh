#ifndef DOGBOG_COMS_HEADER
#define DOGBOG_COMS_HEADER 1

#include <sstream>

#include <thread>
#include <vector>
#include <functional>
#include <future>
#include <assert.h>
#include <mutex>
#include <string.h>

#include "dogbot/protocol.h"
#include <spdlog/spdlog.h>
#include <cstdint>

namespace DogBotN {

  class ComsC;

  //! handle containing call back information

  class ComsCallbackHandleC
  {
  public:
    ComsCallbackHandleC()
    {}

    //! Access packet type
    ComsPacketTypeT PacketType() const
    { return m_packetType; }

    int Id() const
    { return m_id; }

  protected:
    ComsCallbackHandleC(ComsPacketTypeT packetType,int id)
      : m_packetType(packetType),
        m_id(id)
    {}

    ComsPacketTypeT m_packetType = CPT_NoOp;
    int m_id = -1;

    friend class ComsC;
  };

  //! Set of callbacks to de-register when class is destroyed
  //! This manages resource allocation and provides similar
  //! exception safety to 'resource acquisition is initialisation' variable.

  class ComsRegisteredCallbackSetC
  {
  public:
    //! Construct from 'coms' object we're registering callbacks from.
    ComsRegisteredCallbackSetC(std::shared_ptr<ComsC> &coms);

    //! Destructor
    ~ComsRegisteredCallbackSetC();

    //! Access associated communication object.
    ComsC &Coms()
    { return *m_coms; }

    //! \param cb A callback to be added to the list.
    void Push(const ComsCallbackHandleC &cb);

    //! Helper function for adding new functions
    //! \param packetType Type of packet to handle reports for
    //! \param handler Handler to call
    void SetHandler(ComsPacketTypeT packetType,const std::function<void (uint8_t *data,int len)> &handler);

    //! De-register all currently registered callbacks.
    void PopAll();

  protected:
    std::shared_ptr<ComsC> m_coms;
    std::vector<ComsCallbackHandleC> m_callbacks;
  };

  //! Low level communication interface

  class ComsC
  {
  public:
    ComsC(std::shared_ptr<spdlog::logger> &log);

    //! default
    ComsC();

    //! Destructor
    // Disconnects and closes file descriptors
    virtual ~ComsC();

    //! Set the logger to use
    void SetLogger(const std::shared_ptr<spdlog::logger> &log);

    //! Open a port.
    virtual bool Open(const std::string &portAddr);

    //! Close connection
    virtual void Close();

    //! Is connection ready ?
    virtual bool IsReady() const;

    //! Process received packet.
    void ProcessPacket(uint8_t *data,int len);

    //! Send packet
    virtual void SendPacket(const uint8_t *data,int len);

    //! Send an emergency stop
    void SendEmergencyStop();

    //! Send a move command with a current limit.
    void SendMoveWithEffort(int deviceId,float pos,float currentLimit,enum PositionReferenceT posRef);

    //! Send velocity command with a current limit.
    void SendVelocityWithEffort(int deviceId,float velocity,float currentLimit);

    //! Send torque to apply as a motor current.
    void SendTorque(int deviceId,float current);

    //! Set a parameter synchronous, method will not return until parameter
    //! has been confirmed set or an error occurred.
    template<typename ParamT>
    bool SetParam(int deviceId,ComsParameterIndexT param,ParamT value)
    {
      using Ms = std::chrono::milliseconds;
      bool ret = false;

      PacketParam8ByteC msg;
      msg.m_header.m_packetType = CPT_SetParam;
      msg.m_header.m_deviceId = deviceId;
      msg.m_header.m_index = (uint16_t) param;
      memcpy(msg.m_data.uint8, &value,sizeof(value));

      std::promise<bool> promiseDone;
      std::future<bool> done = promiseDone.get_future();

      auto handler = SetHandler(CPT_ReportParam,[this,deviceId,param,&msg,&ret,&promiseDone](uint8_t *data,int len) mutable {
        if(len < sizeof(PacketParamHeaderC)) {
          m_log->error("Short ReportParam packet received. {} Bytes ",len);
          return ;
        }
        PacketParam8ByteC *pkt = reinterpret_cast<PacketParam8ByteC *>(data);
        // Is it from the device we're interested in
        if(pkt->m_header.m_deviceId != deviceId)
          return ;
        if(pkt->m_header.m_index != param)
          return ;
        if(len != sizeof(msg.m_header)+sizeof(value)) {
          m_log->error("Unexpected reply size {}, when {} bytes were sent for parameter {} ",len,sizeof(msg.m_header)+sizeof(value),(int) param);
          promiseDone.set_value(false);
          return ;
        }
        if(memcmp(msg.m_data.uint8,pkt->m_data.uint8,sizeof(value)) == 0) {
          if(!ret) {
            ret = true;
            promiseDone.set_value(true);
          }
        } else {
          m_log->warn("Unexpected value returned. ");
        }
      });
      assert(sizeof(value) <= 7);
      if(sizeof(value) > 7) {
        //! Delete given handler
        DeleteHandler(handler);
        m_log->error("Parameter {} too large.",(int) param);
        return false;
      }
      for(int i = 0;i < 4 && !ret;i++) {
        // Send data.
        SendPacket((uint8_t*) &msg,sizeof(msg.m_header)+sizeof(value));
        if(done.wait_for(Ms(250)) == std::future_status::ready) {
          break;
        }
      }

      //! Delete given handler
      DeleteHandler(handler);

      return ret;
    }

    //! Set a parameter
    void SendSetParam(int deviceId,ComsParameterIndexT param,uint8_t value);

    //! Set a parameter
    void SendSetParam(int deviceId,ComsParameterIndexT param,int value);

    //! Set a parameter
    void SendSetParam(int deviceId,ComsParameterIndexT param,float value);

    //! Set a parameter
    void SendSetParam(int deviceId,ComsParameterIndexT param,BufferTypeT &buff,int len);

    //! Query a parameter
    void SendQueryParam(int deviceId,ComsParameterIndexT param);

    //! Send query devices message
    void SendQueryDevices();

    //! Set a device id
    void SendSetDeviceId(uint8_t deviceId,uint32_t uid0,uint32_t uid1);

    //! Send a sync message
    void SendSync();

    //! Send a ping
    void SendPing(int deviceId);

    //! Send an enable bridge mode
    void SendEnableBridge(bool enable);

    //! Send a calibration zero
    void SendCalZero(int deviceId);

    //! Set handler for all packets, this is called as well as any specific handlers that have been installed.
    //! Only one can be set at any time.
    int SetGenericHandler(const std::function<void (uint8_t *data,int len)> &handler);

    //! Remove generic handler
    void RemoveGenericHandler(int id);

    //! Set the handler for a particular type of packet.
    //! Returns the id of the handler or -1 if failed.
    ComsCallbackHandleC SetHandler(ComsPacketTypeT packetType,const std::function<void (uint8_t *data,int len)> &handler);

    //! Remove given handler
    void DeleteHandler(const ComsCallbackHandleC &handle);

    //! Convert a report value to an angle in radians
    static float PositionReport2Angle(int16_t val)
    { return val * M_PI * 4.0/ 65535.0; }

    //! Convert a report value to a torque
    static float TorqueReport2Current(int16_t val)
    { return val * 10.0/ 65535.0; }

  protected:

    volatile bool m_terminate = false;

    std::shared_ptr<spdlog::logger> m_log = spdlog::get("console");

    std::mutex m_accessPacketHandler;

    std::vector<std::vector<std::function<void (uint8_t *data,int len)> > > m_packetHandler;
    std::vector<std::function<void (uint8_t *data,int len)> > m_genericHandler;
  };


}
#endif
