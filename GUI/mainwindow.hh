#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <chrono>

#include "dogbot/Coms.hh"
#include "dogbot/DogBotAPI.hh"
#include "dogbot/LegKinematics.hh"
#include "ServoTable.hh"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void timerEvent(QTimerEvent *event) override;

private slots:
  void on_pushButtonConnect_clicked();

  void on_pushButtonPWMReport_clicked();

  void on_pushButtonPing_clicked();

  void on_comboBoxMotorControlMode_activated(const QString &arg1);

  void on_sliderPosition_sliderMoved(int position);

  void on_sliderTorque_sliderMoved(int torque);

  void on_pushButtonQueryId_clicked();

  void on_pushButtonGetVersion_clicked();

  void on_pushButtonState_clicked();

  void on_pushButtonQueryDevices_clicked();

  void on_pushButtonPing1_clicked();

  void on_pushButtonOpenLog_clicked();

  void on_pushButtonQueryState_clicked();

  void on_pushButtonDrv8305_2_clicked();

  void on_pushButtonDrv8305_3_clicked();

  void on_pushButtonDrv8305_5_clicked();

  void on_pushButtonDrv8305_4_clicked();

  void on_pushButtonTim1_clicked();

  void on_spinDeviceId_valueChanged(int arg1);

  void on_comboBoxCalibration_activated(const QString &arg1);

  void on_comboBoxControlState_activated(const QString &arg1);

  void on_checkBoxIndicator_toggled(bool checked);

  void on_spinOtherJointId_valueChanged(int arg1);

  void on_comboBox_activated(const QString &arg1);

  void on_comboBoxPositionRef_activated(const QString &arg1);

  void on_pushButton_2_clicked();

  void on_pushButtonCalZero_clicked();

  void on_doubleSpinBoxJointRelGain_valueChanged(double arg1);

  void on_doubleSpinBoxJointRelOffset_valueChanged(double arg1);

  void on_doubleSpinBoxDemandPosition_editingFinished();

  void on_doubleSpinBoxTorqueLimit_editingFinished();

  void on_doubleSpinBoxDemandPosition_valueChanged(double arg1);

  void on_doubleSpinBoxCalibrationOffset_editingFinished();

  void on_pushButton_3_clicked();

  void on_pushButton_4_clicked();

  void on_doubleSpinBoxIGain_valueChanged(double arg1);

  void updateIGain(double arg1);
  void on_sliderPosition_valueChanged(int value);

  void on_doubleSpinBoxVelocityGain_valueChanged(double arg1);

  void on_doubleSpinBoxVelocityIGain_valueChanged(double arg1);

  void on_doubleSpinBoxVelocityLimit_valueChanged(double arg1);

  void on_doubleSpinBoxPositionGain_valueChanged(double arg1);

  void on_pushButton_5_clicked();

  void on_pushButtonEmergencyStop_clicked();

  void on_doubleSpinBoxHomeIndexOffset_valueChanged(double arg1);

  void on_pushButtonQueryHomed_clicked();

  void on_pushButtonOff_clicked();

  void on_pushButtonHold_clicked();

  void on_pushButtonBrake_clicked();

  void on_comboBoxServoName_activated(const QString &arg1);

  void on_actionSaveConfig_triggered();

  void on_actionLoadConfig_triggered();

  void on_actionSave_Config_As_triggered();

  void on_pushButtonResetAll_clicked();

  void on_pushButtonRefresh_clicked();

  void on_actionExit_triggered();

  void on_comboBoxFanState_activated(int index);

  void on_checkBoxJointRelative_stateChanged(int arg1);

signals:
  void setLogText(const QString &str);
  void setControlState(const QString &str);
  void setControlMode(const QString &str);
  void setFault(const QString &str);
  void setCalibrationState(int index);
  void setCalibrationAngle(double value);
  void setOtherJoint(int jointId);
  void setPositionRef(const QString &str);
  void setIndicator(bool state);
  void setOtherJointGain(double gain);
  void setOtherJointOffset(double offset);
  void setSupplyVoltage(QString str);
  void setDriveTemperature(QString str);
  void setMotorTemperature(QString str);
  void setMotorIGain(double offset);
  void setMotorVelocity(double offset);
  void setVelocityPGain(double offset);
  void setVelocityIGain(double offset);
  void setDemandPhaseVelocity(double offset);
  void setVelocityLimit(double offset);
  void setPositionGain(double offset);
  void setHomeIndexOffset(double offset);
  void setHallSensors(const QString &str);
  void setUSBErrors(const QString &str);
  void setUSBDrops(const QString &str);
  void setCANErrors(const QString &str);
  void setCANDrops(const QString &str);
  void setMainLoopTimeout(const QString &str);
  void setFaultMap(const QString &str);
  void setIndexSensor(bool state);
  void setJointRelativeEnabled(bool state);
  void setFanMode(const QString &str);

private:
  void SetupComs();

  void QueryAll();

  bool ProcessParam(struct PacketParam8ByteC *psp, std::string &displayStr);

  //! Close current connection
  void CloseConnection();

  int m_toQuery = 0;
  std::vector<ComsParameterIndexT> m_displayQuery;

  Ui::MainWindow *ui;
  std::shared_ptr<DogBotN::ComsC> m_coms;
  std::shared_ptr<DogBotN::DogBotAPIC> m_dogBotAPI;
  bool m_PWMReportRequested = false;

  std::vector<PacketDeviceIdC> m_devices;
  float m_position = 0;
  float m_torque = 0;
  std::shared_ptr<std::ostream> m_logStrm;
  int m_targetDeviceId = 0;
  enum PositionReferenceT g_positionReference = PR_Relative;

  float m_servoAngle = 0;
  float m_servoTorque = 0;
  enum PositionReferenceT m_servoRef = PR_Relative;
  enum PWMControlDynamicT m_controlMode = CM_Off;
  ServoTable *m_servoTable = 0;
  std::string m_configFilename;
};

#endif // MAINWINDOW_H
