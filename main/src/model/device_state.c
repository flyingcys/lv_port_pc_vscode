#include "model/device_state.h"

#include <stdio.h>
#include <string.h>

static const char *const g_weekdays[] = {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat",
};

static const char *const g_auto_sleep_labels[] = {
    "Off",
    "15s",
    "30s",
    "1 min",
    "5 min",
    "15 min",
};

void device_state_init_defaults(device_state_t *state)
{
    static const char *const default_scene_names[] = {
        "Home",
        "Dining",
        "Away",
        "Sleep",
        "Focus",
        "Party",
        "Relax",
        "Night",
    };

    if(state == NULL) {
        return;
    }

    memset(state, 0, sizeof(*state));

    state->hour = 9U;
    state->minute = 26U;
    state->use_24h = false;
    state->month = 6U;
    state->day = 24U;
    state->weekday_index = 1U;
    state->temperature_c = 26;
    snprintf(state->weather_summary, sizeof(state->weather_summary), "%s", "Sunny");
    state->wifi_signal_level = 4U;
    snprintf(state->wifi_ssid, sizeof(state->wifi_ssid), "%s", "Tuya Demo");

    snprintf(state->switches[0].name, sizeof(state->switches[0].name), "%s", "Switch 1");
    state->switches[0].is_on = false;
    snprintf(state->switches[1].name, sizeof(state->switches[1].name), "%s", "Switch 2");
    state->switches[1].is_on = true;

    state->scene_count = DEVICE_STATE_MAX_SCENES;
    for(size_t i = 0; i < DEVICE_STATE_MAX_SCENES; ++i) {
        snprintf(state->scenes[i].name, sizeof(state->scenes[i].name), "%s", default_scene_names[i]);
        state->scenes[i].enabled = (i % 3U) != 2U;
        state->scenes[i].icon_variant = (uint8_t)(i + 1U);
    }

    state->brightness_percent = 100U;
    state->auto_sleep_enabled = true;
    state->auto_sleep_option = DEVICE_AUTO_SLEEP_5_MINUTES;
    state->proximity_wake_enabled = false;
    state->voice_state = DEVICE_VOICE_STATE_IDLE;
    snprintf(state->voice_input_text, sizeof(state->voice_input_text), "%s", "Turn on Switch 2");
    snprintf(state->voice_output_text, sizeof(state->voice_output_text), "%s", "Switch 2 is now on.");
    state->muted = false;
    state->volume_percent = 50U;
}

void device_state_format_time(const device_state_t *state, char *buf, size_t buf_size)
{
    uint8_t display_hour;

    if(state == NULL || buf == NULL || buf_size == 0U) {
        return;
    }

    if(state->use_24h) {
        display_hour = state->hour;
    }
    else {
        display_hour = state->hour % 12U;
        if(display_hour == 0U) {
            display_hour = 12U;
        }
    }

    snprintf(buf, buf_size, "%02u:%02u", display_hour, state->minute);
}

void device_state_format_date(const device_state_t *state, char *buf, size_t buf_size)
{
    if(state == NULL || buf == NULL || buf_size == 0U) {
        return;
    }

    snprintf(buf, buf_size, "%u/%u %s", state->month, state->day, device_state_get_weekday_short(state->weekday_index));
}

const char *device_state_get_weekday_short(uint8_t weekday_index)
{
    if(weekday_index >= (sizeof(g_weekdays) / sizeof(g_weekdays[0]))) {
        return "--";
    }

    return g_weekdays[weekday_index];
}

const char *device_state_get_auto_sleep_label(device_auto_sleep_option_t option)
{
    if((size_t)option >= (sizeof(g_auto_sleep_labels) / sizeof(g_auto_sleep_labels[0]))) {
        return "--";
    }

    return g_auto_sleep_labels[option];
}
