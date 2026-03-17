/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/







#ifndef PM_INI_H
#define PM_INI_H

typedef int (ini_parser_fn)(const char *file, int line, const char *section,
		char *key, char *value, void *data);

int parse_ini(const char *file, ini_parser_fn cb, void *data);

#endif /* PM_INI_H */
