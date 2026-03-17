/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdlib.h>
#include <string.h>

/* libalpm */
#include "backup.h"
#include "alpm_list.h"
#include "log.h"
#include "util.h"

/* split a backup string "file\thash" into the relevant components */
int _alpm_split_backup(const char *string, alpm_backup_t **backup)
{
	char *str, *ptr;

	STRDUP(str, string, return -1);

	/* tab delimiter */
	ptr = str ? strchr(str, '\t') : NULL;
	if(ptr == NULL) {
		(*backup)->name = str;
		(*backup)->hash = NULL;
		return 0;
	}
	*ptr = '\0';
	ptr++;
	/* now str points to the filename and ptr points to the hash */
	STRDUP((*backup)->name, str, FREE(str); return -1);
	STRDUP((*backup)->hash, ptr, FREE((*backup)->name); FREE(str); return -1);
	FREE(str);
	return 0;
}

/* Look for a filename in a alpm_pkg_t.backup list. If we find it,
 * then we return the full backup entry.
 */
alpm_backup_t *_alpm_needbackup(const char *file, alpm_pkg_t *pkg)
{
	const alpm_list_t *lp;

	if(file == NULL || pkg == NULL) {
		return NULL;
	}

	for(lp = alpm_pkg_get_backup(pkg); lp; lp = lp->next) {
		alpm_backup_t *backup = lp->data;

		if(strcmp(file, backup->name) == 0) {
			return backup;
		}
	}

	return NULL;
}

void _alpm_backup_free(alpm_backup_t *backup)
{
	ASSERT(backup != NULL, return);
	FREE(backup->name);
	FREE(backup->hash);
	FREE(backup);
}

alpm_backup_t *_alpm_backup_dup(const alpm_backup_t *backup)
{
	alpm_backup_t *newbackup;
	CALLOC(newbackup, 1, sizeof(alpm_backup_t), return NULL);

	STRDUP(newbackup->name, backup->name, goto error);
	STRDUP(newbackup->hash, backup->hash, goto error);

	return newbackup;

error:
	free(newbackup->name);
	free(newbackup);
	return NULL;
}
