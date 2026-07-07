#define _POSIX_C_SOURCE 200809L

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "../src/v9_apple_music/am_desktop_file_dialog.h"

static void write_script(const char *path, const char *body)
{
    FILE *fp = fopen(path, "w");

    if(fp == NULL) abort();
    if(fputs(body, fp) == EOF) abort();
    if(fclose(fp) != 0) abort();
    if(chmod(path, 0755) != 0) abort();
}

static void test_pick_audio_success_via_helper(void)
{
    char path_buf[1024] = {0};

    write_script("build/am_pick_ok.sh", "#!/bin/sh\nprintf '/tmp/demo.mp3\\n'");
    if(setenv("AM_FILE_DIALOG_HELPER", "build/am_pick_ok.sh", 1) != 0) abort();
    if(setenv("DISPLAY", "", 1) != 0) abort();
    if(setenv("WAYLAND_DISPLAY", "", 1) != 0) abort();
    if(am_desktop_file_dialog_pick_audio(path_buf, sizeof(path_buf)) != AM_FILE_PICK_OK) abort();
    if(strcmp(path_buf, "/tmp/demo.mp3") != 0) abort();
}

static void test_pick_audio_cancel_via_helper(void)
{
    char path_buf[1024] = {0};

    write_script("build/am_pick_cancel.sh", "#!/bin/sh\nexit 1");
    if(setenv("AM_FILE_DIALOG_HELPER", "build/am_pick_cancel.sh", 1) != 0) abort();
    if(setenv("DISPLAY", "", 1) != 0) abort();
    if(setenv("WAYLAND_DISPLAY", "", 1) != 0) abort();
    if(am_desktop_file_dialog_pick_audio(path_buf, sizeof(path_buf)) != AM_FILE_PICK_CANCEL) abort();
}

static void test_pick_audio_helper_path_with_spaces(void)
{
    char path_buf[1024] = {0};

    write_script("build/am pick ok.sh", "#!/bin/sh\nprintf '/tmp/space-helper.flac\\n'");
    if(setenv("AM_FILE_DIALOG_HELPER", "build/am pick ok.sh", 1) != 0) abort();
    if(setenv("DISPLAY", "", 1) != 0) abort();
    if(setenv("WAYLAND_DISPLAY", "", 1) != 0) abort();
    if(am_desktop_file_dialog_pick_audio(path_buf, sizeof(path_buf)) != AM_FILE_PICK_OK) abort();
    if(strcmp(path_buf, "/tmp/space-helper.flac") != 0) abort();
}

int main(void)
{
    test_pick_audio_success_via_helper();
    test_pick_audio_cancel_via_helper();
    test_pick_audio_helper_path_with_spaces();
    return 0;
}
