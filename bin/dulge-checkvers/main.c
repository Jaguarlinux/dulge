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

#include <sys/stat.h>

#include <assert.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dulge.h>

#define GOT_PKGNAME_VAR 	0x1
#define GOT_VERSION_VAR 	0x2
#define GOT_REVISION_VAR 	0x4

typedef struct _rcv_t {
	const char *prog, *fname, *format;
	char *dulge_conf, *rootdir, *distdir, *buf, *ptr, *cachefile;
	size_t bufsz, len;
	uint8_t have_vars;
	bool show_all, manual, installed, removed, show_removed;
	dulge_dictionary_t env;
	dulge_dictionary_t pkgd;
	dulge_dictionary_t cache;
	dulge_dictionary_t templates;
	struct dulge_handle xhp;
} rcv_t;

typedef int (*rcv_check_func)(rcv_t *);
typedef int (*rcv_proc_func)(rcv_t *, const char *, rcv_check_func);

static char *
xstrdup(const char *src)
{
	char *p;
	if (!(p = strdup(src))) {
		dulge_error_printf("%s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}
	return p;
}

static int
show_usage(const char *prog, bool fail)
{
	fprintf(stderr,
"Usage: %s [OPTIONS] [FILES...]\n\n"
"OPTIONS:\n"
" -h, --help              Show usage\n"
" -C, --config <dir>      Set path to dulge.d\n"
" -D, --distdir <dir>     Set (or override) the path to void-packages\n"
"                         (defaults to ~/void-packages)\n"
" -d, --debug             Enable debug output to stderr\n"
" -e, --removed           List packages present in repos, but not in distdir\n"
" -f, --format <fmt>      Output format\n"
" -I, --installed         Check for outdated packages in rootdir, rather\n"
"                         than in the DULGE repositories\n"
" -i, --ignore-conf-repos Ignore repositories defined in dulge.d\n"
" -m, --manual            Only process listed files\n"
" -R, --repository=<url>  Append repository to the head of repository list\n"
" -r, --rootdir <dir>     Set root directory (defaults to /)\n"
" -s, --show-all          List all packages, in the format 'pkgname repover srcver'\n"
"     --staging           Enable use of staged packages\n"
"\n  [FILES...]           Extra packages to process with the outdated\n"
"                         ones (only processed if missing).\n", prog);
	return fail ? EXIT_FAILURE: EXIT_SUCCESS;
}

static void
rcv_init(rcv_t *rcv, const char *prog)
{
	rcv->prog = prog;
	rcv->have_vars = 0;
	rcv->ptr = rcv->buf = NULL;

	rcv->cache = dulge_dictionary_internalize_from_file(rcv->cachefile);
	if (!rcv->cache)
		rcv->cache = dulge_dictionary_create();
	assert(rcv->cache);

	if (rcv->dulge_conf != NULL) {
		dulge_strlcpy(rcv->xhp.confdir, rcv->dulge_conf, sizeof(rcv->xhp.confdir));
	}
	if (rcv->rootdir != NULL) {
		dulge_strlcpy(rcv->xhp.rootdir, rcv->rootdir, sizeof(rcv->xhp.rootdir));
	}
	if (dulge_init(&rcv->xhp) != 0)
		abort();
	rcv->templates = dulge_dictionary_create();
}

static void
rcv_end(rcv_t *rcv)
{
	dulge_dictionary_externalize_to_file(rcv->cache, rcv->cachefile);

	if (rcv->buf != NULL) {
		free(rcv->buf);
		rcv->buf = NULL;
	}
	if (rcv->env != NULL) {
		dulge_object_release(rcv->env);
		rcv->env = NULL;
	}

	dulge_end(&rcv->xhp);

	if (rcv->dulge_conf != NULL)
		free(rcv->dulge_conf);
	if (rcv->distdir != NULL)
		free(rcv->distdir);

	free(rcv->cachefile);
	dulge_object_release(rcv->templates);
	rcv->templates = NULL;
}

static bool
rcv_load_file(rcv_t *rcv, const char *fname)
{
	FILE *file;
	long offset;
    int rv;
	rcv->fname = fname;

	if ((file = fopen(rcv->fname, "r")) == NULL) {
		if (!rcv->manual) {
			dulge_error_printf("FileError: can't open '%s': %s\n",
				rcv->fname, strerror(errno));
		}
		return false;
	}

	fseek(file, 0, SEEK_END);
	offset = ftell(file);
    rv = errno;
	fseek(file, 0, SEEK_SET);

	if (offset == -1) {
		dulge_error_printf("FileError: failed to get offset: %s", strerror(rv));
		fclose(file);
		return false;
	}
	rcv->len = (size_t)offset;

	if (rcv->buf == NULL) {
		rcv->bufsz = rcv->len+1;
		if (!(rcv->buf = calloc(rcv->bufsz, sizeof(char)))) {
			dulge_error_printf("MemError: can't allocate memory: %s\n",
				strerror(errno));
			fclose(file);
			return false;
		}
	} else if (rcv->bufsz <= rcv->len) {
		rcv->bufsz = rcv->len+1;
		if (!(rcv->buf = realloc(rcv->buf, rcv->bufsz))) {
			dulge_error_printf("MemError: can't allocate memory: %s\n",
				strerror(errno));
			fclose(file);
			return false;
		}
	}

	(void)!fread(rcv->buf, sizeof(char), rcv->len, file);
	rcv->buf[rcv->len] = '\0';
	fclose(file);
	rcv->ptr = rcv->buf;

	return true;
}

static char *
rcv_sh_substitute(rcv_t *rcv, const char *str, size_t len)
{
	const char *p;
	char *cmd, *ret;
	dulge_string_t out = dulge_string_create();
	char b[2] = {0};

	for (p = str; *p && p < str+len; p++) {
		switch (*p) {
		case '$':
			if (p+1 < str+len) {
				char buf[1024] = {0};
				const char *ref;
				const char *val;
				size_t reflen;
				p++;
				if (*p == '(') {
					FILE *fp;
					int c;
					for (ref = ++p; *p && p < str+len && *p != ')'; p++)
						;
					if (*p != ')')
						goto err1;
					cmd = strndup(ref, p-ref);
					if ((fp = popen(cmd, "r")) == NULL)
						goto err2;
					while ((c = fgetc(fp)) != EOF && c != '\n') {
						*b = c;
						dulge_string_append_cstring(out, b);
					}
					if (pclose(fp) != 0)
						goto err2;
					free(cmd);
					cmd = NULL;
					continue;
				} else if (*p == '{') {
					for (ref = ++p; *p && p < str+len && (isalnum((unsigned char)*p) || *p == '_'); p++)
						;
					reflen = p-ref;
					switch (*p) {
					case '/': /* fallthrough */
					case '%': /* fallthrough */
					case '#': /* fallthrough */
					case ':':
						for (; *p && p < str+len && *p != '}'; p++)
							;
						if (*p != '}')
							goto err1;
						break;
					case '}':
						break;
					default:
						goto err1;
					}
				} else {
					for (ref = p; *p && p < str+len && (isalnum((unsigned char)*p) || *p == '_'); p++)
						;
					reflen = p-ref;
					p--;
				}
				if (reflen) {
					if (reflen >= sizeof buf) {
						dulge_error_printf("out of memory\n");
						exit(EXIT_FAILURE);
					}
					strncpy(buf, ref, reflen);
					if (dulge_dictionary_get_cstring_nocopy(rcv->env, buf, &val))
						dulge_string_append_cstring(out, val);
					else
						dulge_string_append_cstring(out, "NULL");
					break;
				}
			}
			/* fallthrough */
		default:
			*b = *p;
			dulge_string_append_cstring(out, b);
		}
	}

	ret = dulge_string_cstring(out);
	dulge_object_release(out);
	return ret;

err1:
	dulge_error_printf("syntax error: in file '%s'\n", rcv->fname);
	exit(EXIT_FAILURE);
err2:
	dulge_error_printf(
		"Shell cmd failed: '%s' for "
		"template '%s'",
		cmd, rcv->fname);
	if (errno > 0) {
		fprintf(stderr, ": %s\n",
			strerror(errno));
	} else {
		fputc('\n', stderr);
	}
	exit(EXIT_FAILURE);
}

static void
rcv_get_pkgver(rcv_t *rcv)
{
	size_t klen, vlen;
	char c, *ptr = rcv->ptr, *e, *p, *k, *v, *comment, *d, *key, *val;
	uint8_t vars = 0;

	while ((c = *ptr) != '\0') {
		if (c == '#' || c == '.') {
			goto nextline;
		}
		if (c == '\n') {
			ptr++;
			continue;
		}
		if (c == 'u' && (strncmp("unset", ptr, 5)) == 0) {
			goto nextline;
		}
		if ((e = strchr(ptr, '=')) == NULL)
			goto nextline;

		p = strchr(ptr, '\n');
		k = ptr;
		v = e + 1;

		assert(p);
		assert(k);
		assert(v);

		klen = strlen(k) - strlen(e);
		vlen = strlen(v) - strlen(p);

		if (v[0] == '"' && vlen == 1) {
			while (*ptr++ != '"')
				;
			goto nextline;
		}
		if (v[0] == '"') {
			v++;
			vlen--;
		}
		if (v[vlen-1] == '"') {
			vlen--;
		}
		comment = strchr(v, '#');
		if (comment && comment < p && (comment > v && comment[-1] == ' ')) {
			while (v[vlen-1] != '#') {
				vlen--;
			}
			vlen--;
			while (isspace((unsigned char)v[vlen-1])) {
				vlen--;
			}
		}
		if (vlen == 0) {
			goto nextline;
		}
		key = strndup(k, klen);

		if ((d = strchr(v, '$')) && d < v+vlen)
			val = rcv_sh_substitute(rcv, v, vlen);
		else
			val = strndup(v, vlen);

		if (!dulge_dictionary_set(rcv->env, key,
		    dulge_string_create_cstring(val))) {
			dulge_error_printf("dulge_dictionary_set failed");
			exit(EXIT_FAILURE);
		}

		dulge_dbg_printf("%s: %s %s\n", rcv->fname, key, val);

		free(key);
		free(val);

		if (strncmp("pkgname", k, klen) == 0) {
			rcv->have_vars |= GOT_PKGNAME_VAR;
			vars++;
		} else if (strncmp("version",  k, klen) == 0) {
			rcv->have_vars |= GOT_VERSION_VAR;
			vars++;
		} else if (strncmp("revision", k, klen) == 0) {
			rcv->have_vars |= GOT_REVISION_VAR;
			vars++;
		}
		if (vars > 2)
			return;

nextline:
		ptr = strchr(ptr, '\n') + 1;
	}
}

static int
rcv_process_file(rcv_t *rcv, const char *fname, rcv_check_func check)
{
	int rv = 0;
	dulge_object_t keysym;
	dulge_object_iterator_t iter;
	dulge_dictionary_t d;
	dulge_data_t mtime;
	const char *pkgname, *version, *revision, *reverts;
	struct stat st;
	bool allocenv = false;

	if (stat(fname, &st) == -1) {
		rv = EXIT_FAILURE;
		goto ret;
	}

	if ((d = dulge_dictionary_get(rcv->cache, fname))) {
		mtime = dulge_dictionary_get(d, "mtime");
		if (!dulge_data_equals_data(mtime, &st.st_mtim, sizeof st.st_mtim))
			goto update;
		rcv->env = d;
		rcv->have_vars = GOT_PKGNAME_VAR | GOT_VERSION_VAR | GOT_REVISION_VAR;
		rcv->fname = fname;
	} else {
update:
		if (!rcv_load_file(rcv, fname)) {
			rv = EXIT_FAILURE;
			goto ret;
		}
		assert(rcv);
		rcv->env = dulge_dictionary_create();
		assert(rcv->env);
		allocenv = true;
		rcv_get_pkgver(rcv);

		if (!dulge_dictionary_get_cstring_nocopy(rcv->env, "pkgname", &pkgname) ||
			!dulge_dictionary_get_cstring_nocopy(rcv->env, "version", &version) ||
			!dulge_dictionary_get_cstring_nocopy(rcv->env, "revision", &revision)) {
			dulge_error_printf("'%s':"
			    " missing required variable (pkgname, version or revision)!",
			    fname);
			exit(EXIT_FAILURE);
		}
		if (!d) {
			d = dulge_dictionary_create();
			dulge_dictionary_set(rcv->cache, fname, d);
		}
		dulge_dictionary_set_cstring(d, "pkgname", pkgname);
		dulge_dictionary_set_cstring(d, "version", version);
		dulge_dictionary_set_cstring(d, "revision", revision);

		reverts = NULL;
		dulge_dictionary_get_cstring_nocopy(rcv->env, "reverts", &reverts);
		if (reverts)
			dulge_dictionary_set_cstring(d, "reverts", reverts);

		mtime = dulge_data_create_data(&st.st_mtim, sizeof st.st_mtim);
		dulge_dictionary_set(d, "mtime", mtime);
	}

	check(rcv);

ret:
	if (allocenv) {
		iter = dulge_dictionary_iterator(rcv->env);
		while ((keysym = dulge_object_iterator_next(iter)))
			dulge_object_release(dulge_dictionary_get_keysym(rcv->env, keysym));
		dulge_object_iterator_release(iter);
		dulge_object_release(rcv->env);
	}
	rcv->env = NULL;
	return rv;
}

static bool
check_reverts(const char *repover, const char *reverts)
{
	const char *s, *e;
	size_t len;

	assert(reverts);

	s = reverts;
	if ((len = strlen(s)) == 0)
		return false;

	for (s = reverts; s < reverts+len; s = e+1) {
		if (!(e = strchr(s, ' ')))
			e = reverts+len;
		if (strncmp(s, repover, e-s) == 0)
			return true;
	}

	return false;
}

static void
rcv_printf(rcv_t *rcv, FILE *fp, const char *pkgname, const char *repover,
    const char *srcver, const char *repourl)
{
	const char *f, *p;

	for (f = rcv->format; *f; f++) {
		if (*f == '\\') {
			f++;
			switch (*f) {
			case '\n': fputc('\n', fp); break;
			case '\t': fputc('\t', fp); break;
			case '\0': fputc('\0', fp); break;
			default:
				fputc('\\', fp);
				fputc(*f, fp);
				break;
			}
		}
		if (*f != '%') {
			fputc(*f, fp);
			continue;
		}
		switch (*++f) {
		case '%': fputc(*f, fp); break;
		case 'n': fputs(pkgname, fp); break;
		case 'r': fputs(repover, fp); break;
		case 'R': fputs(repourl, fp); break;
		case 's': fputs(srcver, fp); break;
		case 't':
			p = strchr(rcv->fname, '/');
			fwrite(rcv->fname, p ? (size_t)(p - rcv->fname) : strlen(rcv->fname), 1, fp);
			break;
		}
	}
	fputc('\n', fp);
}

static int
rcv_check_version(rcv_t *rcv)
{
	const char *repover = NULL;
	char srcver[BUFSIZ] = { '\0' }, *binpkgname = NULL, *s = NULL;
	const char *pkgname, *version, *revision, *reverts, *repourl;
	int sz;
	size_t len;

	assert(rcv);

	if ((rcv->have_vars & GOT_PKGNAME_VAR) == 0) {
		dulge_error_printf("'%s': missing pkgname variable!\n", rcv->fname);
		exit(EXIT_FAILURE);
	}
	if ((rcv->have_vars & GOT_VERSION_VAR) == 0) {
		dulge_error_printf("'%s': missing version variable!\n", rcv->fname);
		exit(EXIT_FAILURE);
	}
	if ((rcv->have_vars & GOT_REVISION_VAR) == 0) {
		dulge_error_printf("'%s': missing revision variable!\n", rcv->fname);
		exit(EXIT_FAILURE);
	}

	if (!dulge_dictionary_get_cstring_nocopy(rcv->env, "pkgname", &pkgname) ||
	    !dulge_dictionary_get_cstring_nocopy(rcv->env, "version", &version) ||
	    !dulge_dictionary_get_cstring_nocopy(rcv->env, "revision", &revision)) {
		dulge_error_printf("couldn't get pkgname, version, and/or revision\n");
		exit(EXIT_FAILURE);
	}

	reverts = NULL;
	dulge_dictionary_get_cstring_nocopy(rcv->env, "reverts", &reverts);

	if (rcv->removed)
		sz = snprintf(srcver, sizeof srcver, "?");
	else
		sz = snprintf(srcver, sizeof srcver, "%s_%s", version, revision);

	if (sz < 0 || (size_t)sz >= sizeof srcver) {
		dulge_error_printf("failed to write version");
		exit(EXIT_FAILURE);
	}

	/* Check against binpkg's pkgname, not pkgname from template */
	s = strchr(rcv->fname, '/');
	len = s ? strlen(rcv->fname) - strlen(s) : strlen(rcv->fname);
	binpkgname = strndup(rcv->fname, len);
	assert(binpkgname);

	repourl = NULL;
	if (rcv->installed) {
		rcv->pkgd = dulge_pkgdb_get_pkg(&rcv->xhp, binpkgname);
	} else {
		rcv->pkgd = dulge_rpool_get_pkg(&rcv->xhp, binpkgname);
		dulge_dictionary_get_cstring_nocopy(rcv->pkgd, "repository", &repourl);
	}
	dulge_dictionary_get_cstring_nocopy(rcv->pkgd, "pkgver", &repover);
	if (repover)
		repover += strlen(binpkgname)+1;

	free(binpkgname);
	if (!repover && rcv->manual)
		;
	else if (rcv->show_all)
		;
	else if (rcv->show_removed && rcv->removed)
		;
	else if (rcv->show_removed && !rcv->removed)
		return 0;
	else if (repover && (dulge_cmpver(repover, srcver) < 0 ||
		    (reverts && check_reverts(repover, reverts))))
		;
	else
		return 0;

	repover = repover ? repover : "?";
	repourl = repourl ? repourl : "?";
	rcv_printf(rcv, stdout, pkgname, repover, srcver, repourl);

	return 0;
}

static int
rcv_process_dir(rcv_t *rcv, rcv_proc_func process)
{
	DIR *dir = NULL;
	struct dirent *result;
	struct stat st;
	char filename[BUFSIZ];
	int ret = 0;

	if (!(dir = opendir(".")))
		goto error;

	while ((result = readdir(dir))) {
		if ((strcmp(result->d_name, ".") == 0) ||
		    (strcmp(result->d_name, "..") == 0))
			continue;
		if ((lstat(result->d_name, &st)) != 0)
			goto error;

		if (rcv->show_removed)
			dulge_dictionary_set_bool(rcv->templates, result->d_name, true);

		if (S_ISLNK(st.st_mode) != 0 && !rcv->installed)
			continue;

		snprintf(filename, sizeof(filename), "%s/template", result->d_name);
		ret = process(rcv, filename, rcv_check_version);
	}

	if ((closedir(dir)) != -1)
		return ret;
error:
	dulge_error_printf("while processing dir '%s/srcpkgs': %s\n",
	    rcv->distdir, strerror(errno));
	exit(EXIT_FAILURE);
}

static int
template_removed_cb(struct dulge_handle *xhp UNUSED,
		dulge_object_t obj UNUSED,
		const char *key,
		void *arg,
		bool *done UNUSED)
{
	char *pkgname;
	char *last_dash;
	bool dummy_bool = false;
	rcv_t *rcv = arg;

	last_dash = strrchr(key, '-');
	if (last_dash && (strcmp(last_dash, "-32bit") == 0 || strcmp(last_dash, "-dbg") == 0)) {
		pkgname = strndup(key, last_dash - key);
	} else {
		pkgname = strdup(key);
	}
	if (!dulge_dictionary_get_bool(rcv->templates, pkgname, &dummy_bool)) {
		rcv->removed = true;
		rcv->env = dulge_dictionary_create();
		dulge_dictionary_set_cstring_nocopy(rcv->env, "pkgname", key);
		dulge_dictionary_set_cstring_nocopy(rcv->env, "version", "-");
		dulge_dictionary_set_cstring_nocopy(rcv->env, "revision", "-");
		rcv->have_vars = GOT_PKGNAME_VAR | GOT_VERSION_VAR | GOT_REVISION_VAR;
		rcv->fname = key;
		rcv_check_version(rcv);
		rcv->removed = false;
		dulge_object_release(rcv->env);
		rcv->env = NULL;
	}
	free(pkgname);
	return 0;
}

static int
repo_templates_removed_cb(struct dulge_repo *repo, void *arg, bool *done UNUSED)
{
	dulge_array_t allkeys;

	allkeys = dulge_dictionary_all_keys(repo->idx);
	dulge_array_foreach_cb(repo->xhp, allkeys, repo->idx, template_removed_cb, arg);
	dulge_object_release(allkeys);
	return 0;
}

int
main(int argc, char **argv)
{
	int i, c;
	rcv_t rcv;
	const char *prog = argv[0], *sopts = "hC:D:def:iImR:r:sV";
	const struct option lopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "config", required_argument, NULL, 'C' },
		{ "distdir", required_argument, NULL, 'D' },
		{ "removed", no_argument, NULL, 'e' },
		{ "debug", no_argument, NULL, 'd' },
		{ "format", required_argument, NULL, 'f' },
		{ "installed", no_argument, NULL, 'I' },
		{ "ignore-conf-repos", no_argument, NULL, 'i' },
		{ "manual", no_argument, NULL, 'm' },
		{ "repository", required_argument, NULL, 'R' },
		{ "rootdir", required_argument, NULL, 'r' },
		{ "show-all", no_argument, NULL, 's' },
		{ "version", no_argument, NULL, 'V' },
		{ "staging", no_argument, NULL, 1 },
		{ NULL, 0, NULL, 0 }
	};

	memset(&rcv, 0, sizeof(rcv_t));
	rcv.manual = false;
	rcv.format = "%n %r %s %t %R";

	while ((c = getopt_long(argc, argv, sopts, lopts, NULL)) != -1) {
		switch (c) {
		case 'h':
			return show_usage(prog, false);
		case 'C':
			rcv.dulge_conf = xstrdup(optarg);
			break;
		case 'D':
			rcv.distdir = xstrdup(optarg);
			break;
		case 'd':
			rcv.xhp.flags |= DULGE_FLAG_DEBUG;
			break;
		case 'e':
			rcv.show_removed = true;
			break;
		case 'f':
			rcv.format = optarg;
			break;
		case 'i':
			rcv.xhp.flags |= DULGE_FLAG_IGNORE_CONF_REPOS;
			break;
		case 'I':
			rcv.installed = true;
			break;
		case 'm':
			rcv.manual = true;
			break;
		case 'R':
			dulge_repo_store(&rcv.xhp, optarg);
			break;
		case 'r':
			rcv.rootdir = optarg;
			break;
		case 's':
			rcv.show_all = true;
			break;
		case 'V':
			printf("%s\n", DULGE_RELVER);
			exit(EXIT_SUCCESS);
		case 1:
			rcv.xhp.flags |= DULGE_FLAG_USE_STAGE;
			break;
		case '?':
		default:
			return show_usage(prog, true);
		}
	}
	/*
	 * If --distdir not set default to ~/void-packages.
	 */
	if (rcv.distdir == NULL)
		rcv.distdir = dulge_xasprintf("%s/void-packages", getenv("HOME"));

	{
		char *tmp = rcv.distdir;
		rcv.distdir = realpath(tmp, NULL);
		if (rcv.distdir == NULL) {
			dulge_error_printf("realpath(%s): %s\n", tmp, strerror(errno));
			exit(EXIT_FAILURE);
		}
		free(tmp);
	}

	rcv.cachefile = dulge_xasprintf("%s/.dulge-checkvers-0.58.plist", rcv.distdir);

	argc -= optind;
	argv += optind;

	rcv_init(&rcv, prog);

	if (chdir(rcv.distdir) == -1 || chdir("srcpkgs") == -1) {
		dulge_error_printf("while changing directory to '%s/srcpkgs': %s\n",
		    rcv.distdir, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!rcv.manual)
		rcv_process_dir(&rcv, rcv_process_file);

	if (rcv.show_removed) {
		if (rcv.installed) {
			dulge_pkgdb_foreach_cb(&rcv.xhp, template_removed_cb, &rcv);
		} else {
			dulge_rpool_foreach(&rcv.xhp, repo_templates_removed_cb, &rcv);
		}
	}

	rcv.manual = true;
	for (i = 0; i < argc; i++) {
		char tmp[PATH_MAX] = {0}, *tmpl, *p;
		if (strncmp(argv[i], "srcpkgs/", sizeof ("srcpkgs/")-1) == 0) {
			argv[i] += sizeof ("srcpkgs/")-1;
		}
		if ((p = strrchr(argv[i], '/')) && (strcmp(p, "/template")) == 0) {
			tmpl = argv[i];
		} else {
			dulge_strlcat(tmp, argv[i], sizeof tmp);
			dulge_strlcat(tmp, "/template", sizeof tmp);
			tmpl = tmp;
		}
		rcv_process_file(&rcv, tmpl, rcv_check_version);
	}
	rcv_end(&rcv);
	exit(EXIT_SUCCESS);
}
