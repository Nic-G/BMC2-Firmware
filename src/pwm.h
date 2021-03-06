#ifndef PWM_HEADER
#define PWM_HEADER 1

#ifdef __cplusplus
extern "C" {
#endif

#include "ch.h"
#include "hal_streams.h"
#include "bmc.h"

int InitPWM(void);

int PWMRun(void);
int PWMStop(void);

void EnableSensorPower(bool enable);
bool HasSensorPower(void);
void EnableFanPower(bool enable);

enum FaultCodeT PWMSelfTest(void);
enum FaultCodeT PWMMotorCal(void);

int PWMCal(BaseSequentialStream *chp);
int PWMCalSVM(BaseSequentialStream *chp);
int PWMSVMScan(BaseSequentialStream *chp);

void PWMUpdateDrive(int phase,int power);

void MotionStep(void);

float hallToAngle(uint16_t *sensors);

extern binary_semaphore_t g_adcInjectedDataReady;
extern binary_semaphore_t g_reportSampleReady;  //! ~100Hz report loop

extern int g_motorReportSampleRate;

extern uint32_t g_faultState;
extern int16_t g_currentADCValue[3];
extern float g_currentZeroOffset[3];
extern uint16_t g_hall[3];
extern int g_adcInjCount;
extern int g_pwmTimeoutCount ;

extern volatile bool g_pwmRun;
extern bool g_pwmThreadRunning;
extern bool g_pwmFullReport; //! If true generate messages detailing PWM state.

extern float g_maxSupplyVoltage;
extern float g_maxOperatingTemperature;

extern float g_driveTemperature;
extern float g_motorTemperature;

extern float g_maxCurrentSense;

#define g_calibrationPointCount (18)

extern int g_phaseAngles[g_calibrationPointCount][3];

extern int g_adcTickCount;

extern float g_vbus_voltage;
extern float g_minSupplyVoltage;

extern bool g_gateDriverWarning;
extern bool g_gateDriverFault;

extern float g_current[3];
extern float g_phaseAngle;
extern float g_current_Ibus;

extern float g_demandPhasePosition;
extern float g_demandPhaseVelocity;
extern float g_demandTorque;

extern float g_velocityLimit;
extern float g_velocityPGain;
extern float g_velocityIGain;
extern float g_velocityFilter;
extern float g_positionGain;
extern float g_positionIGain;
extern float g_positionIClamp;

extern float g_currentLimit;

extern int g_phaseRotationCount;
extern float g_currentPhasePosition;
extern float g_currentPhaseVelocity;
extern float g_torqueAverage;

extern float g_motor_p_gain;
extern float g_motor_i_gain;

extern float g_Id;
extern float g_Iq;
extern float g_Ierr_d;
extern float g_Ierr_q;

extern float g_phaseResistance;
extern float g_phaseOffsetVoltage;
extern float g_phaseInductance;

extern enum PWMControlDynamicT g_controlMode;

#ifdef __cplusplus
}
#endif

#endif
