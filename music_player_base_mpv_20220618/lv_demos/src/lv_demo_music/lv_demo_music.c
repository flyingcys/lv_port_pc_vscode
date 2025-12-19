/**
 * @file lv_demo_music.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_demo_music.h"

#if LV_USE_DEMO_MUSIC

#include "lv_demo_music_list.h"
#include "lv_demo_music_main.h"
#include <dirent.h>
#include <errno.h>
#include <linux/fcntl.h>
#include <linux/input.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h> //-std=c99  -std=gnu99
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
#if LV_DEMO_MUSIC_AUTO_PLAY
static void auto_step_cb(lv_timer_t *timer);
#endif
#define MUSIC_MAX_NUM 256
#define MUSIC_PATH "./music"
/**********************
 *  STATIC VARIABLES
 **********************/
static lv_obj_t *ctrl;
static lv_obj_t *list;
int music_num = 0;
extern uint32_t _time;
char title_list[MUSIC_MAX_NUM][256] = {};

char *artist_list[MUSIC_MAX_NUM][256] = {};

char *genre_list[MUSIC_MAX_NUM][256] = {};

uint32_t time_list[MUSIC_MAX_NUM] = {};

/**********************
 *      MACROS
 **********************/
int32_t fd_mpv;
struct sigaction act;
pid_t pid;
struct sockaddr_un addr;
void sigusr1(int sig) { exit(0); }
/**********************
 *   GLOBAL FUNCTIONS
 **********************/
pthread_t mthread1;
void *get_exec_data(void *arg)
{
    // close(1);
    char buf[1024];
    // dup2(pip[0], 0); //标准输入重定向到管道输入
    // close(pip[1]);

    while (1)
    {
        if (read(fd_mpv, buf, sizeof(buf)) > 0)
        {
            LOG_D("--->:%s\n", buf);
        }
        usleep(10);
    }
    pthread_exit(NULL);
    LOG_D("pthread exit:\n");
}
pthread_t mthread2;
extern void *get_music_playback_time(void *arg);
void lv_demo_music(void)
{
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x343247), 0);
    // if (list_dir("./music", 3) == 0)
    // {
    //     for (int i = 0; i < 512; i++)
    //     {
    //         LOG_D("%d.%s\t", (i + 1), title_list[i]);
    //         LOG_D("\n");
    //     }
    // }

    music_num = scan_music_list(MUSIC_PATH);
    list = _lv_demo_music_list_create(lv_scr_act());
    ctrl = _lv_demo_music_main_create(lv_scr_act());
    pid = vfork();
    if (pid == 0)
    {
        LOG_D("child pid:%d\n", getpid());
        prctl(PR_SET_PDEATHSIG, SIGKILL);
        execlp("mpv", "mpv", "--quiet", "--no-terminal", "--no-video", "--idle=yes", "--term-status-msg=", "--input-ipc-server=/tmp/mpvsocket", NULL);
        LOG_D("child exit!\n");
        return 0;
    }
    else if (pid > 0)
    {
        LOG_D("parent pid:%d\n", getpid());
        sleep(1);
        close(0);
        act.sa_handler = sigusr1;
        sigfillset(&act.sa_mask);
        act.sa_flags = SA_RESTART; /* don't fiddle with EINTR */
        sigaction(SIGUSR1, &act, NULL);
        addr.sun_family = AF_UNIX;
        strcpy(addr.sun_path, "/tmp/mpvsocket");
        fd_mpv = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fd_mpv == -1)
        {
            LOG_D("Create socket failed\n");
        }
        if (connect(fd_mpv, (struct sockaddr *)&addr, sizeof(addr)) == -1)
        {
            LOG_D("Cannot connect to socket %s\n", addr.sun_path);
        }
        // if (pthread_create(&mthread1, NULL, get_exec_data, NULL) != 0)
        // {
        //     LOG_D("pthread create error!\n");
        //     return 0;
        // }
        // LOG_D("pthread create ok!\n");
        if (pthread_create(&mthread2, NULL, get_music_playback_time, NULL) != 0)
        {
            LOG_D("pthread create error!\n");
            return 0;
        }

        LOG_D("get music pos pthread create ok!\n");
    }
    else
    {
        LOG_D("fork error:\n");
    }

#if LV_DEMO_MUSIC_AUTO_PLAY
    lv_timer_create(auto_step_cb, 1000, NULL);
#endif
}
//获取音乐列表
int idx = 0;

int list_dir(char *path, int depth)
{
    DIR *dir;
    struct dirent *file;
    struct stat st;
    dir = opendir(path);
    if (!dir)
    {
        LOG_D("open dir %s failed!", path);
        return -1;
    }
    LOG_D("open dir %s ok!", path);
    while ((file = readdir(dir)) != NULL)
    {
        if (strncmp(file->d_name, ".", 1) == 0 || strncmp(file->d_name, "..", 2) == 0)
        {
            continue;
        }
        strcpy(title_list[idx++], file->d_name);
        if (stat(file->d_name, &st) >= 0 && S_ISDIR(st.st_mode) && depth <= 5)
        {
            list_dir(file->d_name, depth + 1);
        }
    }
    closedir(dir);
    return 0;
}
void get_music_info(char *name, int idx)
{

    char buf[128];
    sprintf(buf, "soxi -D \"%s/%s\"", MUSIC_PATH, name); //注意文件名空格
    FILE *fp = popen(buf, "r");
    if (ferror(fp))
    {
        LOG_D("error\n");
    }

    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        time_list[idx] = (uint32_t)strtol(buf, NULL, 10);
        LOG_D("scan music length:%d\n", time_list[idx]);
        usleep(1);
    }
}
int scan_music_list(char *_path)
{
    char path[128];
    sprintf(path, "ls %s", _path);
    FILE *fp = popen(path, "r");
    if (ferror(fp))
    {
        LOG_D("error\n");
    }
    char buf[128];
    while (fgets(buf, sizeof(buf), fp) != NULL)
    {
        strncpy(title_list[idx], buf, strlen(buf) - 1); //去掉文件名后的\n
        // get_music_info(title_list[idx], idx);//播放列表不需要显示时间
        idx++;
        usleep(1000);
    }
    return idx;
}
const char *_lv_demo_music_get_title(uint32_t track_id)
{
    if (track_id >= MUSIC_MAX_NUM)
        return NULL;
    return title_list[track_id];
}

const char *_lv_demo_music_get_artist(uint32_t track_id)
{
    if (track_id >= MUSIC_MAX_NUM)
        return NULL;
    return artist_list[track_id];
}

const char *_lv_demo_music_get_genre(uint32_t track_id)
{
    if (track_id >= MUSIC_MAX_NUM)
        return NULL;
    return genre_list[track_id];
}

uint32_t _lv_demo_music_get_track_length(uint32_t track_id)
{
    if (track_id >= MUSIC_MAX_NUM)
        return 0;
    return time_list[track_id];
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#if LV_DEMO_MUSIC_AUTO_PLAY
static void auto_step_cb(lv_timer_t *t)
{
    LV_UNUSED(t);
    LV_FONT_DECLARE(lv_demo_music_font_16_bold)
    static uint32_t state = 0;

#if LV_DEMO_MUSIC_LARGE
    lv_font_t *font_small = &lv_font_montserrat_22;
    lv_font_t *font_large = &lv_font_montserrat_32;
#else
    const lv_font_t *font_small = &lv_font_montserrat_12;
    const lv_font_t *font_large = &lv_font_montserrat_16;
#endif
    switch (state)
    {
    case 5:
        _lv_demo_music_album_next(true);
        break;

    case 6:
        _lv_demo_music_album_next(true);
        break;
    case 7:
        _lv_demo_music_album_next(true);
        break;
    case 8:
        _lv_demo_music_play(0);
        break;
    case 12:
        lv_obj_scroll_by(ctrl, 0, -LV_VER_RES, LV_ANIM_ON);
        break;
    case 14:
        //        lv_obj_scroll_by(ctrl, 0, -LV_VER_RES, LV_ANIM_ON);
        break;
    case 15:
        lv_obj_scroll_by(list, 0, -300, LV_ANIM_ON);
        break;
    case 16:
        lv_obj_scroll_by(list, 0, 300, LV_ANIM_ON);
        break;
        //        lv_anim_init(&a);
        //        lv_anim_set_var(&a, list);
        //        lv_anim_set_values(&a, lv_obj_get_y(list), -lv_obj_get_height(list) + LV_DEMO_MUSIC_HANDLE_SIZE);
        //        lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t) lv_obj_set_y);
        //        lv_anim_set_time(&a, 1000);
        //        lv_anim_set_playback_time(&a, 1000);
        //        lv_anim_set_playback_delay(&a, 200);
        //        lv_anim_start(&a);
        break;
    case 18:
        _lv_demo_music_play(1);
        break;
    case 19:
        lv_obj_scroll_by(ctrl, 0, LV_VER_RES, LV_ANIM_ON);
        break;
    case 30:
        _lv_demo_music_play(2);
        break;
    case 40:
    {
        lv_obj_t *bg = lv_layer_top();
        lv_obj_set_style_bg_color(bg, lv_color_hex(0x6f8af6), 0);
        lv_obj_set_style_text_color(bg, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(bg, LV_OPA_COVER, 0);
        lv_obj_fade_in(bg, 400, 0);
        lv_obj_t *dsc = lv_label_create(bg);
        lv_obj_set_style_text_font(dsc, font_small, 0);
        lv_label_set_text(dsc, "The average FPS is");
        lv_obj_align(dsc, LV_ALIGN_TOP_MID, 0, 90);

        lv_obj_t *num = lv_label_create(bg);
        lv_obj_set_style_text_font(num, font_large, 0);
#if LV_USE_PERF_MONITOR
        lv_label_set_text_fmt(num, "%d", lv_refr_get_fps_avg());
#endif
        lv_obj_align(num, LV_ALIGN_TOP_MID, 0, 120);

        lv_obj_t *attr = lv_label_create(bg);
        lv_obj_set_style_text_align(attr, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_font(attr, font_small, 0);
#if LV_DEMO_MUSIC_SQUARE
        lv_label_set_text(attr, "Copyright 2020 LVGL Kft.\nwww.lvgl.io | lvgl@lvgl.io");
#else
        lv_label_set_text(attr, "Copyright 2020 LVGL Kft. | www.lvgl.io | lvgl@lvgl.io");
#endif
        lv_obj_align(attr, LV_ALIGN_BOTTOM_MID, 0, -10);
        break;
    }
    case 41:
        lv_scr_load(lv_obj_create(NULL));
        _lv_demo_music_pause();
        break;
    }
    state++;
}

#endif /*LV_DEMO_MUSIC_AUTO_PLAY*/

#endif /*LV_USE_DEMO_MUSIC*/
