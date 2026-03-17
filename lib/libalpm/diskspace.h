/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#ifndef ALPM_DISKSPACE_H
#define ALPM_DISKSPACE_H

#if defined(HAVE_SYS_MOUNT_H)
#include <sys/mount.h>
#endif
#if defined(HAVE_SYS_STATVFS_H)
#include <sys/statvfs.h>
#endif
#if defined(HAVE_SYS_TYPES_H)
#include <sys/types.h>
#endif

#include "alpm.h"

enum mount_used_level {
	USED_REMOVE = 1,
	USED_INSTALL = (1 << 1),
};

enum mount_fsinfo {
	MOUNT_FSINFO_UNLOADED = 0,
	MOUNT_FSINFO_LOADED,
	MOUNT_FSINFO_FAIL,
};

typedef struct _alpm_mountpoint_t {
	/* mount point information */
	char *mount_dir;
	size_t mount_dir_len;
	/* storage for additional disk usage calculations */
	blkcnt_t blocks_needed;
	blkcnt_t max_blocks_needed;
	enum mount_used_level used;
	int read_only;
	enum mount_fsinfo fsinfo_loaded;
	FSSTATSTYPE fsp;
} alpm_mountpoint_t;

int _alpm_check_diskspace(alpm_handle_t *handle);
int _alpm_check_downloadspace(alpm_handle_t *handle, const char *cachedir,
		size_t num_files, const off_t *file_sizes);

#endif /* ALPM_DISKSPACE_H */
