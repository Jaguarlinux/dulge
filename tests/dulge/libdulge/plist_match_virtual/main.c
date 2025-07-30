/*-
 * Copyright (c) 2012-2015 Juan Romero Pardines.
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
#include <atf-c.h>
#include <dulge.h>

static dulge_array_t
rundeps_init(void)
{
	dulge_array_t a;

	a = dulge_array_create();
	ATF_REQUIRE(a != NULL);

	dulge_array_add_cstring_nocopy(a, "cron-daemon>=0");
	dulge_array_add_cstring_nocopy(a, "dulge>=0.14");

	return a;
}

static dulge_array_t
provides_init(void)
{
	dulge_array_t a;

	a = dulge_array_create();
	ATF_REQUIRE(a != NULL);

	dulge_array_add_cstring_nocopy(a, "cron-daemon-0_1");
	dulge_array_add_cstring_nocopy(a, "dulge-9999_1");

	return a;
}

static dulge_dictionary_t
pkgdict_init(void)
{
	dulge_array_t a;
	dulge_dictionary_t d;

	d = dulge_dictionary_create();
	ATF_REQUIRE(d != NULL);

	a = provides_init();
	ATF_REQUIRE_EQ(dulge_dictionary_set(d, "provides", a), true);

	return d;
}

ATF_TC(match_virtual_pkg_dict_test);
ATF_TC_HEAD(match_virtual_pkg_dict_test, tc)
{
	atf_tc_set_md_var(tc, "descr", "Test dulge_match_virtual_pkg_in_dict");
}

ATF_TC_BODY(match_virtual_pkg_dict_test, tc)
{
	dulge_dictionary_t d;

	d = pkgdict_init();
	ATF_REQUIRE_EQ(dulge_match_virtual_pkg_in_dict(d, "cron-daemon"), true);
	ATF_REQUIRE_EQ(dulge_match_virtual_pkg_in_dict(d, "cron-daemon>=0"), true);
	ATF_REQUIRE_EQ(dulge_match_virtual_pkg_in_dict(d, "cron-daemon>2"), false);
}

ATF_TC(match_any_virtualpkg_rundeps_test);
ATF_TC_HEAD(match_any_virtualpkg_rundeps_test, tc)
{
	atf_tc_set_md_var(tc, "descr", "Test dulge_match_any_virtualpkg_in_rundeps");
}

ATF_TC_BODY(match_any_virtualpkg_rundeps_test, tc)
{
	dulge_array_t provides, rundeps;

	provides = provides_init();
	rundeps = rundeps_init();

	ATF_REQUIRE_EQ(
		dulge_match_any_virtualpkg_in_rundeps(rundeps, provides), true);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, match_virtual_pkg_dict_test);
	ATF_TP_ADD_TC(tp, match_any_virtualpkg_rundeps_test);

	return atf_no_error();
}
