#ifndef AM_DESKTOP_FILE_DIALOG_H
#define AM_DESKTOP_FILE_DIALOG_H

#include <stddef.h>

typedef enum {
    AM_FILE_PICK_OK = 0,
    AM_FILE_PICK_CANCEL,
    AM_FILE_PICK_UNAVAILABLE,
    AM_FILE_PICK_ERROR
} am_file_pick_result_t;

am_file_pick_result_t am_desktop_file_dialog_pick_audio(char *path_buf, size_t path_buf_size);

#endif /* AM_DESKTOP_FILE_DIALOG_H */
