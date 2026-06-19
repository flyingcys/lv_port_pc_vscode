/* main/src/v9_apple_music/am_config.h */
#ifndef AM_CONFIG_H
#define AM_CONFIG_H

#define AM_CONFIG_RADIO_MAX 16

typedef struct {
    char title[128];
    char subtitle[128];
    char url[512];
} am_radio_item_t;

typedef struct {
    char local_dir[512];
    am_radio_item_t radio[AM_CONFIG_RADIO_MAX];
    int  radio_count;
} am_config_t;

/* 从指定路径加载配置。返回 0=成功, -1=文件不存在, -2=解析失败 */
int  am_config_load_from_path(const char *path, am_config_t *out);

/* 按优先级查找配置文件（$AM_CONFIG → ./am_config.json → ~/.config/apple_music/config.json）*/
int  am_config_load(am_config_t *out);

#endif /* AM_CONFIG_H */
