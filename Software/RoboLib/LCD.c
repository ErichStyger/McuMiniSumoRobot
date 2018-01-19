/**
 * \file
 * \brief LCD (OLED) Display
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements the high level LCD task for the OLED.
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
#if PL_CONFIG_HAS_SUMO
  #include "Sumo.h"
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
#define LCD_USE_ENCODER_AS_INPUT   (1 && PL_CONFIG_HAS_QUADRATURE)
#if LCD_USE_ENCODER_AS_INPUT
  #define LCD_ENCODER_COUNTER_MENU_DELTA   20  /* number of encoder counter ticks for a menu event */
#endif

typedef enum {
  LCD_MENU_ID_NONE = LCDMENU_ID_NONE, /* special value! */
  LCD_MENU_ID_GENERAL,
    LCD_MENU_ID_BACK_GENERAL,
    LCD_MENU_ID_NUM_VALUE,
  LCD_MENU_ID_ROBOT,
    LCD_MENU_ID_BACK_ROBOT,
#if PL_CONFIG_HAS_SUMO
    LCD_MENU_ID_SUMO_START_STOP,
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
} LCD_MenuIDs;

typedef enum {
  LCD_MENU_GRP_ID_MAIN,
    LCD_MENU_GRP_ID_GENERAL,
    LCD_MENU_GRP_ID_ROBOT,
} LCD_MenuGroupID;

/* position numbers for menu items in Robot menu */
typedef enum {
  ROBOT_MENU_POS_BACK_ROBOT,
#if PL_CONFIG_HAS_SUMO
  ROBOT_MENU_POS_SUMO_START_STOP,
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

/* IDs for different screens with status information */
typedef enum {
    LCD_MENU_SCREEN_NONE,
#if PL_CONFIG_HAS_QUADRATURE
    LCD_MENU_SCREEN_ENCODER,
#endif
#if PL_CONFIG_HAS_REFLECTANCE
    LCD_MENU_SCREEN_REFLECTANCE,
#endif
#if PL_CONFIG_HAS_LINE
    LCD_MENU_SCREEN_LINE_CALIBRATE,
#endif
#if PL_CONFIG_HAS_PROXIMITY
    LCD_MENU_SCREEN_PROXIMITY,
#endif
#if PL_CONFIG_HAS_SUMO
    LCD_MENU_SCREEN_SUMO,
#endif
} LCD_MenuScreenIDs;

static LCD_MenuScreenIDs LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
#if LCD_USE_ENCODER_AS_INPUT
  static bool LCD_useEncoderForMenuNavigation = FALSE;
#endif

#if PL_CONFIG_HAS_LCD_HEADER
static LCDMenu_StatusFlags HeaderMenuHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;
  (void)item;
  if (event==LCDMENU_EVENT_GET_HEADER_TEXT && dataP!=NULL && item!=NULL) {
    if (item->group==LCD_MENU_GRP_ID_MAIN) {
      *dataP = "DAC Sumo Robot Menu";
      flags |= LCDMENU_STATUS_FLAGS_HANDLED;
    }
    if (item->group==LCD_MENU_GRP_ID_GENERAL) {
      *dataP = "General Menu";
      flags |= LCDMENU_STATUS_FLAGS_HANDLED;
    }
    if (item->group==LCD_MENU_GRP_ID_ROBOT) {
      *dataP = "Robot Menu";
      flags |= LCDMENU_STATUS_FLAGS_HANDLED;
    }
  }
  return flags;
}
#endif

static LCDMenu_StatusFlags ValueChangeHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  static int value = 0;
  static uint8_t valueBuf[16];
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
#if PL_CONFIG_HAS_LCD_HEADER
  if (event==LCDMENU_EVENT_GET_HEADER_TEXT ) {
    return HeaderMenuHandler(item, event, dataP);
  }
#endif
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
#if LCDMENU_CONFIG_LCD_HEADER_HEIGHT>0
  y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT;
#endif
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
#if LCDMENU_CONFIG_LCD_HEADER_HEIGHT>0
  y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT;
#endif
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

#if PL_CONFIG_HAS_LINE
static void ShowLineCalibrateScreen(void) {
  McuFontDisplay_PixelDim x, y;
  uint8_t buf[24];
  int i;

  McuGDisplaySSD1306_Clear();
  McuGDisplaySSD1306_DrawBox(0, 0, McuGDisplaySSD1306_GetWidth(), McuGDisplaySSD1306_GetHeight(), 1, McuGDisplaySSD1306_COLOR_BLUE);

  x = 2; y = 2;
  McuFontDisplay_WriteString((uint8_t*)"Calibrate over B/W\n", McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT());
  buf[0] = '\0';
  for(i=0;i<REF_NOF_SENSORS;i++) {
    McuUtility_strcatNum16Hex(buf, sizeof(buf), REF_GetRawValue(i));
    McuUtility_chcat(buf, sizeof(buf), ' ');
  }
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
#if LCDMENU_CONFIG_LCD_HEADER_HEIGHT>0
  y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT;
#endif
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());
#if PL_CONFIG_HAS_LINE
  buf[0] = '\0';
  for(i=0;i<REF_NOF_SENSORS;i++) {
    McuUtility_strcatNum16Hex(buf, sizeof(buf), LINE_GetMinValue(i));
    McuUtility_chcat(buf, sizeof(buf), ' ');
  }
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  buf[0] = '\0';
  for(i=0;i<REF_NOF_SENSORS;i++) {
    McuUtility_strcatNum16Hex(buf, sizeof(buf), LINE_GetMaxValue(i));
    McuUtility_chcat(buf, sizeof(buf), ' ');
  }
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  buf[0] = '\0';
  for(i=0;i<REF_NOF_SENSORS;i++) {
    McuUtility_strcatNum16Hex(buf, sizeof(buf), LINE_Get1kValue(i));
    McuUtility_chcat(buf, sizeof(buf), ' ');
  }
  McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
  x = 2;
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());
#endif
  McuGDisplaySSD1306_UpdateFull();
}
#endif

#if PL_CONFIG_HAS_PROXIMITY
static void ShowProxDataGraph(McuFontDisplay_PixelDim x, McuFontDisplay_PixelDim y) {
  #define LCD_PROX_BOX_HEIGHT   (8)
  #define LCD_PROX_BOX_WIDTH    (10)
  #define LCD_PROX_BOX_BORDER   (2)
  PROX_Bits prox;

  prox = PROX_GetProxBits();

  if (prox&PROX_L_LEFT_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  x += LCD_PROX_BOX_BORDER+LCD_PROX_BOX_WIDTH;
  if (prox&PROX_L_MIDDLE_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  x += LCD_PROX_BOX_BORDER+LCD_PROX_BOX_WIDTH;
  if (prox&PROX_L_RIGHT_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  x += LCD_PROX_BOX_BORDER+2*LCD_PROX_BOX_WIDTH;
  if (prox&PROX_R_LEFT_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  x += LCD_PROX_BOX_BORDER+LCD_PROX_BOX_WIDTH;
  if (prox&PROX_R_MIDDLE_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
  x += LCD_PROX_BOX_BORDER+LCD_PROX_BOX_WIDTH;
  if (prox&PROX_R_RIGHT_BIT) {
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, McuGDisplaySSD1306_COLOR_BLUE);
  } else {
    McuGDisplaySSD1306_DrawBox(x, y, LCD_PROX_BOX_WIDTH, LCD_PROX_BOX_HEIGHT, 1, McuGDisplaySSD1306_COLOR_BLUE);
  }
}

static void ShowProximityScreen(void) {
  McuFontDisplay_PixelDim x, y;
  uint8_t buf[24];
  int i;
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
#if LCDMENU_CONFIG_LCD_HEADER_HEIGHT>0
  y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT;
#endif
  McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());

  ShowProxDataGraph(x, y);

  McuGDisplaySSD1306_UpdateFull();
}
#endif

#if PL_CONFIG_HAS_SUMO
static void ShowSumoScreen(void) {
  McuFontDisplay_PixelDim x, y;
  uint8_t buf[24];
  uint16_t ms;

  McuGDisplaySSD1306_Clear();
  McuGDisplaySSD1306_DrawBox(0, 0, McuGDisplaySSD1306_GetWidth(), McuGDisplaySSD1306_GetHeight(), 1, McuGDisplaySSD1306_COLOR_BLUE);

  x = 2; y = 2;
  McuFontDisplay_WriteString((uint8_t*)"Sumo:\n", McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT());

  x = 2;
#if LCDMENU_CONFIG_LCD_HEADER_HEIGHT>0
  y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT;
#endif
  ms = SUMO_GetCountDownMs();
  if (ms>0) {
    McuUtility_strcpy(buf, sizeof(buf), (uint8_t*)"Countdown: ");
    McuUtility_strcatNum16s(buf, sizeof(buf), ms);
    McuUtility_strcat(buf, sizeof(buf), (uint8_t*)"\n");
    McuFontDisplay_WriteString(buf, McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());
  } else if (SUMO_IsDoingSumo()) {
    McuFontDisplay_WriteString((uint8_t*)"Running Sumo....\nPress button to abort.", McuGDisplaySSD1306_COLOR_BLUE, &x, &y, GET_FONT_FIXED());
  }
#if PL_CONFIG_HAS_PROXIMITY
  x = 2;
  ShowProxDataGraph(x, y);
#endif
  McuGDisplaySSD1306_UpdateFull();
}
#endif

static LCDMenu_StatusFlags RobotMenuHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
#if PL_CONFIG_HAS_LCD_HEADER
  if (event==LCDMENU_EVENT_GET_HEADER_TEXT ) {
    return HeaderMenuHandler(item, event, dataP);
  }
#endif
  if (event==LCDMENU_EVENT_ENTER || event==LCDMENU_EVENT_RIGHT) {
#if PL_CONFIG_HAS_QUADRATURE
	  if (item->id==LCD_MENU_ID_ROBOT_ENCODER) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
      #if LCD_USE_ENCODER_AS_INPUT
        LCD_useEncoderForMenuNavigation = FALSE;
      #endif
        ShowEncoderScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_ENCODER;
	  }
#endif
#if PL_CONFIG_HAS_REFLECTANCE
    if (item->id==LCD_MENU_ID_ROBOT_REFLECTANCE) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
      #if LCD_USE_ENCODER_AS_INPUT
        LCD_useEncoderForMenuNavigation = FALSE;
      #endif
        ShowReflectanceScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_REFLECTANCE;
    }
#endif
#if PL_CONFIG_HAS_PROXIMITY
    if (item->id==LCD_MENU_ID_ROBOT_PROXIMITY) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
      #if LCD_USE_ENCODER_AS_INPUT
        LCD_useEncoderForMenuNavigation = FALSE;
      #endif
        ShowProximityScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_PROXIMITY;
    }
#endif
#if PL_CONFIG_HAS_LINE
    if (item->id==LCD_MENU_ID_ROBOT_CALIBRATE) {
    #if LCD_USE_ENCODER_AS_INPUT
      LCD_useEncoderForMenuNavigation = FALSE;
    #endif
      LINE_CalibrateStartStop();
      vTaskDelay(pdMS_TO_TICKS(100)); /* wait some time to get it started, otherwise menu won't refresh properly */
      flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
    }
#endif
#if PL_CONFIG_HAS_SUMO
    if (item->id==LCD_MENU_ID_SUMO_START_STOP) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
      #if LCD_USE_ENCODER_AS_INPUT
        LCD_useEncoderForMenuNavigation = FALSE;
      #endif
        SUMO_StartStopSumo();
        ShowSumoScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_SUMO;
    }
#endif
#if PL_CONFIG_HAS_LINE
    if (item->id==LCD_MENU_ID_ROBOT_CALIBRATE) {
        flags |= LCDMENU_STATUS_FLAGS_HANDLED;
      #if LCD_USE_ENCODER_AS_INPUT
        LCD_useEncoderForMenuNavigation = FALSE;
      #endif
        ShowLineCalibrateScreen();
        LCD_CurrentScreen = LCD_MENU_SCREEN_LINE_CALIBRATE;
    }
#endif
  } else if (event==LCDMENU_EVENT_GET_TEXT && dataP!=NULL) {
#if PL_CONFIG_HAS_LINE
    if (item->id==LCD_MENU_ID_ROBOT_CALIBRATE) {
      if (LINE_IsCalibrating()) {
        *dataP = "Stop Line Calibration";
      } else {
        *dataP = "Start Line Calibration";
      }
      flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
    }
#endif
#if PL_CONFIG_HAS_SUMO
    if (item->id==LCD_MENU_ID_SUMO_START_STOP) {
      if (SUMO_IsDoingSumo()) {
        *dataP = "Stop Sumo";
      } else {
        *dataP = "Start Sumo";
      }
      flags |= LCDMENU_STATUS_FLAGS_HANDLED|LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
    }
#endif
  }
  return flags;
}

static LCDMenu_StatusFlags MenuBackHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
#if PL_CONFIG_HAS_LCD_HEADER
  if (event==LCDMENU_EVENT_GET_HEADER_TEXT ) {
    return HeaderMenuHandler(item, event, dataP);
  }
#endif
  if (event==LCDMENU_EVENT_ENTER) {
    flags |= LCDMENU_STATUS_FLAGS_HANDLED;
    LCDMenu_OnEvent(LCDMENU_EVENT_LEFT, item); /* go up on menu structure */
  }
  return flags;
}

static LCDMenu_StatusFlags MainMenuHandler(const struct LCDMenu_MenuItem_ *item, LCDMenu_EventType event, void **dataP) {
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  (void)item;
#if PL_CONFIG_HAS_LCD_HEADER
  if (event==LCDMENU_EVENT_GET_HEADER_TEXT ) {
    return HeaderMenuHandler(item, event, dataP);
  }
#endif
  return flags;
}

static const LCDMenu_MenuItem menus[] =
{/* id,                               grp,                      pos,                              up,                       down,                             text,           callback                      flags                  */
    {LCD_MENU_ID_GENERAL,             LCD_MENU_GRP_ID_MAIN,     0,                                LCD_MENU_ID_NONE,         LCD_MENU_ID_BACK_GENERAL,            "General",      MainMenuHandler,              LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_BACK_GENERAL,      LCD_MENU_GRP_ID_GENERAL,  0,                                LCD_MENU_ID_GENERAL,      LCD_MENU_ID_NONE,                 "BACK",         MenuBackHandler,              LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_NUM_VALUE,         LCD_MENU_GRP_ID_GENERAL,  1,                                LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 NULL,           ValueChangeHandler,           LCDMENU_MENU_FLAGS_EDITABLE},
    {LCD_MENU_ID_ROBOT,               LCD_MENU_GRP_ID_MAIN,     1,                                LCD_MENU_ID_NONE,         LCD_MENU_ID_BACK_ROBOT,           "Robot",        MainMenuHandler,              LCDMENU_MENU_FLAGS_NONE},
      {LCD_MENU_ID_BACK_ROBOT,        LCD_MENU_GRP_ID_ROBOT,    ROBOT_MENU_POS_BACK_ROBOT,        LCD_MENU_ID_ROBOT,        LCD_MENU_ID_NONE,                 "BACK",         MenuBackHandler,              LCDMENU_MENU_FLAGS_NONE},
#if PL_CONFIG_HAS_SUMO
      {LCD_MENU_ID_SUMO_START_STOP,   LCD_MENU_GRP_ID_ROBOT,    ROBOT_MENU_POS_SUMO_START_STOP,   LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                 NULL,           RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_QUADRATURE
      {LCD_MENU_ID_ROBOT_ENCODER,     LCD_MENU_GRP_ID_ROBOT,     ROBOT_MENU_POS_ENCODER,          LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                "Encoder",      RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_REFLECTANCE
      {LCD_MENU_ID_ROBOT_REFLECTANCE, LCD_MENU_GRP_ID_ROBOT,     ROBOT_MENU_POS_REFLECTANCE,      LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                "Reflectance",  RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_LINE
      {LCD_MENU_ID_ROBOT_CALIBRATE,   LCD_MENU_GRP_ID_ROBOT,     ROBOT_MENU_POS_CALIBRATE,        LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                NULL,           RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
#if PL_CONFIG_HAS_PROXIMITY
      {LCD_MENU_ID_ROBOT_PROXIMITY,   LCD_MENU_GRP_ID_ROBOT,     ROBOT_MENU_POS_PROXIMITY,        LCD_MENU_ID_NONE,         LCD_MENU_ID_NONE,                "Proximity",    RobotMenuHandler,             LCDMENU_MENU_FLAGS_NONE},
#endif
};

static void OnLCDExitScreen(void) {
#if PL_CONFIG_HAS_SUMO
  if (LCD_CurrentScreen==LCD_MENU_SCREEN_SUMO) { /* if leaving Sumo screen: stop running sumo */
    SUMO_StopSumo();
    vTaskDelay(pdMS_TO_TICKS(50)); /* give time to update status */
  }
#endif
#if PL_CONFIG_HAS_SUMO
  if (LCD_CurrentScreen==LCD_MENU_SCREEN_LINE_CALIBRATE) { /* if leaving calibration screen: stop calibration */
    if (LINE_IsCalibrating()) {
      LINE_CalibrateStartStop();
    }
    vTaskDelay(pdMS_TO_TICKS(50)); /* give time to update status */
  }
#endif
  LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
  LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL);
#if LCD_USE_ENCODER_AS_INPUT
  LCD_useEncoderForMenuNavigation = TRUE;
#endif
}

#endif /* PL_CONFIG_HAS_LCD_MENU */

static void LCD_Task(void *param) {
  (void)param; /* not used */
#if LCD_USE_ENCODER_AS_INPUT
  int lastCntrLeft, cntrLeft;
  int lastCntrRight, cntrRight;
#endif

#if PL_CONFIG_HAS_LCD_MENU
  LCD_CurrentScreen = LCD_MENU_SCREEN_NONE;
  LCDMenu_InitMenu(menus, sizeof(menus)/sizeof(menus[0]), LCD_MENU_ID_GENERAL);
  LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, NULL); /* initial refresh */
#endif
#if LCD_USE_ENCODER_AS_INPUT
  LCD_useEncoderForMenuNavigation = TRUE;
  lastCntrLeft = cntrLeft = QUAD_GetLeftPos();
  lastCntrRight = cntrRight = QUAD_GetRightPos();
#endif
  for(;;) {
#if PL_CONFIG_HAS_LCD_MENU
  #if LCD_USE_ENCODER_AS_INPUT
    /* handling encoder/wheels as input device */
    #if PL_CONFIG_HAS_SUMO
    if (SUMO_IsDoingSumo()) { /* disable encoder navigation during Sumo */
      LCD_useEncoderForMenuNavigation = FALSE;
    }
    #endif
    if (LCD_useEncoderForMenuNavigation) {
      cntrLeft = QUAD_GetLeftPos();
      if (cntrLeft > lastCntrLeft+LCD_ENCODER_COUNTER_MENU_DELTA) {
        LCDMenu_OnEvent(LCDMENU_EVENT_LEFT, NULL);
        lastCntrLeft = cntrLeft;
      } else if (cntrLeft < lastCntrLeft-LCD_ENCODER_COUNTER_MENU_DELTA) {
        LCDMenu_OnEvent(LCDMENU_EVENT_RIGHT, NULL);
        lastCntrLeft = cntrLeft;
      }
      cntrRight = QUAD_GetRightPos();
      if (cntrRight > lastCntrRight+LCD_ENCODER_COUNTER_MENU_DELTA) {
        LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
        lastCntrRight = cntrRight;
      } else if (cntrRight < lastCntrRight-LCD_ENCODER_COUNTER_MENU_DELTA) {
        LCDMenu_OnEvent(LCDMENU_EVENT_DOWN, NULL);
        lastCntrRight = cntrRight;
      }
    } else {
      lastCntrLeft = cntrLeft = QUAD_GetLeftPos();
      lastCntrRight = cntrRight = QUAD_GetRightPos();
    }
  #endif

    if (EVNT_EventIsSetAutoClear(EVNT_SW1_LLRELEASED)) {
      if (LCD_CurrentScreen!=LCD_MENU_SCREEN_NONE) { /* key press leaves screen */
        OnLCDExitScreen();
      } else {
        LCDMenu_OnEvent(LCDMENU_EVENT_ENTER, NULL);
      }
    } else if (EVNT_EventIsSetAutoClear(EVNT_SW1_LRELEASED)) {
      if (LCD_CurrentScreen!=LCD_MENU_SCREEN_NONE) { /* key press leaves screen */
        OnLCDExitScreen();
      } else {
        LCDMenu_OnEvent(LCDMENU_EVENT_UP, NULL);
      }
    } else if (EVNT_EventIsSetAutoClear(EVNT_SW1_RELEASED)) {
      if (LCD_CurrentScreen!=LCD_MENU_SCREEN_NONE) { /* key press leaves screen */
        OnLCDExitScreen();
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
#if PL_CONFIG_HAS_SUMO
    if (LCD_CurrentScreen==LCD_MENU_SCREEN_SUMO) {
      ShowSumoScreen();
    }
#endif
#if PL_CONFIG_HAS_LINE
    if (LCD_CurrentScreen==LCD_MENU_SCREEN_LINE_CALIBRATE) {
      ShowLineCalibrateScreen();
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
