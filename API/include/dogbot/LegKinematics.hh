#ifndef DOGBOT_LEGKINEMATICS_HEADER
#define DOGBOT_LEGKINEMATICS_HEADER 1

#include <jsoncpp/json/json.h>

namespace DogBotN {


  //! This computes the position of the end effector given the end effector

  // l1 = Upper leg length.
  // l2 = Lower leg length

  // Coordinates:
  //  x = forward/back
  //  y = sideways
  //  z = Height above ground
  //
  // Angles:
  //  0 - Roll
  //  1 - Pitch
  //  2 - Knee
  //
  // In the 4 bar linkage code the following variables are used:
  //   theta - Servo position
  //   psi - Joint angle

  class LegKinematicsC
  {
  public:
    //! Default constructor
    LegKinematicsC();

    //! Construct from a json object
    LegKinematicsC(const Json::Value &value);

    //! Create
    LegKinematicsC(float l1,float l2);

    //! Access name of leg.
    const std::string &Name() const
    { return m_name; }

    //! Set name of leg.
    void SetName(const std::string &name)
    { m_name = name; }

    //! Configure from JSON
    bool ConfigureFromJSON(const Json::Value &value);

    //! Get the servo configuration as JSON
    Json::Value ConfigAsJSON() const;

    //! Compute the joint angles given a location.
    bool Inverse(const float (&position)[3],float (&angles)[3]) const;

    //! Forward kinematics for the leg.
    bool Forward(const float (&angles)[3],float (&position)[3]) const;

    //! 4 bar linkage angle forward.
    //! Servo to achieved joint angle
    //! theta - Servo position
    //! Returns psi, the knee joint angle.
    //! solution2 - Gives the alternate solution, this shouldn't be required as the robot's possible angles are physically limited.
    float Linkage4BarForward(float theta,bool solution2 = false) const;

    //! 4 bar linkage angle backward,
    //! Joint angle to required servo position.
    //! psi - Required knee joint position
    //! theta - Variable to store the servo position
    //! solution2 - There are two possible solutions, changing this from true to false switches between them.
    // Returns true if angle exists, false if it is unreachable
    bool Linkage4BarBack(float psi,float &theta,bool solution2 = false) const;

    //! See: https://synthetica.eng.uci.edu/mechanicaldesign101/McCarthyNotes-2.pdf
    bool Linkage4Bar(float theta,float a,float b,float g,float h,float &psi,bool solution2 = false) const;

    //! Compute the speed ratio at the given input angle. This can also be used to convert torques
    //! theta - Servo position
    //! psi - Joint angle
    float LinkageSpeedRatio(float theta,float psi) const;

  protected:
    std::string m_name; // Leg name

    float m_l1 = 0.361; // Upper leg length
    float m_l2 = 0.31;  // Lower leg length
    float m_zoff = 0.08;// Vertical offset from central axis

    float m_linkA = 0.032; // Hip CAD: 0.032
    float m_linkB = 0.04;  // Knee CAD: 0.04
    float m_linkH = 0.363; // Push rod 0.363
  };

}

#endif
