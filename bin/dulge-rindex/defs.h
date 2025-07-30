/*-
 * Copyright (c) 2025 TigerClips1 <spongebob1966@proton.me>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-
 */

#ifndef _DULGE_RINDEX_DEFS_H_
#define _DULGE_RINDEX_DEFS_H_

#include <dulge.h>

#define _DULGE_RINDEX		"dulge-rindex"

/* From index-add.c */
int	index_add(struct dulge_handle *, int, int, char **, bool, const char *);

/* From index-clean.c */
int	index_clean(struct dulge_handle *, const char *, bool, const char *);

/* From remove-obsoletes.c */
int	remove_obsoletes(struct dulge_handle *, const char *);

/* From sign.c */
int	sign_repo(struct dulge_handle *, const char *, const char *,
		const char *, const char *);
int	sign_pkgs(struct dulge_handle *, int, int, char **, const char *, bool);

/* From repoflush.c */
int	repodata_flush(const char *repodir, const char *arch,
		dulge_dictionary_t index, dulge_dictionary_t stage, dulge_dictionary_t meta,
		const char *compression);

#endif /* !_DULGE_RINDEX_DEFS_H_ */
