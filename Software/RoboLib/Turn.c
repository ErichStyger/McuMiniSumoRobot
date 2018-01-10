/*
 * Turn.c
 *
 *  Created on: Mar 11, 2013
 *      Author: Erich Styger
 */
#include "Platform.h"
#if PL_CONFIG_HAS_TURN
#include "Turn.h"
#include "McuWait.h"
#include "Motor.h"
#include "McuUtility.h"
#if PL_CONFIG_HAS_SHELL
  #include "McuShell.h"
  #include "Shell.h"
#endif
#include "Reflectance.h"
#if PL_CONFIG_HAS_QUADRATURE
  #include "Quadrature.h"
  #include "Pid.h"
#endif
#if PL_CONFIG_HAS_DRIVE
  #include "Drive.h"
#endif

#if PL_CONFIG_HAS_QUADRATURE
  #if PL_CONFIG_IS_DAC_ROBOT
    #define TURN_STEPS_90      600
      /*!< number of steps for a 90 degree turn */
    #define TURN_STEPS_FW_BW   80
      /*!< number of steps for one inch forward/backward */
    #define TURN_STEPS_LINE    150
      /*!< number of steps stepping over the line */
   #define TURN_STEPS_POST_LINE  70
      /*!< number of steps after the line, before making a turn */
    #define TURN_STEPS_90_TIMEOUT_MS        1000
    #define TURN_STEPS_LINE_TIMEOUT_MS      200
    #define TURN_STEPS_POST_LINE_TIMEOUT_MS 200
    #define TURN_STEPS_STOP_TIMEOUT_MS      150
  #elif PL_IS_ROUND_ROBOT
    #define TURN_STEPS_90      20
      /*!< number of steps for a 90 degree turn */
    #define TURN_STEPS_FW_BW   20
      /*!< number of steps for one inch forward/backward */
  #elif PL_IS_INTRO_ZUMO_ROBOT
    #define TURN_STEPS_90         34
      /*!< number of steps for a 90 degree turn */
    #define TURN_STEPS_LINE       12
      /*!< number of steps stepping over the line */
    #define TURN_STEPS_POST_LINE  7
      /*!< number of steps after the line, before making a turn */
  #elif PL_IS_INTRO_ZUMO_ROBOT2
    #define TURN_STEPS_90         36
      /*!< number of steps for a 90 degree turn */
   #define TURN_STEPS_LINE       11
      /*!< number of steps stepping over the line */
    #define TURN_STEPS_POST_LINE  7
      /*!< number of steps after the line, before making a turn */
    #define TURN_STEPS_90_TIMEOUT_MS        1000
    #define TURN_STEPS_LINE_TIMEOUT_MS      200
    #define TURN_STEPS_POST_LINE_TIMEOUT_MS 200
    #define TURN_STEPS_STOP_TIMEOUT_MS      150
  #elif PL_IS_INTRO_ZUMO_K22 && PL_HAS_LIPO
    #define TURN_STEPS_90         600
      /*!< number of steps for a 90 degree turn */
    #define TURN_STEPS_LINE       230
      /*!< number of steps stepping over the line */
    #define TURN_STEPS_POST_LINE  160
      /*!< number of steps after the line, before making a turn */
    #define TURN_STEPS_90_TIMEOUT_MS        1000
    #define TURN_STEPS_LINE_TIMEOUT_MS      500
    #define TURN_STEPS_POST_LINE_TIMEOUT_MS 500
    #define TURN_STEPS_STOP_TIMEOUT_MS      150
  #elif PL_IS_INTRO_ZUMO_K22 /* no LiPo */
    #define TURN_STEPS_90         720
      /*!< number of steps for a 90 degree turn */
    #define TURN_STEPS_LINE       230
      /*!< number of steps stepping over the line */
    #define TURN_STEPS_POST_LINE  160
      /*!< number of steps after the line, before making a turn */
    #define TURN_STEPS_90_TIMEOUT_MS        1000
    #define TURN_STEPS_LINE_TIMEOUT_MS      200
    #define TURN_STEPS_POST_LINE_TIMEOUT_MS 200
    #define TURN_STEPS_STOP_TIMEOUT_MS      150
  #else
    #error "unknown configuration!"
  #endif
#else /* no quadrature, use timed move */
  #if PL_IS_ZUMO_ROBOT && PL_IS_MOTOR_1_100
    #define TURN_90_WAIT_TIME_MS        680 
      /*!< ms to wait for a 90 degree turn */
    #define TURN_STEP_LINE_MS           170
      /*!< ms to do a step over the line */
    #define TURN_STEP_POST_LINE_MS      100
      /*!< ms to do a step over the line */
    #define TURN_MOTOR_DUTY_PERCENT     100
      /*!< maximum motor duty for turn operation */
  #elif PL_IS_ZUMO_ROBOT
    #define TURN_90_WAIT_TIME_MS        400 
      /*!< ms to wait for a 90 degree turn */
    #define TURN_STEP_LINE_MS            90
      /*!< ms to do a step over the line */
    #define TURN_STEP_POST_LINE_MS       60
      /*!< ms to do a step over the line */
    #define TURN_MOTOR_DUTY_PERCENT      60
      /*!< maximum motor duty for turn operation */
  #elif PL_IS_ROUND_ROBOT
    #define TURN_90_WAIT_TIME_MS        250 
      /*!< ms to wait for a 90 degree turn */
    #define TURN_STEP_LINE_MS            50
      /*!< ms to do a step over the line */
    #define TURN_STEP_POST_LINE_MS       50
      /*!< ms to do a step over the line */
    #define TURN_MOTOR_DUTY_PERCENT      20
      /*!< maximum motor duty for turn operation */
  #elif PL_IS_TRACK_ROBOT
    #define TURN_90_WAIT_TIME_MS       1050 
      /*!< ms to wait for a 90 degree turn */
    #define TURN_STEP_LINE_MS           160
      /*!< ms to do a step over the line */
    #define TURN_STEP_POST_LINE_MS      170
      /*!< ms to do a step over the line */
    #define TURN_MOTOR_DUTY_PERCENT      25
      /*!< maximum motor duty for turn operation */
  #elif PL_IS_INTRO_ZUMO_ROBOT
    #define TURN_90_WAIT_TIME_MS        470 
      /*!< ms to wait for a 90 degree turn */
    #define TURN_STEP_LINE_MS            80
      /*!< ms to do a step over the line */
    #define TURN_STEP_POST_LINE_MS       90
      /*!< ms to do a step over the line */
    #define TURN_MOTOR_DUTY_PERCENT      40
      /*!< maximum motor duty for turn operation */
  #elif PL_IS_INTRO_ZUMO_ROBOT2 || PL_IS_INTRO_ZUMO_K22
    #define TURN_90_WAIT_TIME_MS        300 
      /*!< ms to wait for a 90 degree turn */
    #define TURN_STEP_LINE_MS            80
      /*!< ms to do a step over the line */
    #define TURN_STEP_POST_LINE_MS       90
      /*!< ms to do a step over the line */
    #define TURN_MOTOR_DUTY_PERCENT      40
      /*!< maximum motor duty for turn operation */
  #else
    #error "unknown configuration!"
  #endif
#endif /* PL_CONFIG_HAS_QUADRATURE */

#if PL_IS_ZUMO_ROBOT || PL_CONFIG_IS_DAC_ROBOT
  #define TURN_WAIT_AFTER_STEP_MS  0   /* wait this time after a step to have the motor PWM effective */
#elif PL_IS_ROUND_ROBOT
  #define TURN_WAIT_AFTER_STEP_MS  50  /* wait this time after a step to have the motor PWM effective */
#elif PL_IS_TRACK_ROBOT
  #define TURN_WAIT_AFTER_STEP_MS  50  /* wait this time after a step to have the motor PWM effective */
#elif PL_IS_INTRO_ZUMO_ROBOT || PL_IS_INTRO_ZUMO_ROBOT2 || PL_IS_INTRO_ZUMO_K22
  #define TURN_WAIT_AFTER_STEP_MS  0   /* >0: wait this time after a step to have the motor PWM effective */
#else
  #error "unknown configuration!"
#endif

#if PL_CONFIG_HAS_QUADRATURE
static int32_t TURN_Steps90 = TURN_STEPS_90;
static int32_t TURN_StepsLine = TURN_STEPS_LINE;
static int32_t TURN_StepsPostLine = TURN_STEPS_POST_LINE;
#else
static uint8_t TURN_DutyPercent = TURN_MOTOR_DUTY_PERCENT;
static uint16_t TURN_Time90ms = TURN_90_WAIT_TIME_MS;
static uint16_t TURN_StepLineMs = TURN_STEP_LINE_MS;
static uint16_t TURN_StepPostLineMs = TURN_STEP_POST_LINE_MS;
#endif

void TURN_SetStepsLine(int32_t stepsLine, int32_t stepsPostLine) {
  TURN_StepsLine = stepsLine;
  TURN_StepsPostLine = stepsPostLine;
}

/*!
 * \brief Translate a turn kind into a string
 * \return Returns a descriptive string
 */
const unsigned char *TURN_TurnKindStr(TURN_Kind kind) {
  switch(kind) {
    case TURN_LEFT45:                 return (const unsigned char*)"LEFT45";
    case TURN_LEFT90:                 return (const unsigned char*)"LEFT90";
    case TURN_RIGHT45:                return (const unsigned char*)"RIGHT45";
    case TURN_RIGHT90:                return (const unsigned char*)"RIGHT90";
    case TURN_LEFT180:                return (const unsigned char*)"LEFT180";
    case TURN_RIGHT180:               return (const unsigned char*)"RIGHT180";
    case TURN_STRAIGHT:               return (const unsigned char*)"STRAIGHT";
    case TURN_STEP_LINE_FW:           return (const unsigned char*)"STEP_LINE_FW";
    case TURN_STEP_LINE_BW:           return (const unsigned char*)"STEP_LINE_BW";
    case TURN_STEP_POST_LINE_FW:      return (const unsigned char*)"STEP_POST_LINE_FW";
    case TURN_STEP_POST_LINE_BW:      return (const unsigned char*)"STEP_POST_LINE_BW";
    case TURN_STEP_LINE_FW_POST_LINE: return (const unsigned char*)"STEP_LINE_FW_POST_LINE";
    case TURN_STEP_LINE_BW_POST_LINE: return (const unsigned char*)"STEP_LINE_BW_POST_LINE";
    case TURN_STOP:                   return (const unsigned char*)"STOP";
    case TURN_FINISHED:               return (const unsigned char*)"FINISHED";
    default:                          return (const unsigned char*)"TURN_UNKNOWN!";
  }
}

#if !PL_CONFIG_HAS_QUADRATURE
uint8_t TURN_SetDutyPercent(uint8_t duty) {
  uint8_t old;
  
  old = TURN_DutyPercent;
  TURN_DutyPercent = duty;
  return old;
}

#endif

#if PL_CONFIG_HAS_QUADRATURE
void TURN_MoveToPos(int32_t targetLPos, int32_t targetRPos, bool wait, TURN_StopFct stopIt, int32_t timeoutMs) {
#if PL_CONFIG_HAS_DRIVE
  (void)DRV_SetPos(targetLPos, targetRPos);
  (void)DRV_SetMode(DRV_MODE_POS);
#else
  (void)targetLPos;
  (void)targetRPos;
  #warning "not possible to move to a position without drive enabled!"
#endif
  for(;;) { /* breaks */
    if (stopIt!=NULL) {
      if (stopIt()) { /* check stop condition */
        break;
      }
    }
    McuWait_WaitOSms(1); /* wait some time, and give Drive module enough time to use new value */
    timeoutMs--;
    if (timeoutMs<=0) {
      break; /* timeout */
    }
#if PL_CONFIG_HAS_DRIVE
    if (wait && DRV_HasTurned()) {
      break;
    }
#endif
    if (!wait) {
      break;
    }
  } /* for */
#if PL_CONFIG_HAS_SHELL
  if (timeoutMs<=0) {
    McuShell_SendStr((unsigned char*)"MoveToPos Timeout.\r\n", McuShell_GetStdio()->stdErr);
  }
#endif
}

static void StepsTurn(int32_t stepsL, int32_t stepsR, TURN_StopFct stopIt, int32_t timeOutMS) {
  int32_t currLPos, currRPos, targetLPos, targetRPos;
#if PL_CONFIG_HAS_DRIVE
#if 0 /* stop before turn */
#if 1
  int timeout = TURN_STEPS_STOP_TIMEOUT_MS;
  
  DRV_SetMode(DRV_MODE_STOP); /* stop it */
  McuWait_WaitOSms(5);

  while (timeout>0 && !DRV_IsStopped()) {
    /* wait until stopped */
    timeout-=5;
    McuWait_WaitOSms(5);
  }
#if PL_CONFIG_HAS_SHELL
  if (timeout<=0) {
    SHELL_SendString((unsigned char*)"StepsTurn Stopping Timeout.\r\n");
  }
#endif
#else
  DRV_SetMode(DRV_MODE_STOP); /* stop it */
  McuWait_WaitOSms(200); /* give it time to stop */
#endif
#endif
#endif
  currLPos = QUAD_GetLeftPos();
  currRPos = QUAD_GetRightPos();
  targetLPos = currLPos+stepsL;
  targetRPos = currRPos+stepsR;
  TURN_MoveToPos(targetLPos, targetRPos, TRUE, stopIt, timeOutMS); /* go to final position */
}
#endif

static void PostTurn(void) {
#if !PL_CONFIG_HAS_QUADRATURE /* with quadrature counter we already stopped the motors */
  MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), 0);
  MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), 0);
#endif
#if TURN_WAIT_AFTER_STEP_MS > 0
#if PL_CONFIG_HAS_DRIVE
  DRV_SetMode(DRV_MODE_NONE); /* disable any drive mode */
#endif
  McuWait_WaitOSms(TURN_WAIT_AFTER_STEP_MS);
#endif /* TURN_WAIT_AFTER_STEP_MS */
}

#if !PL_CONFIG_HAS_QUADRATURE
static void TimeTurn(bool isLeft, uint8_t duty, uint16_t val) {
  if (isLeft) {
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -duty);
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), duty);
  } else {
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT),   duty);
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -duty);
  }
  McuWait_WaitOSms(val); /* waiting time */
}
#endif

void TURN_Turn(TURN_Kind kind, TURN_StopFct stopIt) {
  switch(kind) {
    case TURN_LEFT45:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-TURN_Steps90/2, TURN_Steps90/2, stopIt, TURN_STEPS_90_TIMEOUT_MS/2);
#else
      TimeTurn(TRUE, TURN_DutyPercent, TURN_Time90ms/2);
#endif
      break;
    case TURN_RIGHT45:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(TURN_Steps90/2, -TURN_Steps90/2, stopIt, TURN_STEPS_90_TIMEOUT_MS/2);
#else
      TimeTurn(FALSE, TURN_DutyPercent, TURN_Time90ms/2);
#endif
      break;
    case TURN_LEFT90:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-TURN_Steps90, TURN_Steps90, stopIt, TURN_STEPS_90_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
      McuWait_WaitOSms(TURN_Time90ms); /* only use waiting time */
#endif
      break;
    case TURN_RIGHT90:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(TURN_Steps90, -TURN_Steps90, stopIt, TURN_STEPS_90_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(TURN_Time90ms); /* only use waiting time */
#endif
      break;
    case TURN_LEFT180:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-(2*TURN_Steps90), 2*TURN_Steps90, stopIt, TURN_STEPS_90_TIMEOUT_MS*2);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
      McuWait_WaitOSms(2*TURN_Time90ms);
#endif
     break;
    case TURN_RIGHT180:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(2*TURN_Steps90, -(2*TURN_Steps90), stopIt, TURN_STEPS_90_TIMEOUT_MS*2);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(2*TURN_Time90ms);
#endif
     break;
    case TURN_STEP_BORDER_BW:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-(3*TURN_StepsLine), -(3*TURN_StepsLine), stopIt, TURN_STEPS_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(3*TURN_StepLineMs);
#endif
      break;
    case TURN_STEP_LINE_FW:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(TURN_StepsLine, TURN_StepsLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepLineMs);
#endif
      break;
    case TURN_STEP_LINE_BW:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-TURN_StepsLine, -TURN_StepsLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepLineMs);
#endif
      break;
    case TURN_STEP_POST_LINE_FW:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(TURN_StepsPostLine, TURN_StepsPostLine, stopIt, TURN_STEPS_POST_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepPostLineMs);
#endif
      break;
    case TURN_STEP_POST_LINE_BW:
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-TURN_StepsPostLine, -TURN_StepsPostLine, stopIt, TURN_STEPS_POST_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepPostLineMs);
#endif
      break;
    case TURN_STEP_LINE_FW_POST_LINE: /* combination of TURN_STEP_LINE_FW and TURN_STEP_POST_LINE_FW */
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(TURN_StepsLine+TURN_StepsPostLine, TURN_StepsLine+TURN_StepsPostLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS+TURN_STEPS_POST_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepLineMs);

      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepPostLineMs);
#endif
      break;
    case TURN_STEP_LINE_BW_POST_LINE: /* combination of TURN_STEP_LINE_BW and TURN_STEP_POST_LINE_BW */
#if PL_CONFIG_HAS_QUADRATURE
      StepsTurn(-TURN_StepsLine-TURN_StepsPostLine, -TURN_StepsLine-TURN_StepsPostLine, stopIt, TURN_STEPS_LINE_TIMEOUT_MS+TURN_STEPS_POST_LINE_TIMEOUT_MS);
#else
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepLineMs);

      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
      McuWait_WaitOSms(TURN_StepPostLineMs);
#endif
      break;
    case TURN_STOP_LEFT:
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), 0);
      break;
    case TURN_STOP_RIGHT:
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), 0);
      break;
    case TURN_STOP:
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), 0);
      MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), 0);
      break;
  default:
    break;
  }
  /*lint -save  -e522 Highest operation, function 'PostTurn', lacks side-effect. */
  PostTurn(); /* perform post-turn action */
  /*lint -restore Highest operation, function 'PostTurn', lacks side-effect. */
}

void TURN_TurnAngle(int16_t angle, TURN_StopFct stopIt) {
  bool isLeft = angle<0;
#if PL_CONFIG_HAS_QUADRATURE
  int32_t steps;
#else
  uint32_t time;
#endif
  
  if (isLeft) {
    angle = -angle; /* make it positive */
  }
  angle %= 360; /* keep it inside 360° */
#if PL_CONFIG_HAS_QUADRATURE
  steps = (angle*TURN_Steps90)/90;
#else
  time = (angle*TURN_Time90ms)/90;
#endif
  if (isLeft) {
#if PL_CONFIG_HAS_QUADRATURE
    StepsTurn(-steps, steps, stopIt, ((angle/90)+1)*TURN_STEPS_90_TIMEOUT_MS);
#else
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), -TURN_DutyPercent);
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), TURN_DutyPercent);
    McuWait_WaitOSms(time);
#endif
  } else { /* right */
#if PL_CONFIG_HAS_QUADRATURE
    StepsTurn(steps, -steps, stopIt, ((angle/90)+1)*TURN_STEPS_90_TIMEOUT_MS);
#else
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_LEFT), TURN_DutyPercent);
    MOT_SetSpeedPercent(MOT_GetMotorHandle(MOT_MOTOR_RIGHT), -TURN_DutyPercent);
    McuWait_WaitOSms(time);
#endif
  }
  /*lint -save  -e522 Highest operation, function 'PostTurn', lacks side-effect. */
  PostTurn(); /* perform post-turn action */
  /*lint -restore Highest operation, function 'PostTurn', lacks side-effect. */
}

#if PL_CONFIG_HAS_SHELL
static void TURN_PrintHelp(const McuShell_StdIOType *io) {
  McuShell_SendHelpStr((unsigned char*)"turn", (unsigned char*)"Group of turning commands\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  help|status", (unsigned char*)"Shows turn help or status\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  <angle>", (unsigned char*)"Turn the robot by angle, negative is counter-clockwise, e.g. 'turn -90'\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  forward", (unsigned char*)"Move one step forward\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  forward postline", (unsigned char*)"Move one step forward post the line\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  backward", (unsigned char*)"Move one step backward\r\n", io->stdOut);
#if PL_CONFIG_HAS_QUADRATURE
  McuShell_SendHelpStr((unsigned char*)"  steps90 <steps>", (unsigned char*)"Number of steps for a 90 degree turn\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  stepsline <steps>", (unsigned char*)"Number of steps for stepping over line\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  stepspostline <steps>", (unsigned char*)"Number of steps for a step post the line\r\n", io->stdOut);
#else
  McuShell_SendHelpStr((unsigned char*)"  duty <percent>", (unsigned char*)"Turning motor PWM duty percent\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  time90 <ms>", (unsigned char*)"Time in milli-seconds for 90 degree\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  timeline <ms>", (unsigned char*)"Time in milli-seconds for a single line step\r\n", io->stdOut);
  McuShell_SendHelpStr((unsigned char*)"  timepostline <ms>", (unsigned char*)"Time in milli-seconds for a step post the line\r\n", io->stdOut);
#endif
}

static void TURN_PrintStatus(const McuShell_StdIOType *io) {
  unsigned char buf[32];

  McuShell_SendStatusStr((unsigned char*)"turn", (unsigned char*)"\r\n", io->stdOut);
#if PL_CONFIG_HAS_QUADRATURE
  McuUtility_Num32sToStr(buf, sizeof(buf), TURN_Steps90);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" steps\r\n");
  McuShell_SendStatusStr((unsigned char*)"  90°", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), TURN_StepsLine);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" steps\r\n");
  McuShell_SendStatusStr((unsigned char*)"  line", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), TURN_StepsPostLine);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" steps\r\n");
  McuShell_SendStatusStr((unsigned char*)"  postline", buf, io->stdOut);

  McuUtility_Num32sToStr(buf, sizeof(buf), QUAD_GetLeftPos());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", ");
  McuUtility_strcatNum32u(buf, sizeof(buf), QUAD_NofLeftErrors());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" errors\r\n");
  McuShell_SendStatusStr((unsigned char*)"  left pos", buf, io->stdOut);
  
  McuUtility_Num32sToStr(buf, sizeof(buf), QUAD_GetLeftPos());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)", ");
  McuUtility_strcatNum32u(buf, sizeof(buf), QUAD_NofRightErrors());
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" errors\r\n");
  McuShell_SendStatusStr((unsigned char*)"  right pos", buf, io->stdOut);
#else
  McuUtility_Num16uToStr(buf, sizeof(buf), TURN_DutyPercent);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)"%\r\n");
  McuShell_SendStatusStr((unsigned char*)"  duty", buf, io->stdOut);

  McuUtility_Num16uToStr(buf, sizeof(buf), TURN_Time90ms);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" ms\r\n");
  McuShell_SendStatusStr((unsigned char*)"  time90", buf, io->stdOut);

  McuUtility_Num16uToStr(buf, sizeof(buf), TURN_StepLineMs);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" ms\r\n");
  McuShell_SendStatusStr((unsigned char*)"  line", buf, io->stdOut);

  McuUtility_Num16uToStr(buf, sizeof(buf), TURN_StepPostLineMs);
  McuUtility_strcat(buf, sizeof(buf), (unsigned char*)" ms\r\n");
  McuShell_SendStatusStr((unsigned char*)"  postline", buf, io->stdOut);
#endif
}

static bool isNumberStart(uint8_t ch) {
  if (ch=='-') { /* negative number start */
    return TRUE;
  } else if (ch>='0' && ch<='9') {
    return TRUE;
  }
  return FALSE;
}

uint8_t TURN_ParseCommand(const unsigned char *cmd, bool *handled, const McuShell_StdIOType *io) {
  uint8_t res = ERR_OK;
  const unsigned char *p;
#if !PL_CONFIG_HAS_QUADRATURE
  uint8_t val8u;
#endif
  uint16_t val16u;

  if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_HELP)==0 || McuUtility_strcmp((char*)cmd, (char*)"turn help")==0) {
    TURN_PrintHelp(io);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)McuShell_CMD_STATUS)==0 || McuUtility_strcmp((char*)cmd, (char*)"turn status")==0) {
    TURN_PrintStatus(io);
    *handled = TRUE;
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn ", sizeof("turn ")-1)==0 && isNumberStart(cmd[sizeof("turn ")-1])) {
    int32_t angle;

    p = cmd+sizeof("turn ")-1;
    if (McuUtility_xatoi(&p, &angle)==ERR_OK) {
      TURN_TurnAngle((int16_t)angle, NULL);
      TURN_Turn(TURN_STOP, NULL);
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strcmp((char*)cmd, (char*)"turn forward postline")==0) {
    TURN_Turn(TURN_STEP_LINE_FW_POST_LINE, NULL);
    TURN_Turn(TURN_STOP, NULL);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"turn forward")==0) {
    TURN_Turn(TURN_STEP_LINE_FW, NULL);
    TURN_Turn(TURN_STOP, NULL);
    *handled = TRUE;
  } else if (McuUtility_strcmp((char*)cmd, (char*)"turn backward")==0) {
    TURN_Turn(TURN_STEP_LINE_BW, NULL);
    TURN_Turn(TURN_STOP, NULL);
    *handled = TRUE;
#if PL_CONFIG_HAS_QUADRATURE
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn steps90 ", sizeof("turn steps90 ")-1)==0) {
    p = cmd+sizeof("turn steps90");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_Steps90 = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn stepspostline ", sizeof("turn stepspostline ")-1)==0) {
    p = cmd+sizeof("turn stepspostline");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_StepsPostLine = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn stepsline ", sizeof("turn stepsline ")-1)==0) {
    p = cmd+sizeof("turn stepsline");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_StepsLine = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
#else
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn duty ", sizeof("turn duty ")-1)==0) {
    p = cmd+sizeof("turn duty");
    if (McuUtility_ScanDecimal8uNumber(&p, &val8u)==ERR_OK && val8u<=100) {
      TURN_DutyPercent = val8u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument, must be in the range 0..100\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn time90 ", sizeof("turn time90 ")-1)==0) {
    p = cmd+sizeof("turn time90");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_Time90ms = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn timeline ", sizeof("turn timeline ")-1)==0) {
    p = cmd+sizeof("turn timeline");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_StepLineMs = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
  } else if (McuUtility_strncmp((char*)cmd, (char*)"turn timepostline ", sizeof("turn timepostline ")-1)==0) {
    p = cmd+sizeof("turn timepostline");
    if (McuUtility_ScanDecimal16uNumber(&p, &val16u)==ERR_OK) {
      TURN_StepPostLineMs = val16u;
      *handled = TRUE;
    } else {
      McuShell_SendStr((unsigned char*)"Wrong argument\r\n", io->stdErr);
      res = ERR_FAILED;
    }
#endif
  }
  return res;
}
#endif /* PL_CONFIG_HAS_SHELL */

void TURN_Init(void) {
#if PL_CONFIG_HAS_QUADRATURE
  TURN_Steps90 = TURN_STEPS_90;
  TURN_StepsPostLine = TURN_STEPS_POST_LINE;
  TURN_StepsLine = TURN_STEPS_LINE;
#else
  TURN_DutyPercent = TURN_MOTOR_DUTY_PERCENT;
  TURN_Time90ms = TURN_90_WAIT_TIME_MS;
  TURN_StepLineMs = TURN_STEP_LINE_MS;
  TURN_StepPostLineMs = TURN_STEP_POST_LINE_MS;
#endif
}
#endif /* PL_CONFIG_HAS_TURN */
