/* main/src/v9_apple_music/apple_music.c */
#include "apple_music.h"
#include "lvgl/lvgl.h"
#include "am_theme.h"
#include "am_data.h"
#include "am_metrics.h"
#include "am_shell.h"
#include "am_page_home.h"
#include "am_page_list.h"
#include "am_page_settings.h"
#include "am_widgets.h"
#include "am_config.h"
#include "am_player.h"
#include "am_page_local.h"
#include <stdlib.h>   /* getenv */
#include <string.h>

/* lv_obj_set_grid_dsc_array 仅存指针不拷贝,数组必须为文件级静态 */
static int32_t col_dsc[3];
static int32_t row_dsc[3];

typedef enum { AM_PAGE_HOME, AM_PAGE_RADIO, AM_PAGE_LOCAL, AM_PAGE_PLAYLIST, AM_PAGE_SETTINGS } am_page_e;
static am_page_e s_page         = AM_PAGE_HOME;
static int       s_settings_tab = 0;
static am_config_t s_config;
static lv_obj_t *s_root, *s_sidebar, *s_content, *s_player;

static void on_nav(int idx, void *u);
static void on_theme_pick(int idx, void *u);
static void on_tab_pick(int idx, void *u);
static void rebuild_content(void);
static void build_all(void);

/* 用当前激活页填充内容区,并将滚动位置重置到顶部 */
static void rebuild_content(void)
{
    lv_obj_clean(s_content);
    switch(s_page){
        case AM_PAGE_HOME:     am_page_home_create(s_content);                                                break;
        case AM_PAGE_RADIO:    am_page_radio_create(s_content, &s_config);                                    break;
        case AM_PAGE_LOCAL:    am_page_local_create(s_content, &s_config);                                   break;
        case AM_PAGE_PLAYLIST: am_page_list_create(s_content, &am_page_playlist);                             break;
        case AM_PAGE_SETTINGS: am_page_settings_create(s_content, s_settings_tab, on_theme_pick, on_tab_pick, NULL); break;
    }
    lv_obj_scroll_to_y(s_content, 0, LV_ANIM_OFF);   /* 新页面从顶部开始 */
}

/* 全量重建 shell(首次构建 + 主题切换) */
static void build_all(void)
{
    const am_metrics_t *m = am_metrics();
    const am_theme_t *t = am_theme_get(am_theme_current());
    s_root = lv_screen_active();
    lv_obj_clean(s_root);
    lv_obj_remove_style_all(s_root);
    lv_obj_set_scrollbar_mode(s_root, LV_SCROLLBAR_MODE_OFF);
    am_fill_grad2(s_root, t->bg_top, t->bg_bottom);

    /* 根据当前 tier 填充 grid 描述符(指针生命期:文件级静态) */
    col_dsc[0] = m->sidebar_w; col_dsc[1] = LV_GRID_FR(1); col_dsc[2] = LV_GRID_TEMPLATE_LAST;
    row_dsc[0] = LV_GRID_FR(1); row_dsc[1] = m->player_h;  row_dsc[2] = LV_GRID_TEMPLATE_LAST;

    lv_obj_set_grid_dsc_array(s_root, col_dsc, row_dsc);
    lv_obj_set_style_pad_all(s_root, 0, 0);
    lv_obj_set_style_pad_gap(s_root, 0, 0);

    /* 侧栏:col0 row0 跨两行 */
    s_sidebar = lv_obj_create(s_root);
    lv_obj_remove_style_all(s_sidebar);
    lv_obj_set_grid_cell(s_sidebar, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_STRETCH, 0, 2);

    /* 内容区:col1 row0,竖向滚动容器 */
    s_content = lv_obj_create(s_root);
    lv_obj_remove_style_all(s_content);
    lv_obj_set_grid_cell(s_content, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_set_flex_flow(s_content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_scroll_dir(s_content, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(s_content, LV_SCROLLBAR_MODE_AUTO);
    lv_obj_set_style_pad_all(s_content, m->content_pad, 0);

    /* 迷你播放条:col1 row1 */
    s_player = lv_obj_create(s_root);
    lv_obj_remove_style_all(s_player);
    lv_obj_set_grid_cell(s_player, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_STRETCH, 1, 1);

    am_shell_build_sidebar(s_sidebar, (int)s_page, on_nav, NULL);
    am_miniplayer_handles_t h = am_shell_build_miniplayer(s_player);
    am_player_bind_miniplayer(&h);
    rebuild_content();
}

/* 切换页面:重建内容区 + 刷新侧栏激活项 */
static void on_nav(int idx, void *u)
{
    LV_UNUSED(u);
    s_page = (am_page_e)idx;
    lv_obj_clean(s_sidebar);
    am_shell_build_sidebar(s_sidebar, idx, on_nav, NULL);
    rebuild_content();
}

/* 切换主题:全量重建 shell */
static void on_theme_pick(int idx, void *u)
{
    LV_UNUSED(u);
    am_theme_set((am_theme_id_t)idx);
    build_all();
}

/* 切换设置 tab:仅重建内容区 */
static void on_tab_pick(int idx, void *u)
{
    LV_UNUSED(u);
    s_settings_tab = idx;
    rebuild_content();
}

/* 通过环境变量设置初始状态(无头截图/测试用) */
static void apply_env_initial_state(void)
{
    const char *p = getenv("AM_PAGE");
    if(p){
        if(!strcmp(p,"home"))          s_page = AM_PAGE_HOME;
        else if(!strcmp(p,"radio"))    s_page = AM_PAGE_RADIO;
        else if(!strcmp(p,"local"))    s_page = AM_PAGE_LOCAL;
        else if(!strcmp(p,"playlist")) s_page = AM_PAGE_PLAYLIST;
        else if(!strcmp(p,"settings")) s_page = AM_PAGE_SETTINGS;
    }
    const char *th = getenv("AM_THEME");
    if(th){
        if(!strcmp(th,"cyan"))        am_theme_set(AM_THEME_CYAN);
        else if(!strcmp(th,"blue"))   am_theme_set(AM_THEME_BLUE);
        else if(!strcmp(th,"mint"))   am_theme_set(AM_THEME_MINT);
        else if(!strcmp(th,"orange")) am_theme_set(AM_THEME_ORANGE);
    }
    const char *tab = getenv("AM_TAB");
    if(tab){
        if(!strcmp(tab,"appearance"))   s_settings_tab = 0;
        else if(!strcmp(tab,"playback")) s_settings_tab = 1;
        else if(!strcmp(tab,"about"))    s_settings_tab = 2;
    }
}

void apple_music_create(void)
{
    am_config_load(&s_config);   /* 失败时 s_config 保持全零，页面显示占位 */
    am_player_init();
    apply_env_initial_state();
    build_all();
}
