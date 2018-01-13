/**
 * \file
 * \brief Hierarchical Menu System
 * \author Erich Styger, erich.styger@hslu.ch
 *
 * This module implements a text menu for small displays
 */

#include "Platform.h"
#if PL_CONFIG_HAS_LCD_MENU
#include "LCDMenu.h"
#include "McuGDisplaySSD1306.h"
#include "McuFontDisplay.h"
#include "McuFontHelv08Normal.h"

/* LCD specific constants */
#define LCDMENU_SUBMENU_INDICATOR_CHAR    '>' /* sub-menu indicator */
#define LCDMENU_UPMENU_INDICATOR_CHAR     '<' /* up-menu indicator */
#define LCDMENU_V_SCROLLBAR_WIDTH         1   /* vertical scroll bar width */

#define LCDMENU_COLOR_TEXT_FG_NORMAL      McuGDisplaySSD1306_COLOR_BLUE  /* normal text foreground color */
#define LCDMENU_COLOR_TEXT_BG_NORMAL      McuGDisplaySSD1306_COLOR_BLACK /* normal text background color */
#define LCDMENU_COLOR_TEXT_FG_HIGHLIGHT   McuGDisplaySSD1306_COLOR_BLACK /* highlighted text foreground */
#define LCDMENU_COLOR_TEXT_BG_HIGHLIGHT   McuGDisplaySSD1306_COLOR_BLUE  /* highlighted text background */
#define LCDMENU_COLOR_SCROLL_BAR          McuGDisplaySSD1306_COLOR_BLUE  /* color for side scroll bar */
#define LCDMENU_GET_FONT()                McuFontHelv08Normal_GetFont()  /* font used for menus */

typedef struct {
  const LCDMenu_MenuItem *menus; /* pointer to array of menu items */
  uint16_t nofMenuItems;   /* number of menu items */
  uint8_t selectedID;  /* currently selected menu ID */
  uint8_t editID;  /* currently edited ID, or LCDMENU_ID_NONE if not in edit mode */
  uint8_t topGroup, topPos;   /* display top position */
} LCDMenu_Status;

static LCDMenu_Status menuStatus;

void LCDMenu_InitMenu(const LCDMenu_MenuItem *menus, uint8_t nofMenuItems, uint8_t selectedID) {
  menuStatus.menus = menus;
  menuStatus.nofMenuItems = nofMenuItems;
  menuStatus.selectedID = selectedID;
  menuStatus.editID = LCDMENU_ID_NONE;
  menuStatus.topGroup = LCDMENU_GROUP_ROOT;
  menuStatus.topPos = 0;
}

static const LCDMenu_MenuItem *LCDMenu_GetGroupPosMenuItem(uint8_t group, uint8_t pos) {
  int i;

  i = 0;
  while(i<menuStatus.nofMenuItems) {
    if (menuStatus.menus[i].group==group && menuStatus.menus[i].pos==pos) { /* match */
      return &menuStatus.menus[i];
    }
    i++;
  }
  return NULL; /* not found */
}

static const LCDMenu_MenuItem *LCDMenu_GeIdMenuItem(uint8_t id) {
  int i;

  i = 0;
  while(i<menuStatus.nofMenuItems) {
    if (menuStatus.menus[i].id==id) { /* match */
      return &menuStatus.menus[i];
    }
    i++;
  }
  return NULL; /* not found */
}

static uint16_t LCDMenu_NofMenuItemsInGroup(uint8_t group) {
  int i;
  uint16_t cntr;

  i = 0;
  cntr = 0;
  while(i<menuStatus.nofMenuItems) {
    if (menuStatus.menus[i].group==group) { /* match */
      cntr++;
    }
    i++;
  }
  return cntr; /* return number of items found */
}

static unsigned int LCDMenu_MaxNofMenuItems(void) {
  PGFONT_Callbacks font;
  unsigned int nofMaxMenuItems;
  McuFontDisplay_PixelDim charHeight, fontHeight;

  font = LCDMENU_GET_FONT();
  McuFontDisplay_GetFontHeight(font, &charHeight, &fontHeight);
  nofMaxMenuItems = (McuGDisplaySSD1306_GetHeight()-LCDMENU_CONFIG_LCD_HEADER_HEIGHT)/fontHeight;
  return nofMaxMenuItems;
}

static void LCDMenu_Draw(void) {
  PGFONT_Callbacks font;
  McuFontDisplay_PixelDim x, y;
  McuFontDisplay_PixelDim charHeight, fontHeight;
  int i;
  unsigned int nofMaxMenuItems;
  const LCDMenu_MenuItem *item;
  uint8_t group, pos;
  uint8_t *text;
  McuGDisplaySSD1306_PixelColor textColor;
  McuFontDisplay_PixelDim charWidth, totalWidth;
  McuFontDisplay_PixelDim scrollBarWidth;
  uint16_t nofTotalMenusOnLevel;

  font = LCDMENU_GET_FONT();
  McuFontDisplay_GetFontHeight(font, &charHeight, &fontHeight);
  nofMaxMenuItems = LCDMenu_MaxNofMenuItems();
  McuGDisplaySSD1306_Clear(); /* clear display */
  group = menuStatus.topGroup;
  pos = menuStatus.topPos;
  nofTotalMenusOnLevel = LCDMenu_NofMenuItemsInGroup(group);
  if (nofTotalMenusOnLevel>nofMaxMenuItems) { /* show scroll-bar only if needed */
    McuFontDisplay_PixelDim h;

    scrollBarWidth = LCDMENU_V_SCROLLBAR_WIDTH+1; /* plus one for a border to the left */
    x = McuGDisplaySSD1306_GetWidth()-LCDMENU_V_SCROLLBAR_WIDTH;
    y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT + ((McuGDisplaySSD1306_GetHeight()-LCDMENU_CONFIG_LCD_HEADER_HEIGHT)*menuStatus.topPos)/nofTotalMenusOnLevel;
    h = ((McuGDisplaySSD1306_GetHeight()-LCDMENU_CONFIG_LCD_HEADER_HEIGHT)*nofMaxMenuItems)/nofTotalMenusOnLevel; /* h proportional to the items visible */
    McuGDisplaySSD1306_DrawFilledBox(x, y, LCDMENU_V_SCROLLBAR_WIDTH, h, LCDMENU_COLOR_SCROLL_BAR);
  } else {
    scrollBarWidth = 0; /* no scroll-bar */
  }
#if LCDMENU_CONFIG_LCD_HEADER_HEIGHT!=0
  McuGDisplaySSD1306_DrawBox(0, 0, McuGDisplaySSD1306_GetWidth(), LCDMENU_CONFIG_LCD_HEADER_HEIGHT, 1, LCDMENU_COLOR_TEXT_FG_NORMAL);
  x = 2; y = 3;
  item = LCDMenu_GeIdMenuItem(menuStatus.selectedID);
  if (item!=NULL && item->handler!=NULL) {
    text = NULL;
    (void)item->handler(item, LCDMENU_EVENT_GET_HEADER_TEXT, (void**)&text);
    if (text!=NULL) {
      McuFontDisplay_WriteString((unsigned char*)text, LCDMENU_COLOR_TEXT_FG_NORMAL, &x, &y, font); /* write header */
    }
  }
#endif
  x = 0;
  y = LCDMENU_CONFIG_LCD_HEADER_HEIGHT+1; /* have a small border on top of the text */
  for(i=0; i<nofMaxMenuItems; i++) {
    item = LCDMenu_GetGroupPosMenuItem(group, pos);
    if (item!=NULL) {
      x = 0;
      text = (uint8_t*)item->menuText;
      if (text==NULL && item->handler!=NULL) {
         text = NULL;
         if (item->id==menuStatus.editID) { /* it's for the item in edit mode */
           (void)item->handler(item, LCDMENU_EVENT_GET_EDIT_TEXT, (void**)&text);
         } else {
           (void)item->handler(item, LCDMENU_EVENT_GET_TEXT, (void**)&text);
         }
         if (text==NULL) {
           text = (uint8_t*)"";
         }
      }
      if (item->id == menuStatus.selectedID) { /* selected item */
        McuGDisplaySSD1306_DrawFilledBox(x, y-1, McuGDisplaySSD1306_GetWidth()-scrollBarWidth, fontHeight, LCDMENU_COLOR_TEXT_BG_HIGHLIGHT); /* -1 because of small border */
        textColor = LCDMENU_COLOR_TEXT_FG_HIGHLIGHT; /* selection is black text on blue background */
      } else {
        textColor = LCDMENU_COLOR_TEXT_FG_NORMAL;
      }
      /* level up menu indicator */
      if (item->lvlUpID!=LCDMENU_ID_NONE) { /* there is a upper level menu: write up indicator */
        if (menuStatus.editID==item->id) { /* currently editing the item */
          /* for edited item, do not write indicator: skip space for it */
          McuFontDisplay_GetCharWidth(LCDMENU_UPMENU_INDICATOR_CHAR, &charWidth, &totalWidth, font);
          x += totalWidth;
        } else {
          McuFontDisplay_WriteChar(LCDMENU_UPMENU_INDICATOR_CHAR, textColor, &x, &y, font);
        }
      } else if (item->group!=LCDMENU_GROUP_ROOT) { /* skip space, but not for root menu */
        McuFontDisplay_GetCharWidth(LCDMENU_UPMENU_INDICATOR_CHAR, &charWidth, &totalWidth, font);
        x += totalWidth;
      }
      McuFontDisplay_WriteString(text, textColor, &x, &y, font); /* write menu text */
      if (item->lvlDownID!=LCDMENU_ID_NONE) { /* menu entry with a sub-menu entry: write sub-menu indicator */
        /* sub-menu indicator */
        McuFontDisplay_GetCharWidth(LCDMENU_SUBMENU_INDICATOR_CHAR, &charWidth, &totalWidth, font);
        x = McuGDisplaySSD1306_GetWidth()-charWidth-scrollBarWidth; /* display width */
        McuFontDisplay_WriteChar(LCDMENU_SUBMENU_INDICATOR_CHAR, textColor, &x, &y, font);
      }
      y += fontHeight;
    }
    pos++;
  }
  McuGDisplaySSD1306_UpdateFull();
}

static void LCDMenu_CursorUp(void) {
  const LCDMenu_MenuItem *item;
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  item = LCDMenu_GeIdMenuItem(menuStatus.selectedID); /* get current item */
  if (item!=NULL) {
    if (item->handler!=NULL) { /* custom handler present */
      flags |= item->handler(item, LCDMENU_EVENT_UP, NULL); /* call handler with event */
      if ((item->flags&LCDMENU_MENU_FLAGS_EDITABLE) && menuStatus.editID==menuStatus.selectedID) { /* editable item and editing it? */
        flags |= item->handler(item, LCDMENU_EVENT_INCREMENT, NULL); /* send edit event */
      }
    }
    if (menuStatus.editID==LCDMENU_ID_NONE) { /* we are currently not editing it */
      if (item->pos>0) {
        item = LCDMenu_GetGroupPosMenuItem(item->group, item->pos-1); /* get next possible item */
        /* returns NULL if not found */
      } else {
        item = NULL; /* no previous item in list */
      }
      if (item !=NULL) {
        menuStatus.selectedID = item->id;
        if (item->pos<menuStatus.topPos) { /* check if outside visible area */
          menuStatus.topPos = item->pos;
        }
        flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
#if LCDMENU_CONFIG_MENU_WRAP_AROUND
      } else { /* first item in list: wrap around to last item in group */
        int pos, nofPossibleMenuItems;

        pos = LCDMenu_NofMenuItemsInGroup(menuStatus.topGroup); /* find out how many items we have in group */
        if (pos>0) { /* sanity check, should be larger than zero */
          pos--; /* position of last menu item in group */
          item = LCDMenu_GetGroupPosMenuItem(menuStatus.topGroup, pos); /* get item */
          if (item!=NULL) { /* sanity check */
            nofPossibleMenuItems = LCDMenu_MaxNofMenuItems();
            if (pos >= nofPossibleMenuItems) { /* is the full menu visible? */
              menuStatus.topPos = pos - nofPossibleMenuItems + 1;
            } else {
              menuStatus.topPos = 0;
            }
            menuStatus.selectedID = item->id;
          }
          flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
        }
#endif
      }
    }
    if (flags&LCDMENU_STATUS_FLAGS_UPDATE_VIEW) { /* need to update view? */
      LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, item);
    }
  }
}

static void LCDMenu_CursorDown(void) {
  const LCDMenu_MenuItem *item;
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  item = LCDMenu_GeIdMenuItem(menuStatus.selectedID); /* get current item */
  if (item!=NULL) {
    if (item->handler!=NULL) { /* custom handler present */
      flags |= item->handler(item, LCDMENU_EVENT_DOWN, NULL); /* call handler with event */
      if ((item->flags&LCDMENU_MENU_FLAGS_EDITABLE) && menuStatus.editID==menuStatus.selectedID) { /* editable item and editing it? */
        flags |= item->handler(item, LCDMENU_EVENT_DECREMENT, NULL); /* send edit event */
      }
    }
    if (menuStatus.editID==LCDMENU_ID_NONE) { /* we are currently not editing it. */
      item = LCDMenu_GetGroupPosMenuItem(item->group, item->pos+1); /* get next possible item */
      /* returns NULL if not found */
      if (item!=NULL) { /* yes, it exists */
        unsigned int maxNofMenuItems = LCDMenu_MaxNofMenuItems();
        menuStatus.selectedID = item->id;
        if (item->pos>=maxNofMenuItems) { /* check if outside visible area */
          menuStatus.topPos = item->pos-maxNofMenuItems+1;
        }
        flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
#if LCDMENU_CONFIG_MENU_WRAP_AROUND
      } else { /* last item in list: wrap around to first item in group */
        item = LCDMenu_GeIdMenuItem(menuStatus.selectedID); /* get current selected item */
        if (item!=NULL && item->pos>0) {
          item = LCDMenu_GetGroupPosMenuItem(item->group, 0); /* get first item in list with position 0 */
          if (item!=NULL) {
            menuStatus.topPos = 0;
            menuStatus.selectedID = item->id;
            flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
          }
        }
#endif
      }
    }
    if (flags&LCDMENU_STATUS_FLAGS_UPDATE_VIEW) { /* need to update view? */
      LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, item);
    }
  }
}

static void LCDMenu_CursorRight(void) {
  const LCDMenu_MenuItem *item;
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  item = LCDMenu_GeIdMenuItem(menuStatus.selectedID); /* get current item */
  if (item!=NULL) {
    if (item->handler!=NULL) { /* custom handler present? */
      flags |= item->handler(item, LCDMENU_EVENT_RIGHT, NULL); /* send notification */
      if ((item->flags&LCDMENU_MENU_FLAGS_EDITABLE) && menuStatus.editID==menuStatus.selectedID) { /* editable item and already editing it? */
        flags |= item->handler(item, LCDMENU_EVENT_INCREMENT, NULL); /* send edit event */
      }
    }
    if (menuStatus.editID==LCDMENU_ID_NONE && item->lvlDownID!=LCDMENU_ID_NONE) { /* we are currently not editing it, and it has a sub menu item? go down menu structure */
      item = LCDMenu_GeIdMenuItem(item->lvlDownID); /* get target item */
      menuStatus.selectedID = item->id;
      menuStatus.topGroup = item->group;
      menuStatus.topPos = 0;
      flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
    }
    if (flags&LCDMENU_STATUS_FLAGS_UPDATE_VIEW) { /* need to update view? */
      LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, item);
    }
  }
}

static void LCDMenu_CursorLeft(void) {
  const LCDMenu_MenuItem *item;
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  item = LCDMenu_GeIdMenuItem(menuStatus.selectedID); /* get current item */
  if (item!=NULL) {
    if (item->handler!=NULL) { /* custom handler present? */
      flags |= item->handler(item, LCDMENU_EVENT_LEFT, NULL); /* send notification */
      if ((item->flags&LCDMENU_MENU_FLAGS_EDITABLE) && menuStatus.editID==menuStatus.selectedID) { /* editable item and editing it? */
        flags |= item->handler(item, LCDMENU_EVENT_DECREMENT, NULL); /* send edit event */
      }
    }
    if (menuStatus.editID==LCDMENU_ID_NONE && item->lvlUpID!=LCDMENU_ID_NONE) { /* we are currently not ediging, and havinig a menu on the upper level? */
      unsigned int maxNofMenuItems = LCDMenu_MaxNofMenuItems();
      item = LCDMenu_GeIdMenuItem(item->lvlUpID); /* get target item */
      menuStatus.selectedID = item->id;
      menuStatus.topGroup = item->group;
      if (item->pos>=maxNofMenuItems) { /* check if outside visible area */
        menuStatus.topPos = item->pos-maxNofMenuItems+1;
      } else {
        menuStatus.topPos = 0;
      }
      flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
    }
    if (flags&LCDMENU_STATUS_FLAGS_UPDATE_VIEW) { /* need to update view? */
      LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, item);
    }
  }
}

static void LCDMenu_CursorEnter(void) {
  const LCDMenu_MenuItem *item;
  LCDMenu_StatusFlags flags = LCDMENU_STATUS_FLAGS_NONE;

  item = LCDMenu_GeIdMenuItem(menuStatus.selectedID); /* get currently selected menu item */
  if (item!=NULL) {
    /* process ENTER action */
    if (item->handler!=NULL) { /* send notification to handler */
      flags |= item->handler(item, LCDMENU_EVENT_ENTER, NULL);
    }
    if (menuStatus.editID==LCDMENU_ID_NONE && item->lvlDownID!=LCDMENU_ID_NONE) { /* we are not editing it, and enter on a menu item with a sub-menu: go down in menu hierarchy */
      LCDMenu_OnEvent(LCDMENU_EVENT_RIGHT, NULL); /* emulate key: go down one menu level */
    } else if (item->flags&LCDMENU_MENU_FLAGS_EDITABLE) { /* enter on an editable item? */
      if (menuStatus.editID==menuStatus.selectedID) { /* is it already in edit mode? */
        menuStatus.editID = LCDMENU_ID_NONE; /* de-select it for edit */
        flags |= item->handler(item, LCDMENU_EVENT_EXIT_EDIT, NULL);
        flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
      } else if (menuStatus.editID!=menuStatus.selectedID) {
        menuStatus.editID = menuStatus.selectedID; /* select it for edit */
        flags |= item->handler(item, LCDMENU_EVENT_ENTER_EDIT, NULL);
        flags |= LCDMENU_STATUS_FLAGS_UPDATE_VIEW;
      }
    }
    if (flags&LCDMENU_STATUS_FLAGS_UPDATE_VIEW) { /* redraw needed? */
      LCDMenu_OnEvent(LCDMENU_EVENT_DRAW, item); /* entered edit mode, redraw menu */
    }
  }
}

void LCDMenu_OnEvent(LCDMenu_EventType event, const LCDMenu_MenuItem *menu) {
  (void)menu;
  switch(event) {
    case LCDMENU_EVENT_INIT:
      break;
    case LCDMENU_EVENT_DRAW:
      LCDMenu_Draw();
      break;
    case LCDMENU_EVENT_UP:
      LCDMenu_CursorUp();
      break;
    case LCDMENU_EVENT_DOWN:
      LCDMenu_CursorDown();
      break;
    case LCDMENU_EVENT_LEFT:
      LCDMenu_CursorLeft();
      break;
    case LCDMENU_EVENT_RIGHT:
      LCDMenu_CursorRight();
      break;
    case LCDMENU_EVENT_ENTER:
      LCDMenu_CursorEnter();
      break;
    default:
      break;
  }
}

void LCDMenu_Init(void) {
  menuStatus.topGroup = LCDMENU_GROUP_ROOT;
  menuStatus.topPos = 0;
  menuStatus.selectedID = 1;
}

#endif /* PL_CONFIG_HAS_LCD_MENU */
