#include <assert.h>

#include "../src/v9_music_player/music_player_mock_data.h"

int main(void)
{
    assert(music_player_radio_count() == 3);
    assert(music_player_recent_count() == 3);
    assert(music_player_playlist_count() >= 2);
    return 0;
}
