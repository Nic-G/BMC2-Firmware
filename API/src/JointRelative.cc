
#include <string>
#include "dogbot/JointRelative.hh"
#include "dogbot/DogBotAPI.hh"

namespace DogBotN {

  //! Default constructor
  JointRelativeC::JointRelativeC()
  {}

  //! Constructor
  JointRelativeC::JointRelativeC(std::shared_ptr<JointC> &jointDrive,std::shared_ptr<JointC> &jointRef)
    : m_jointDrive(jointDrive),
      m_jointRef(jointRef)
  {}

  //! Type of joint
  std::string JointRelativeC::JointType() const
  {
    return "relative";
  }

  bool JointRelativeC::Raw2Simple(
      float refPosition,float refVelocity,float refTorque,
      float drivePosition,float driveVelocity,float driveTorque,
      double &position,double &velocity,double &torque
  ) const
  {
    position = drivePosition - (refPosition * m_refGain + m_refOffset);
    velocity = driveVelocity - refVelocity;
    torque = driveTorque;
    return true;
  }

  bool JointRelativeC::Simple2Raw(
       float refPosition,float refTorque,
       float position,float torque,
       double &drivePosition,double &driveTorque
  ) const
  {
    drivePosition = position + (refPosition * m_refGain + m_refOffset);
    driveTorque = torque;
    return true;
  }

  //! Configure from JSON
  bool JointRelativeC::ConfigureFromJSON(
      DogBotAPIC &api,
      const Json::Value &value
      )
  {
    if(!JointC::ConfigureFromJSON(api,value))
      return false;
    std::string jointRefName = value.get("jointRef","").asString();
    if(!jointRefName.empty()) {
      m_jointRef = api.GetJointByName(jointRefName);
      if(!m_jointRef) {
        api.Log().error("Failed to find servo '{}' ",jointRefName);
        return false;
      }
    }
    std::string jointDriveName = value.get("jointDrive","").asString();
    if(!jointDriveName.empty()) {
      m_jointDrive = api.GetJointByName(jointDriveName);
      if(!m_jointDrive) {
        api.Log().error("Failed to find servo '{}' ",jointDriveName);
        return false;
      }
    }
    m_refGain = value.get("refGain",1.0).asFloat();
    m_refOffset = value.get("refOffset",0.0).asFloat();
    return true;
  }

  //! Get the servo configuration as JSON
  Json::Value JointRelativeC::ConfigAsJSON() const
  {
    Json::Value ret = JointC::ConfigAsJSON();

    if(m_jointRef)
      ret["jointRef"] = m_jointRef->Name();
    if(m_jointDrive)
      ret["jointDrive"] = m_jointDrive->Name();
    ret["refGain"] = m_refGain;
    ret["refOffset"] = m_refOffset;
    return ret;
  }

  //! Get last reported state of the servo and the time it was taken.
  bool JointRelativeC::GetState(TimePointT &tick,double &position,double &velocity,double &torque) const
  {
    double drivePosition = 0;
    double driveVelocity = 0;
    double driveTorque = 0;

    if(!m_jointDrive->GetState(tick,drivePosition,driveVelocity,driveTorque))
      return false;

    double refPosition = 0;
    double refVelocity = 0;
    double refTorque = 0;

    if(!m_jointRef->GetStateAt(tick,refPosition,refVelocity,refTorque))
      return false;

    Raw2Simple(refPosition,refVelocity,refTorque,
               drivePosition,driveVelocity,driveTorque,
               position,velocity,torque
               );

    return true;
  }

  //! Estimate state at the given time.
  //! This will linearly extrapolate position, and assume velocity and torque are
  //! the same as the last reading.
  //! If the data is more than 5 ticks away from the
  bool JointRelativeC::GetStateAt(TimePointT tick,double &position,double &velocity,double &torque) const
  {
    double drivePosition = 0;
    double driveVelocity = 0;
    double driveTorque = 0;

    if(!m_jointDrive->GetState(tick,drivePosition,driveVelocity,driveTorque)) {
      return false;
    }

    double refPosition = 0;
    double refVelocity = 0;
    double refTorque = 0;

    if(!m_jointRef->GetStateAt(tick,refPosition,refVelocity,refTorque)) {
      return false;
    }

    return Raw2Simple(refPosition,refVelocity,refTorque,
               drivePosition,driveVelocity,driveTorque,
               position,velocity,torque
               );
  }

  //! Update torque for the servo.
  bool JointRelativeC::DemandTorque(float torque)
  {
    return m_jointDrive->DemandTorque(torque);
  }

  //! Demand a position for the servo
  bool JointRelativeC::DemandPosition(float position,float torqueLimit)
  {
    m_demandPosition = position;
    m_demandTorqueLimit = torqueLimit;

    double refPosition = 0;
    double refVelocity = 0;
    double refTorque = 0;

    TimePointT tick = TimePointT::clock::now();
    if(!m_jointRef->GetStateAt(tick,refPosition,refVelocity,refTorque)) {
      return false;
    }

    double drivePosition = 0;
    double driveTorqueLimit = 0;
    if(!Simple2Raw(
        refPosition,refTorque,
        position,torqueLimit,
        drivePosition,driveTorqueLimit)) {
      return false;
    }

    m_demandPosition = position;
    m_demandTorqueLimit = torqueLimit;

    m_jointDrive->DemandPosition(drivePosition,driveTorqueLimit);

    return true;
  }

}
