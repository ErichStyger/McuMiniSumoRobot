/*
 * oled.c
 *
 *  Created on: 29.12.2017
 *      Author: Erich Styger
 */


#include "Platform.h"

#if PL_CONFIG_HAS_LCD
#include "McuSSD1306.h"
#include "McuGDisplaySSD1306.h"
#include "McuUtility.h"
#if PL_CONFIG_HAS_LCD_MENU
  #include "LCDMenu.h"
#endif
#if PL_CONFIG_HAS_QUADRATURE
  #include "Quadrature.h"
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  #include "Reflectance.h"
#endif
#if PL_CONFIG_HAS_LINE
  #include "Line.h"
#endif
#if PL_CONFIG_HAS_PROXIMITY
  #include "Proximity.h"
#endif
#include "McuFontDisplay.h"
#include "McuFontHelv08Normal.h"
#include "McuFontCour08Normal.h"
#include "Event.h"

#if PL_CONFIG_HAS_LCD_MENU
#define GET_FONT()          McuFontHelv08Normal_GetFont()
#if 0
#define GET_FONT_FIXED()    McuFontCour08Normal_GetFont()
#else /* save about 5kB FLASH */
#define GET_FONT_FIXED()    GET_FONT()
#endif

typedef enum {
  LCD_MENU_ID_NONE = LCDMENU_ID_NONE, /* special value! */
  LCD_MENU_ID_MAIN,
    LCD_MENU_ID_NUM_VALUE,
    LCD_MENU_ID_BACK_MAIN,
  LCD_MENU_ID_ROBOT,
#if PL_CONFIG_HAS_SUMO
    LCD_MENU_ID_SUMO_START,
    LCD_MENU_ID_SUMO_STOP,
#endif
#if PL_CONFIG_HAS_QUADRATURE
    LCD_MENU_ID_ROBOT_ENCODER,
#endif
#if PL_CONFIG_HAS_REFLECTANCE
    LCD_MENU_ID_ROBOT_REFLECTANCE,
#endif
#if PL_CONFIG_HAS_LINE
    LCD_MENU_ID_ROBOT_CALIBRATE,
#endif
#if PL_CONFIG_HAS_PROXIMITY
    LCD_MENU_ID_ROBOT_PROXIMITY,
#endif
    LCD_MENU_ID_BACK_ROBOT,
} LCD_MenuIDs;

typedef enum {
  ROBOT_MENU_POS_BACK_ROBOT,
#if PL_CONFIG_HAS_SUMO
  ROBOT_MENU_POS_START,
  ROBOT_MENU_POS_STOP,
#endif
#if PL_CONFIG_HAS_QUADRATURE
  ROBOT_MENU_POS_ENCODER,
#endif
#if PL_CONFIG_HAS_REFLECTANCE
  ROBOT_MENU_POS_REFLECTANCE,
#endif
#if PL_CONFIG_HAS_LINE
  ROBOT_MENU_POS_CALIBRATE,
#endif
#if PL_CONFIG_HAS_PROXIMITY
  ROBOT_MENU_POS_PROXIMITY,
#endif
} RobotMenuPos_e;

typedef enum {
    LCD_MENU_SCREEN_NONE,
#if PL_CONFIG_HAS_QUADRATURE
    LCD_MENU_SCREEN_ENCODER,
#endif
#if PL_CONFIG_HAS_REFLECTANCE
    LCD_MENU_SCREEN_REFLECTANCE,
#endif
#if PL_CONFIG_HAS_PROXIMITY
    LCD_MENU_SCREEN_PROXIMITY,
#endif
} LCD_MenuScreenIDs;

static LCD_MenuScreenIDs LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;

static LCDMenu_StatusFlags ValueChangeHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  static int value = 0;
  static uint8_t valueBuf[16];
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
  if (event==LCDMENU_EVENT_GET_TEXT) {
    McuUtility_strcpy(valueBuf, sizeof(valueBuf), (uint8_t*)"Val: ");
    McuUtility_strcatNum32s(valueBuf, sizeof(valueBuf), value);
    *dataP = valueBuf;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_GET_EDIT_TEXT) {
    McuUtility_strcpy(valueBuf, sizeof(valueBuf), (uint8_t*)"[-] ");
    McuUtility_strcatNum32s(valueBuf, sizeof(valueBuf), value);
    McuUtility_strcat(valueBuf, sizeof(valueBuf), (uint8_t*)" [+]");
    *dataP = valueBuf;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_DECREMENT) {
    value--;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  } else if (event==LCDMENU_EVENT_INCREMENT) {
    value++;
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
  }
  return flags;
}

#if PL_CONFIG_HAS_QUADRATURE
static void ShowEncoderScreen(void) {
  McuFontDisplay_PixelDim x, y;
  uint8_t buf[24];

  McuGDisplaySSD1306_Clear();
  McuGDisplaySSD1306_DrawBox(0, 0, McuGDisplaySSD1306_GetWidth(), McuGDisplaySSD1306_GetHeight(), 1, McuGDisplaySSD1306_COLOR_BLUE);

  x = 2; y = 2;
  McuFontDisplay_WriteString((uint8_t*)"Encoder:\n", McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT());

  McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"L: ");
  McuUtility_strcatNum32sFormatted(buf, sizeof(buf), QUAD_GetLeftPos(), ' ', 8);
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"R: ");
  McuUtility_strcatNum32sFormatted(buf, sizeof(buf), QUAD_GetRightPos(), ' ', 8);
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  McuGDisplaySSD1306_UpdateFull();
}
#endif

#if PL_CONFIG_HAS_REFLECTANCE
static void ShowReflectanceScreen(void) {
  McuFontDisplay_PixelDim x, y;
  uint8_t buf[24];
  int i;

  McuGDisplaySSD1306_Clear();
  McuGDisplaySSD1306_DrawBox(0, 0, McuGDisplaySSD1306_GetWidth(), McuGDisplaySSD1306_GetHeight(), 1, McuGDisplaySSD1306_COLOR_BLUE);

  x = 2; y = 2;
  McuFontDisplay_WriteString((uint8_t*)"Reflectance:\n", McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT());
  buf[0] = '\0';
  for(i=0;i<REF_NOF_SENSORS;i++) {
    McuUtility_strcatNum16Hex(buf, sizeof(buf), REF_GetRawValue(i));
    McuUtility_chcat(buf, sizeof(buf), ' ');
  }
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());
#if PL_CONFIG_HAS_LINE
  buf[0] = '\0';
  for(i=0;i<REF_NOF_SENSORS;i++) {
    McuUtility_strcatNum16Hex(buf, sizeof(buf), LINE_Get1kValue(i));
    McuUtility_chcat(buf, sizeof(buf), ' ');
  }
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"line: ");
  McuUtility_strcatNum16u(buf, sizeof(buf), LINE_GetLinePos());
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());
#endif
  McuGDisplaySSD1306_UpdateFull();
}
#endif

#if PL_CONFIG_HAS_PROXIMITY
static void ShowProximityScreen(void) {
  McuFontDisplay_PixelDim x, y;
  uint8_t buf[24];
  int i;
  PROX_Bits prox;
  McuGDisplaySSD1306_Clear();
  McuGDisplaySSD1306_DrawBox(0, 0, McuGDisplaySSD1306_GetWidth(), McuGDisplaySSD1306_GetHeight(), 1, McuGDisplaySSD1306_COLOR_BLUE);

  x = 2; y = 2;
  McuFontDisplay_WriteString((uint8_t*)"Proximity:\n", McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT());
  if (PROX_HasTarget()) {
    McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"Target: yes, at ");
    McuUtility_strcatNum32s(buf, sizeof(buf), PROX_GetTargetAngle());
    McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"°\n");
  } else {
    McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"Target: no\n");
  }
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  prox = PROX_GetProxBits();

  if (prox&PROX_L_LEFT_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, 5, 8, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, 5, 8, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  if (prox&PROX_L_MIDDLE_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x+6, y, 5, 8, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x+6, y, 5, 8, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  if (prox&PROX_R_MIDDLE_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x+12, y, 5, 8, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x+12, y, 5, 8, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  if (prox&PROX_R_RIGHT_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x+18, y, 5, 8, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x+18, y, 5, 8, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }

  McuGDisplaySSD1306_UpdateFull();
}
#endif

static LCDMenu_StatusFlags RobotMenuHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
  if (event==LCDMENU_EVENT_ENTER) {
#if PL_CONFIG_HAS_QUADRATURE
	  if (item->id==LCD_MENU_ID_ROBOT_ENCODER) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
        ShowEncoderScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_ENCODER;
	  }
#endif
#if PL_CONFIG_HAS_REFLECTANCE
    if (item->id==LCD_MENU_ID_ROBOT_REFLECTANCE) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
        ShowReflectanceScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_REFLECTANCE;
    }
#endif
#if PL_CONFIG_HAS_LINE
    if (item->id==LCD_MENU_ID_ROBOT_CALIBRATE) {
      LINE_CalibrateStartStop();
      vTaskDelay(pdMS_TO_TICKS(100)); /* wait some time to get it started, otherwise menu won't refresh properly */
      flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
    }
#endif
#if PL_CONFIG_HAS_PROXIMITY
    if (item->id==LCD_MENU_ID_ROBOT_PROXIMITY) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
        ShowProximityScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_PROXIMITY;
    }
#endif
  } else if (event==LCDMENU_EVENT_GET_TEXT && dataP!=NULL) {
#if PL_CONFIG_HAS_LINE
    if (item->id==LCD_MENU_ID_ROBOT_CALIBRATE) {
      if (LINE_IsCalibrating()) {
        *dataP = "Stop Calibrate";
      } else {
        *dataP = "Start Calibrate";
      }
    }
    flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
#endif
  }
  return flags;
}

static LCDMenu_StatusFlags MenuBackHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
  if (event==LCDMENU_EVENT_ENTER) {
    flags |= LCDMENU_STATUS_FLAGS_HANDLED;
    LCDMenu_OnEvent(LCDMENU_EVENT_LEFT, item); /* go up on menu structure */
  }
  return flags;
}

static const LCDMenu_MenuItem menus[] =
{/* id,                                     grp, pos,                           up,                       down,                             text,           callback                      flags                  */
    {LCD_MENU_ID_MAIN,                        0,   0,                           LCD_MENU_ID_NONE,         LCD_MENU_ID_NUM_VALUE,            "General",      NULL,                         LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_NUM_VALUE,                 1,   0,                           LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 NULL,           ValueChangeHandler,           LCDMENU_MENU_FLAGS_EDITABLE},
      {LCD_MENU_ID_BACK_MAIN,                 1,   1,                           LCD_MENU_ID_MAIN,         LCD_MENU_ID_NONE,                 "BACK",         MenuBackHandler,              LCDMENU_MENU_FLAGS_NONE},
    {LCD_MENU_ID_ROBOT,                       0,   1,                           LCD_MENU_ID_NONE,         LCD_MENU_ID_BACK_ROBOT,           "Robot",        NULL,                         LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_BACK_ROBOT,                2,   ROBOT_MENU_POS_BACK_ROBOT,   LCD_MENU_ID_ROBOT,        LCD_MENU_ID_NONE,                 "BACK",         MenuBackHandler,              LCDMENU_MENU_FLAGS_NONE},
#if PL_CONFIG_HAS_SUMO
      {LCD_MENU_ID_SUMO_START,                2,   ROBOT_MENU_POS_START,        LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 "Sumo Start",   RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_SUMO_STOP,                 2,   ROBOT_MENU_POS_STOP,         LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 "Sumo Stop",    RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_QUADRATURE
      {LCD_MENU_ID_ROBOT_ENCODER,             2,   ROBOT_MENU_POS_ENCODER,      LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 "Encoder",      RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_REFLECTANCE
      {LCD_MENU_ID_ROBOT_REFLECTANCE,         2,   ROBOT_MENU_POS_REFLECTANCE,  LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 "Reflectance",  RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_LINE
      {LCD_MENU_ID_ROBOT_CALIBRATE,           2,   ROBOT_MENU_POS_CALIBRATE,    LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 NULL,           RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_PROXIMITY
      {LCD_MENU_ID_ROBOT_PROXIMITY,           2,   ROBOT_MENU_POS_PROXIMITY,    LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 "Proximity",    RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
};

#endif /* PL_CONFIG_HAS_LCD_MENU */

static void LCD_Task(void *param) {
  (void)param; /* not used */
#if PL_CONFIG_HAS_LCD_MENU
  LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
  LCDMenu_InitMenu(menus, sizeof(menus)/sizeof(menus[0]), LCD_MENU_ID_MAIN);
  LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL); /* initial refresh */
#endif
  for(;;) {
#if PL_CONFIG_HAS_LCD_MENU
    if (EVNT_EventIsSetAutoClear(EVNT_SW1_LLRELEASED)) {
      if (LCD_CurrentScreen!=LCD_MENU_SCREEN_NONE) { /* key press leaves screen */
        LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
        LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
      } else {
        LCDMenu_OnEvent(LCDMENU_EVENT_ENTER, NULL);
      }
    } else if (EVNT_EventIsSetAutoClear(EVNT_SW1_LRELEASED)) {
      if (LCD_CurrentScreen!=LCD_MENU_SCREEN_NONE) { /* key press leaves screen */
        LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
        LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
      } else {
        LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
      }
    } else if (EVNT_EventIsSetAutoClear(EVNT_SW1_RELEASED)) {
      if (LCD_CurrentScreen!=LCD_MENU_SCREEN_NONE) { /* key press leaves screen */
        LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
        LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
      } else {
        LCDMenu_OnEvent(LCDMENU_EVENT_DOWN, NULL);
      }
    }
    /* update screens */
#if PL_CONFIG_HAS_QUADRATURE
    if (LCD_CurrentScreen==LCD_MENU_SCREEN_ENCODER) {
      ShowEncoderScreen();
    }
#endif
#if PL_CONFIG_HAS_REFLECTANCE
    if (LCD_CurrentScreen==LCD_MENU_SCREEN_REFLECTANCE) {
      ShowReflectanceScreen();
    }
#endif
#if PL_CONFIG_HAS_PROXIMITY
    if (LCD_CurrentScreen==LCD_MENU_SCREEN_PROXIMITY) {
      ShowProximityScreen();
    }
#endif
#endif /* PL_CONFIG_HAS_LCD_MENU */
    vTaskDelay(pdMS_TO_TICKS(50));
  } /* for */
}

void LCD_Init(void) {
  McuSSD1306_Clear();
  if (xTaskCreate(LCD_Task, "LCD", 600/sizeof(StackType_t), NULL, tskIDLE_PRIORITY, NULL) != pdPASS) {
    for(;;){} /* error! probably out of memory */
  }
}
#endif /* PL_CONFIG_HAS_LCD */
