/*
 * Copyright (C) 2026 Arch Linux team and TigerClips1 <TigerClips1@ps4jaguarlinux.com>
*/




#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <alpm.h>
#include <alpm_list.h>

/* dulge */
#include "dulge.h"
#include "conf.h"
#include "util.h"

/* add targets to the created transaction */
static int load_packages(alpm_list_t *targets, int siglevel)
{
	alpm_list_t *i;
	int retval = 0;

	for(i = targets; i; i = alpm_list_next(i)) {
		const char *targ = i->data;
		alpm_pkg_t *pkg;

		if(alpm_pkg_load(config->handle, targ, 1, siglevel, &pkg) != 0) {
			pm_printf(ALPM_LOG_ERROR, "'%s': %s\n",
					targ, alpm_strerror(alpm_errno(config->handle)));
			retval = 1;
			continue;
		}
		if(alpm_add_pkg(config->handle, pkg) == -1) {
			pm_printf(ALPM_LOG_ERROR, "'%s': %s\n",
					targ, alpm_strerror(alpm_errno(config->handle)));
			alpm_pkg_free(pkg);
			retval = 1;
			continue;
		}
		config->explicit_adds = alpm_list_add(config->explicit_adds, pkg);
	}
	return retval;
}

/**
 * @brief Upgrade a specified list of packages.
 *
 * @param targets a list of packages (as strings) to upgrade
 *
 * @return 0 on success, 1 on failure
 */
int dulge_upgrade(alpm_list_t *targets)
{
	int retval = 0;
	alpm_list_t *remote_targets = NULL, *fetched_files = NULL;
	alpm_list_t *local_targets = NULL;
	alpm_list_t *i;

	if(targets == NULL) {
		pm_printf(ALPM_LOG_ERROR, _("no targets specified (use -h for help)\n"));
		return 1;
	}

	/* carve out remote targets and move it into a separate list */
	for(i = targets; i; i = alpm_list_next(i)) {
		if(strstr(i->data, "://")) {
			remote_targets = alpm_list_add(remote_targets, i->data);
		} else {
			local_targets = alpm_list_add(local_targets, i->data);
		}
	}

	if(remote_targets) {
		retval = alpm_fetch_pkgurl(config->handle, remote_targets, &fetched_files);
		if(retval) {
			goto fail_free;
		}
	}

	/* Step 1: create a new transaction */
	if(trans_init(config->flags, 1) == -1) {
		retval = 1;
		goto fail_free;
	}

	if(!config->print) {
		printf(_("loading packages...\n"));
	}
	retval |= load_packages(local_targets, alpm_option_get_local_file_siglevel(config->handle));
	retval |= load_packages(fetched_files, alpm_option_get_remote_file_siglevel(config->handle));

	if(retval) {
		goto fail_release;
	}

	alpm_list_free(remote_targets);
	alpm_list_free(local_targets);
	FREELIST(fetched_files);

	/* now that targets are resolved, we can hand it all off to the sync code */
	return sync_prepare_execute();

fail_release:
	trans_release();
fail_free:
	alpm_list_free(remote_targets);
	alpm_list_free(local_targets);
	FREELIST(fetched_files);

	return retval;
}
