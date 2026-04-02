#ifndef DEVICE_STATE_H
#define DEVICE_STATE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define DEVICE_STATE_SWITCH_COUNT 2U
#define DEVICE_STATE_MAX_SCENES 8U

typedef enum {
    DEVICE_AUTO_SLEEP_OFF = 0,
    DEVICE_AUTO_SLEEP_15_SECONDS,
    DEVICE_AUTO_SLEEP_30_SECONDS,
    DEVICE_AUTO_SLEEP_1_MINUTE,
    DEVICE_AUTO_SLEEP_5_MINUTES,
    DEVICE_AUTO_SLEEP_15_MINUTES,
    DEVICE_AUTO_SLEEP_COUNT
} device_auto_sleep_option_t;

typedef enum {
    DEVICE_VOICE_STATE_IDLE = 0,
    DEVICE_VOICE_STATE_LISTENING,
    DEVICE_VOICE_STATE_THINKING,
    DEVICE_VOICE_STATE_SPEAKING,
    DEVICE_VOICE_STATE_MUTED
} device_voice_state_t;

typedef struct {
    char name[32];
    bool is_on;
} device_switch_state_t;

typedef struct {
    char name[32];
    bool enabled;
    uint8_t icon_variant;
} device_scene_state_t;

typedef struct {
    uint8_t hour;
    uint8_t minute;
    bool use_24h;
    uint8_t month;
    uint8_t day;
    uint8_t weekday_index;
    int temperature_c;
    char weather_summary[16];
    uint8_t wifi_signal_level;
    char wifi_ssid[32];
    device_switch_state_t switches[DEVICE_STATE_SWITCH_COUNT];
    size_t scene_count;
    device_scene_state_t scenes[DEVICE_STATE_MAX_SCENES];
    uint8_t brightness_percent;
    bool auto_sleep_enabled;
    device_auto_sleep_option_t auto_sleep_option;
    bool proximity_wake_enabled;
    device_voice_state_t voice_state;
    char voice_input_text[96];
    char voice_output_text[96];
    bool muted;
    uint8_t volume_percent;
} device_state_t;

void device_state_init_defaults(device_state_t *state);
void device_state_format_time(const device_state_t *state, char *buf, size_t buf_size);
void device_state_format_date(const device_state_t *state, char *buf, size_t buf_size);
const char *device_state_get_weekday_short(uint8_t weekday_index);
const char *device_state_get_auto_sleep_label(device_auto_sleep_option_t option);

#endif /* DEVICE_STATE_H */
