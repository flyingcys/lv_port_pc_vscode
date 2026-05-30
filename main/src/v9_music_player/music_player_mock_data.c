#include "music_player_mock_data.h"

static const music_player_station_t g_radio_items[] = {
    { "Pop Pulse", "Current singles in rotation" },
    { "Morning Acoustic", "Soft start for early hours" },
    { "City Drive", "Rhythm for commute and night roads" },
};

static const music_player_recent_item_t g_recent_items[] = {
    { "New Music Daily", "Editor radio stream" },
    { "Low Pressure Flow", "Focus playlist" },
    { "Golden Hour", "Single replay" },
};

static const music_player_playlist_item_t g_playlist_items[] = {
    { "Morning Build", "Steady progress mix" },
    { "Night Drive", "Synth and drum machine set" },
    { "Radio Archive", "Long-form program shelf" },
};

size_t music_player_radio_count(void)
{
    return sizeof(g_radio_items) / sizeof(g_radio_items[0]);
}

size_t music_player_recent_count(void)
{
    return sizeof(g_recent_items) / sizeof(g_recent_items[0]);
}

size_t music_player_playlist_count(void)
{
    return sizeof(g_playlist_items) / sizeof(g_playlist_items[0]);
}

const music_player_station_t * music_player_radio_items(void)
{
    return g_radio_items;
}

const music_player_recent_item_t * music_player_recent_items(void)
{
    return g_recent_items;
}

const music_player_playlist_item_t * music_player_playlist_items(void)
{
    return g_playlist_items;
}
