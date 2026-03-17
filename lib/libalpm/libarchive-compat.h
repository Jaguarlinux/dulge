#ifndef LIBARCHIVE_COMPAT_H
#define LIBARCHIVE_COMPAT_H

/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdint.h>

static inline int _alpm_archive_read_free(struct archive *archive)
{
	return archive_read_free(archive);
}

static inline int64_t _alpm_archive_compressed_ftell(struct archive *archive)
{
	return archive_filter_bytes(archive, -1);
}

static inline int _alpm_archive_read_open_file(struct archive *archive,
		const char *filename, size_t block_size)
{
	return archive_read_open_filename(archive, filename, block_size);
}

static inline int _alpm_archive_filter_code(struct archive *archive)
{
	return archive_filter_code(archive, 0);
}

static inline int _alpm_archive_read_support_filter_all(struct archive *archive)
{
	return archive_read_support_filter_all(archive);
}

#endif /* LIBARCHIVE_COMPAT_H */
