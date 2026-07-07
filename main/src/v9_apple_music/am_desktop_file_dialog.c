#define _POSIX_C_SOURCE 200809L

#include "am_desktop_file_dialog.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

typedef struct {
    const char *cmd;
    const char *args[8];
} am_dialog_candidate_t;

static bool am_has_text(const char *s)
{
    return s != NULL && s[0] != '\0';
}

static void am_trim_line(char *s)
{
    size_t len;

    if(s == NULL) return;
    len = strlen(s);
    while(len > 0U) {
        char ch = s[len - 1U];
        if(ch != '\n' && ch != '\r') break;
        s[len - 1U] = '\0';
        len--;
    }
}

static bool am_dialog_env_has_display(void)
{
    return am_has_text(getenv("DISPLAY")) || am_has_text(getenv("WAYLAND_DISPLAY"));
}

static am_file_pick_result_t am_status_to_result(int status)
{
    if(WIFEXITED(status)) {
        int code = WEXITSTATUS(status);

        if(code == 0) return AM_FILE_PICK_OK;
        if(code == 1) return AM_FILE_PICK_CANCEL;
        if(code == 127) return AM_FILE_PICK_UNAVAILABLE;
        return AM_FILE_PICK_ERROR;
    }

    return AM_FILE_PICK_ERROR;
}

static am_file_pick_result_t am_run_argv(const char *const *argv, char *path_buf, size_t path_buf_size)
{
    pid_t pid;
    FILE *fp;
    int pipefd[2];
    int status;

    if(argv == NULL || argv[0] == NULL || path_buf == NULL || path_buf_size == 0U) return AM_FILE_PICK_ERROR;

    if(pipe(pipefd) != 0) return AM_FILE_PICK_ERROR;

    pid = fork();
    if(pid < 0) {
        close(pipefd[0]);
        close(pipefd[1]);
        return AM_FILE_PICK_ERROR;
    }

    if(pid == 0) {
        close(pipefd[0]);
        if(dup2(pipefd[1], STDOUT_FILENO) < 0) _exit(127);
        close(pipefd[1]);
        execvp(argv[0], (char *const *)argv);
        _exit(127);
    }

    close(pipefd[1]);
    fp = fdopen(pipefd[0], "r");
    if(fp == NULL) {
        close(pipefd[0]);
        (void)waitpid(pid, NULL, 0);
        return AM_FILE_PICK_ERROR;
    }

    if(fgets(path_buf, (int)path_buf_size, fp) == NULL) {
        path_buf[0] = '\0';
    } else {
        am_trim_line(path_buf);
    }

    if(fclose(fp) != 0) {
        (void)waitpid(pid, NULL, 0);
        return AM_FILE_PICK_ERROR;
    }

    if(waitpid(pid, &status, 0) < 0) return AM_FILE_PICK_ERROR;
    if(WIFEXITED(status) && WEXITSTATUS(status) == 127) return AM_FILE_PICK_UNAVAILABLE;
    if(WIFEXITED(status) && WEXITSTATUS(status) == 1) return AM_FILE_PICK_CANCEL;
    if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        return am_has_text(path_buf) ? AM_FILE_PICK_OK : AM_FILE_PICK_ERROR;
    }
    return am_status_to_result(status);
}

static am_file_pick_result_t am_run_helper_override(char *path_buf, size_t path_buf_size)
{
    const char *helper = getenv("AM_FILE_DIALOG_HELPER");
    const char *argv[2] = { helper, NULL };

    if(!am_has_text(helper)) return AM_FILE_PICK_UNAVAILABLE;
    return am_run_argv(argv, path_buf, path_buf_size);
}

static am_file_pick_result_t am_run_candidate(const am_dialog_candidate_t *candidate,
                                              char *path_buf, size_t path_buf_size)
{
    const char *argv[10] = {0};
    size_t i;

    if(candidate == NULL || !am_has_text(candidate->cmd)) return AM_FILE_PICK_UNAVAILABLE;

    argv[0] = candidate->cmd;
    for(i = 0U; i < sizeof(candidate->args) / sizeof(candidate->args[0]); i++) {
        if(candidate->args[i] == NULL) break;
        argv[i + 1U] = candidate->args[i];
    }

    return am_run_argv(argv, path_buf, path_buf_size);
}

am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size)
{
    static const am_dialog_candidate_t candidates[] = {
        { "zenity",
          { "--file-selection", "--title=打开本地音频",
            "--file-filter=音频文件 | *.mp3 *.wav *.flac *.aac *.m4a *.ogg *.opus *.wma *.mp4 *.ts *.aiff *.ac3",
            NULL } },
        { "qarma",
          { "--file-selection", "--title=打开本地音频",
            "--file-filter=音频文件 | *.mp3 *.wav *.flac *.aac *.m4a *.ogg *.opus *.wma *.mp4 *.ts *.aiff *.ac3",
            NULL } },
        { "kdialog",
          { "--getopenfilename", ".",
            "*.mp3 *.wav *.flac *.aac *.m4a *.ogg *.opus *.wma *.mp4 *.ts *.aiff *.ac3",
            NULL } },
    };
    size_t i;
    am_file_pick_result_t rc;

    if(path_buf == NULL || path_buf_size == 0U) return AM_FILE_PICK_ERROR;
    path_buf[0] = '\0';

    rc = am_run_helper_override(path_buf, path_buf_size);
    if(rc != AM_FILE_PICK_UNAVAILABLE) return rc;
    if(!am_dialog_env_has_display()) return AM_FILE_PICK_UNAVAILABLE;

    for(i = 0U; i < sizeof(candidates) / sizeof(candidates[0]); i++) {
        rc = am_run_candidate(&candidates[i], path_buf, path_buf_size);
        if(rc == AM_FILE_PICK_OK || rc == AM_FILE_PICK_CANCEL) return rc;
    }

    return AM_FILE_PICK_UNAVAILABLE;
}
