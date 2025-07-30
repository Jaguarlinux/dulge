/*-
 * Copyright (c) 2013-2014 Juan Romero Pardines.
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <openssl/err.h>
#include <openssl/sha.h>
#include <openssl/rsa.h>
#include <openssl/ssl.h>
#include <openssl/pem.h>

#include "dulge_api_impl.h"

static bool
rsa_verify_hash(struct dulge_repo *repo, dulge_data_t pubkey,
		unsigned char *sig, unsigned int siglen,
		unsigned char *sha256)
{
	BIO *bio;
	RSA *rsa;
	int rv;

	ERR_load_crypto_strings();
	SSL_load_error_strings();

	bio = BIO_new_mem_buf(dulge_data_data_nocopy(pubkey),
			dulge_data_size(pubkey));
	assert(bio);

	rsa = PEM_read_bio_RSA_PUBKEY(bio, NULL, NULL, NULL);
	if (rsa == NULL) {
		dulge_dbg_printf("`%s' error reading public key: %s\n",
		    repo->uri, ERR_error_string(ERR_get_error(), NULL));
		return false;
	}

	rv = RSA_verify(NID_sha256, sha256, SHA256_DIGEST_LENGTH, sig, siglen, rsa);
	RSA_free(rsa);
	BIO_free(bio);
	ERR_free_strings();

	return rv ? true : false;
}

bool
dulge_verify_signature(struct dulge_repo *repo, const char *sigfile,
		unsigned char *digest)
{
	dulge_dictionary_t repokeyd = NULL;
	dulge_data_t pubkey;
	char *hexfp = NULL;
	unsigned char *sig_buf = NULL;
	size_t sigbuflen, sigfilelen;
	char *rkeyfile = NULL;
	bool val = false;

	if (!dulge_dictionary_count(repo->idxmeta)) {
		dulge_dbg_printf("%s: unsigned repository\n", repo->uri);
		return false;
	}
	hexfp = dulge_pubkey2fp(dulge_dictionary_get(repo->idxmeta, "public-key"));
	if (hexfp == NULL) {
		dulge_dbg_printf("%s: incomplete signed repo, missing hexfp obj\n", repo->uri);
		return false;
	}

	/*
	 * Prepare repository RSA public key to verify fname signature.
	 */
	rkeyfile = dulge_xasprintf("%s/keys/%s.plist", repo->xhp->metadir, hexfp);
	repokeyd = dulge_plist_dictionary_from_file(rkeyfile);
	if (dulge_object_type(repokeyd) != DULGE_TYPE_DICTIONARY) {
		dulge_dbg_printf("cannot read rkey data at %s: %s\n",
		    rkeyfile, strerror(errno));
		goto out;
	}

	pubkey = dulge_dictionary_get(repokeyd, "public-key");
	if (dulge_object_type(pubkey) != DULGE_TYPE_DATA)
		goto out;

	if (!dulge_mmap_file(sigfile, (void *)&sig_buf, &sigbuflen, &sigfilelen)) {
		dulge_dbg_printf("can't open signature file %s: %s\n",
		    sigfile, strerror(errno));
		goto out;
	}
	/*
	 * Verify fname RSA signature.
	 */
	if (rsa_verify_hash(repo, pubkey, sig_buf, sigfilelen, digest))
		val = true;

out:
	if (hexfp)
		free(hexfp);
	if (rkeyfile)
		free(rkeyfile);
	if (sig_buf)
		(void)munmap(sig_buf, sigbuflen);
	if (repokeyd)
		dulge_object_release(repokeyd);

	return val;
}

bool
dulge_verify_file_signature(struct dulge_repo *repo, const char *fname)
{
	char sig[PATH_MAX];
	unsigned char digest[DULGE_SHA256_DIGEST_SIZE];
	bool val = false;

	if (!dulge_file_sha256_raw(digest, sizeof digest, fname)) {
		dulge_dbg_printf("can't open file %s: %s\n", fname, strerror(errno));
		return false;
	}

	snprintf(sig, sizeof sig, "%s.sig2", fname);
	val = dulge_verify_signature(repo, sig, digest);

	return val;
}
