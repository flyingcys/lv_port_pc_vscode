
#ifndef InfoNES_MAPPER_003_H_INCLUDED
#define InfoNES_MAPPER_003_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/*-------------------------------------------------------------------*/
/*  Include files                                                    */
/*-------------------------------------------------------------------*/

#include "../InfoNES_Types.h"

/*-------------------------------------------------------------------*/
/*  Function prototypes                                              */
/*-------------------------------------------------------------------*/

void Map3_Init();
void Map3_Write( WORD wAddr, BYTE byData );

#endif  /*LV_USE_100ASK_NES*/

#ifdef __cplusplus
} /*extern "C"*/
#endif

