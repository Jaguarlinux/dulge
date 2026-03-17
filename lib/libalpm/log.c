/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include <syslog.h>
#include <time.h>

/* libalpm */
#include "log.h"
#include "handle.h"
#include "util.h"
#include "alpm.h"

static int _alpm_log_leader(FILE *f, const char *prefix)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	int length = 32;
	char timestamp[length];

	/* Use ISO-8601 date format */
	strftime(timestamp,length,"%FT%T%z", tm);
	return fprintf(f, "[%s] [%s] ", timestamp, prefix);
}

int SYMEXPORT alpm_logaction(alpm_handle_t *handle, const char *prefix,
		const char *fmt, ...)
{
	int ret = 0;
	va_list args;

	ASSERT(handle != NULL, return -1);

	if(!(prefix && *prefix)) {
		prefix = "UNKNOWN";
	}

	/* check if the logstream is open already, opening it if needed */
	if(handle->logstream == NULL && handle->logfile != NULL) {
		int fd;
		do {
			fd = open(handle->logfile, O_WRONLY | O_APPEND | O_CREAT | O_CLOEXEC,
					0644);
		} while(fd == -1 && errno == EINTR);
		/* if we couldn't open it, we have an issue */
		if(fd < 0 || (handle->logstream = fdopen(fd, "a")) == NULL) {
			if(errno == EACCES) {
				handle->pm_errno = ALPM_ERR_BADPERMS;
			} else if(errno == ENOENT) {
				handle->pm_errno = ALPM_ERR_NOT_A_DIR;
			} else {
				handle->pm_errno = ALPM_ERR_SYSTEM;
			}
			ret = -1;
		}
	}

	va_start(args, fmt);

	if(handle->usesyslog) {
		/* we can't use a va_list more than once, so we need to copy it
		 * so we can use the original when calling vfprintf below. */
		va_list args_syslog;
		va_copy(args_syslog, args);
		vsyslog(LOG_WARNING, fmt, args_syslog);
		va_end(args_syslog);
	}

	if(handle->logstream) {
		if(_alpm_log_leader(handle->logstream, prefix) < 0
				|| vfprintf(handle->logstream, fmt, args) < 0) {
			ret = -1;
			handle->pm_errno = ALPM_ERR_SYSTEM;
		}
		fflush(handle->logstream);
	}

	va_end(args);
	return ret;
}

void _alpm_log(alpm_handle_t *handle, alpm_loglevel_t flag, const char *fmt, ...)
{
	va_list args;

	if(handle == NULL || handle->logcb == NULL) {
		return;
	}

	va_start(args, fmt);
	handle->logcb(handle->logcb_ctx, flag, fmt, args);
	va_end(args);
}
