/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#ifndef ALPM_SANDBOX_H
#define ALPM_SANDBOX_H

#include <stdbool.h>

/* utility function to test if sandboxing should be used */
bool _alpm_use_sandbox(alpm_handle_t *handle);

/* The type of callbacks that can happen during a sandboxed operation */
typedef enum {
	ALPM_SANDBOX_CB_LOG,
	ALPM_SANDBOX_CB_DOWNLOAD
} _alpm_sandbox_callback_t;

typedef struct {
	int callback_pipe;
} _alpm_sandbox_callback_context;


/* Sandbox callbacks */

__attribute__((format(printf, 3, 0)))
void _alpm_sandbox_cb_log(void *ctx, alpm_loglevel_t level, const char *fmt, va_list args);

void _alpm_sandbox_cb_dl(void *ctx, const char *filename, alpm_download_event_type_t event, void *data);


/* Functions to capture sandbox callbacks and convert them to alpm callbacks */

bool _alpm_sandbox_process_cb_log(alpm_handle_t *handle, int callback_pipe);
bool _alpm_sandbox_process_cb_download(alpm_handle_t *handle, int callback_pipe);


#endif /* ALPM_SANDBOX_H */
