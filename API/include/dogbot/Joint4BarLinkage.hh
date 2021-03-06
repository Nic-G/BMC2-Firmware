#ifndef DOGBOG_JOINT4BARLINKAGE_HEADER
#define DOGBOG_JOINT4BARLINKAGE_HEADER 1

#include "dogbot/JointRelative.hh"
#include <memory>
#include "dogbot/LegKinematics.hh"

namespace DogBotN {

  //! Virtual joint.
  //! These give a simplified view of the robot.

  class Joint4BarLinkageC
   : public JointRelativeC
  {
  public:
    typedef std::chrono::time_point<std::chrono::steady_clock,std::chrono::duration< double > > TimePointT;

    Joint4BarLinkageC();

    //! Constructor
    Joint4BarLinkageC(std::shared_ptr<JointC> &jointDrive,std::shared_ptr<JointC> &jointRef);

    //! Type of joint
    virtual std::string JointType() const override;

    //! Configure from JSON
    virtual bool ConfigureFromJSON(DogBotAPIC &api,const Json::Value &value) override;

    //! Get the servo configuration as JSON
    virtual Json::Value ConfigAsJSON() const override;

  protected:
    virtual bool Raw2Simple(
        float refPosition,float refVelocity,float refTorque,
        float drivePosition,float driveVelocity,float driveTorque,
        double &position,double &velocity,double &torque
    ) const override;

    virtual bool Simple2Raw(
         float refPosition,float refTorque,
         float position,float torque,
         double &drivePosition,double &driveTorque
         ) const override;


    std::shared_ptr<LegKinematicsC> m_legKinematics; //!< Kinematics for linkage
  };

}

#endif
