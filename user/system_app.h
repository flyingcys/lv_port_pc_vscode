#ifndef system_app_H
#define system_app_H

#include "static_include.h"
#include "stdint.h"


/**
    系统APP托管功能：
        1.屏幕背光
        2.休眠息屏
        3.时间
*/

#define system_rest_sec_default 60

#define nvs_storage_name "sys_app_datas"
#define nvs_version 1, 0, 0      // nvs版本号
#define software_version 1, 0, 8 // 固件版本号
#define software_version_release 0

#define SYS_APP_DEBUG 0

#define power_on_load_app "desktop"

 /*电池信息结构体*/
    typedef struct
    {
        uint8_t soc;
        uint8_t low_soc;
        uint16_t voltage;
        uint16_t rtt;
    } cw2015_bat_info_t;

typedef struct
{
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t week;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
} clock_time_t;

enum
{
    screen_always_on = 0xffffffff,
};

typedef enum
{
    power_on = 0,
    wake_up = 1
} system_wake_t;

void system_get_soft_version(uint8_t *ver);

/**
 * @brief 系统初始化，初始化底层接口，初始化APP
 */
void system_init();

/**
 * @brief 安装系统app
 *
 */
void system_app_install();

/**
 * @brief 系统进入休眠
 */
void system_deep_sleep_start();

/**
 * @brief 关机
 */
void system_power_off();

/**
 * @brief 重启
 */
void system_restart();

/**
 * @brief 获取唤醒模式
 *
 * @return system_wake_t power_on（开机或重启） wake_up（休眠唤醒）
 */
system_wake_t system_get_power_on_mode();

/**
 * @brief 获取GUI增删权限,在要创建和删除控件时，这是必要的，否则将造成宕机
 */
void system_take_gui_key();

/**
 * @brief 归还GUI增删权限，完成创建和删除控件后，必须归还权限让UI工作
 */
void system_give_gui_key();

/**
 * @brief 同步时间到系统，不改变副处理器RTC时间
 *
 * @param time week会自动计算，可以不管
 */
void system_upload_time(clock_time_t time);

/**
 * @brief 同步时间到系统，同时同步副处理器RTC时间
 *
 * @param time week会自动计算，可以不管
 */
void system_set_time(clock_time_t time);

/**
 * @brief 获取系统时间
 *
 * @return clock_time_t
 */
void system_get_time(clock_time_t *time);

/**
 * @brief 屏幕息屏时间计时清零
 */
void system_screen_rest_clr();

/**
 * @brief 设置屏幕休眠时间
 *
 * @param rest_sec s
 */
void system_set_screen_rest(uint32_t rest_sec);

/**
 * @brief 获取屏幕休眠时间
 *
 * @return uint32_t s
 */
uint32_t system_get_screen_rest();

/**
 * @brief 设置屏幕保持常亮
 *
 * @param en 0：按照设定值休眠，1：常亮
 */
void system_screen_keep_on(uint8_t en);

/**
 * @brief 设置屏幕背光
 *
 * @param blk 1-100
 */
void system_set_blk(uint8_t blk);

/**
 * @brief 更新电池信息到系统
 *
 * @param info
 */
void system_upload_bat(cw2015_bat_info_t *info);

/**
 * @brief 获取屏幕背光
 *
 * @return uint8_t 1-100
 */
uint8_t system_get_blk();

/**
 * @brief 设置低电量报警阈值
 *
 * @param low_soc 3-30
 */
void system_set_bat(uint8_t low_soc);

/**
 * @brief 获取电池信息
 */
void system_get_bat(cw2015_bat_info_t *info);

/**
 * @brief 设置振动器震动强度
 *
 * @param var 0-100，0关闭
 */
void system_set_motor(uint8_t var);

/**
 * @brief 获取USB连接状态
 *
 * @return uint8_t 0:未连接，1：已连接
 */
uint8_t system_get_usb_sta();

/**
 * @brief 副处理器指令接口
 *
 * @param cmd 指令名
 * @param dat 数据
 * @param lenth 数据长度，字节
 */
void hc32_trans_send_pack(const char *cmd, uint8_t *dat, size_t lenth);

/**
 * @brief 获取TP操作类型，可用于防止LVGL滑动切换SCREEN时误触按钮
 *
 * @param ret 0：点击操作，1：有滑动的操作
 */
uint8_t system_get_tp_type();


lv_indev_t *system_get_indev_pointer();

#endif
