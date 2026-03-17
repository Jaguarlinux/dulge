/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef PM_CALLBACK_H
#define PM_CALLBACK_H

#include <stdbool.h>
#include <sys/types.h> /* off_t */

#include <alpm.h>

/* callback to handle messages/notifications from libalpm */
void cb_event(void *ctx, alpm_event_t *event);

/* callback to handle questions from libalpm (yes/no) */
void cb_question(void *ctx, alpm_question_t *question);

/* callback to handle display of progress */
void cb_progress(void *ctx, alpm_progress_t event, const char *pkgname,
		int percent, size_t howmany, size_t remain);

/* callback to handle display of download progress */
void cb_download(void *ctx, const char *filename, alpm_download_event_type_t event,
		void *data);

/* callback to handle messages/notifications from dulge library */
__attribute__((format(printf, 3, 0)))
void cb_log(void *ctx, alpm_loglevel_t level, const char *fmt, va_list args);

/* specify if multibar UI should move completed bars to the top of the screen */
void multibar_move_completed_up(bool value);

#endif /* PM_CALLBACK_H */
