/**
 * @file scenes_page.h
 * @brief Scenes page module entry
 * @version 1.0
 * @date 2026-03-31
 * @copyright Copyright (c) 2026
 */
#ifndef __SCENES_PAGE_H__
#define __SCENES_PAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl/lvgl.h"
#include "scenes_state.h"
#include <stdint.h>

typedef void VOID_T;
typedef int OPERATE_RET;

#define OPRT_OK 0
#define OPRT_INVALID_PARM (-1)

/* ---------------------------------------------------------------------------
 * Type definitions
 * --------------------------------------------------------------------------- */
/**
 * @brief Runtime state for the scenes page root
 */
typedef struct {
    lv_obj_t * screen;
    lv_obj_t * cards[SCENES_CARD_COUNT];
    lv_obj_t * card_titles[SCENES_CARD_COUNT];
    lv_obj_t * card_icons[SCENES_CARD_COUNT];
    SCENES_STATE_T state;
} SCENES_PAGE_CTX_T;

/* ---------------------------------------------------------------------------
 * Function declarations
 * --------------------------------------------------------------------------- */
/**
 * @brief Create the scenes page root container on the active display screen
 * @return OPRT_OK on success, OPRT_INVALID_PARM if LVGL is not ready
 */
OPERATE_RET scenes_page_create(VOID_T);

#ifdef __cplusplus
}
#endif

#endif /* __SCENES_PAGE_H__ */
