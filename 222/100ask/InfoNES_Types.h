/*===================================================================*/
/*                                                                   */
/*  InfoNES_Types.h : Type definitions for InfoNES                   */
/*                                                                   */
/*  2000/5/4    InfoNES Project ( based on pNesX )                   */
/*                                                                   */
/*===================================================================*/

#ifndef InfoNES_TYPES_H_INCLUDED
#define InfoNES_TYPES_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include "lv_100ask_nes.h"

/*-------------------------------------------------------------------*/
/*  Type definition                                                  */
/*-------------------------------------------------------------------*/
#ifndef DWORD
typedef unsigned long  DWORD;
#endif /* !DWORD */

#ifndef WORD
    #if LV_COLOR_DEPTH == 16
    //typedef unsigned short WORD;
    #define WORD unsigned short
    #elif LV_COLOR_DEPTH == 32
    //typedef unsigned int WORD;
    #define WORD unsigned int
    #endif
#endif /* !WORD */

#ifndef BYTE
typedef unsigned char  BYTE;
#endif /* !BYTE */

/*-------------------------------------------------------------------*/
/*  NULL definition                                                  */
/*-------------------------------------------------------------------*/
#ifndef NULL
#define NULL  0
#endif /* !NULL */


#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /* !InfoNES_TYPES_H_INCLUDED */
