/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/



#ifndef PM_PACKAGE_H
#define PM_PACKAGE_H

#include <alpm.h>

void dump_pkg_full(alpm_pkg_t *pkg, int extra);

void dump_pkg_backups(alpm_pkg_t *pkg, unsigned short cols);
void dump_pkg_files(alpm_pkg_t *pkg, int quiet);
void dump_pkg_changelog(alpm_pkg_t *pkg);

void print_installed(alpm_db_t *db_local, alpm_pkg_t *pkg);
void print_groups(alpm_pkg_t *pkg);
int dump_pkg_search(alpm_db_t *db, alpm_list_t *targets, int show_status);

#endif /* PM_PACKAGE_H */
