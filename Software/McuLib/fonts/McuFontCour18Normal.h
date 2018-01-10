/* ###################################################################
**     This component module is generated by Processor Expert. Do not modify it.
**     Filename    : McuFontCour18Normal.h
**     Project     : FRDM-K64F_Generator
**     Processor   : MK64FN1M0VLL12
**     Component   : GFont
**     Version     : Component 01.125, Driver 01.00, CPU db: 3.00.000
**     Repository  : Legacy User Components
**     Compiler    : GNU C Compiler
**     Date/Time   : 2017-01-16, 14:34, # CodeGen: 142
**     Abstract    :
**
**     Settings    :
**          Component name                                 : McuFontCour18Normal
**          SDK                                            : McuLib
**          Overwrite Bounding Box Height                  : Disabled
**          Name                                           : Cour
**          Size                                           : 18
**          Style                                          : normal
**     Contents    :
**         GetFontChar           - PGFONT_CharInfo McuFontCour18Normal_GetFontChar(uint8_t ch);
**         GetBoxHeight          - uint8_t McuFontCour18Normal_GetBoxHeight(void);
**         GetFont               - PGFONT_Callbacks McuFontCour18Normal_GetFont(void);
**         GetUnderlineBoxHeight - uint8_t McuFontCour18Normal_GetUnderlineBoxHeight(void);
**         GetLineSpaceHeight    - uint8_t McuFontCour18Normal_GetLineSpaceHeight(void);
**
**     License   :  Open Source (LGPL)
**     Copyright : (c) Copyright Erich Styger, 2011, all rights reserved.
**     This an open source software in the form of a Processor Expert Embedded Component.
**     This is a free software and is opened for education,  research  and commercial developments under license policy of following terms:
**     * This is a free software and there is NO WARRANTY.
**     * No restriction on use. You can use, modify and redistribute it for personal, non-profit or commercial product UNDER YOUR RESPONSIBILITY.
**     * Redistributions of source code must retain the above copyright notice.
** ###################################################################*/
/*!
** @file McuFontCour18Normal.h
** @version 01.00
** @brief
**
*/         
/*!
**  @addtogroup McuFontCour18Normal_module McuFontCour18Normal module documentation
**  @{
*/         

#ifndef __McuFontCour18Normal_H
#define __McuFontCour18Normal_H

/* MODULE McuFontCour18Normal. */
#include "McuLib.h" /* SDK and API used */
#include "McuFontCour18Normalconfig.h" /* configuration */

/* Include inherited components */
#include "McuLib.h"


#ifndef __BWUserType_PConstByte
#define __BWUserType_PConstByte
  typedef const uint8_t *PConstByte;   /* Pointer to a constant uint8_t. */
#endif
#ifndef __BWUserType_GFONT_CharInfo
#define __BWUserType_GFONT_CharInfo
  typedef struct {                     /* A descriptor describing the font. */
    int8_t dwidth;                     /* position of next char. relative to the current */
    uint8_t width;                     /* width of character in pixels */
    uint8_t height;                    /* height of character in pixels */
    int8_t offsetX;                    /* horizontal offset to left edge of bounding box */
    int8_t offsetY;                    /* vertical offset to bottom edge of bounding box */
    PConstByte CharBMP;                /* pointer to character bitmap defined above */
  } GFONT_CharInfo;
#endif
#ifndef __BWUserType_PGFONT_CharInfo
#define __BWUserType_PGFONT_CharInfo
  typedef GFONT_CharInfo* PGFONT_CharInfo ; /* A pointer to GFONT_CharInfo */
#endif
#ifndef __BWUserType_CallbackGetFontChar
#define __BWUserType_CallbackGetFontChar
  typedef PGFONT_CharInfo (*CallbackGetFontChar)(uint8_t ch); /* Callback used to get the font information for a single character. */
#endif
#ifndef __BWUserType_GFONT_Callbacks
#define __BWUserType_GFONT_Callbacks
  typedef struct {                     /* Structure with font relevant callbacks. */
    CallbackGetFontChar GetFontChar;   /* Callback for retrieving a font character. */
    uint8_t boundingBoxHeight;         /* Height of the bounding box. This includes the height of the underline box height. */
    uint8_t underlineBoxHeight;        /* Height of the underline box height. */
    uint8_t lineSpaceBoxHeight;        /* Height of the space between lines */
  } GFONT_Callbacks;
#endif
#ifndef __BWUserType_PGFONT_Callbacks
#define __BWUserType_PGFONT_Callbacks
  typedef GFONT_Callbacks* PGFONT_Callbacks ; /* Pointer to struct for font callbacks. */
#endif



PGFONT_CharInfo McuFontCour18Normal_GetFontChar(uint8_t ch);
/*
** ===================================================================
**     Method      :  McuFontCour18Normal_GetFontChar (component GFont)
**     Description :
**         Returns for a given character the corresponding font bitmap.
**     Parameters  :
**         NAME            - DESCRIPTION
**         ch              - The character for that a bitmap is required.
**     Returns     :
**         ---             - Character font bitmap
** ===================================================================
*/

uint8_t McuFontCour18Normal_GetBoxHeight(void);
/*
** ===================================================================
**     Method      :  McuFontCour18Normal_GetBoxHeight (component GFont)
**     Description :
**         Function to get the height of the bounding box.
**     Parameters  : None
**     Returns     :
**         ---             - Height of bounding box
** ===================================================================
*/

PGFONT_Callbacks McuFontCour18Normal_GetFont(void);
/*
** ===================================================================
**     Method      :  McuFontCour18Normal_GetFont (component GFont)
**     Description :
**         Returns callbacks for fonts to be used by the font driver.
**     Parameters  : None
**     Returns     :
**         ---             - Font callbacks.
** ===================================================================
*/

#define McuFontCour18Normal_GetLineSpaceHeight() \
  2

/*
** ===================================================================
**     Method      :  McuFontCour18Normal_GetLineSpaceHeight (component GFont)
**     Description :
**         Returns the size of the line space height
**     Parameters  : None
**     Returns     :
**         ---             - Error code
** ===================================================================
*/

#define McuFontCour18Normal_GetUnderlineBoxHeight() \
  8

/*
** ===================================================================
**     Method      :  McuFontCour18Normal_GetUnderlineBoxHeight (component GFont)
**     Description :
**         Returns the size of the underline box height
**     Parameters  : None
**     Returns     :
**         ---             - Error code
** ===================================================================
*/

/* END McuFontCour18Normal. */

#endif
/* ifndef __McuFontCour18Normal_H */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/
