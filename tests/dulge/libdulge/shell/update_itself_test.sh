#!/usr/bin/env atf-sh

atf_test_case update_dulge

update_dulge_head() {
	atf_set "descr" "Tests for pkg updates: dulge autoupdates itself"
}

update_dulge_body() {
	mkdir -p repo dulge
	touch dulge/foo

	cd repo
	dulge-create -A noarch -n dulge-1.0_1 -s "dulge pkg" ../dulge
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yd dulge
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out dulge-1.0_1

	cd repo
	dulge-create -A noarch -n dulge-1.1_1 -s "dulge pkg" ../dulge
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/dulge-1.1_1.noarch.dulge
	atf_check_equal $? 0
	cd ..

	# EBUSY
	dulge-install -r root --repository=$PWD/repo -yud
	atf_check_equal $? 16

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out dulge-1.0_1

	dulge-install -r root --repository=$PWD/repo -yu dulge
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out dulge-1.1_1
}

atf_test_case update_dulge_with_revdeps

update_dulge_with_revdeps_head() {
	atf_set "descr" "Tests for pkg updates: dulge updates itself with revdeps"
}

update_dulge_with_revdeps_body() {
	mkdir -p repo dulge dulge-dbg baz
	touch dulge/foo dulge-dbg/bar baz/blah

	cd repo
	dulge-create -A noarch -n dulge-1.0_1 -s "dulge pkg" ../dulge
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yd dulge-1.0_1
	atf_check_equal $? 0

	cd repo
	dulge-create -A noarch -n baz-1.0_1 -s "baz pkg" ../baz
	atf_check_equal $? 0
	dulge-create -A noarch -n dulge-dbg-1.0_1 -s "dulge-dbg pkg" --dependencies "dulge-1.0_1" ../dulge-dbg
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yd dulge-dbg baz
	atf_check_equal $? 0

	cd repo
	dulge-create -A noarch -n dulge-1.1_1 -s "dulge pkg" ../dulge
	atf_check_equal $? 0
	dulge-create -A noarch -n baz-1.1_1 -s "baz pkg" ../baz
	atf_check_equal $? 0
	dulge-create -A noarch -n dulge-dbg-1.1_1 -s "dulge-dbg pkg" --dependencies "dulge-1.1_1" ../dulge-dbg
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	# first time, dulge must be updated (returns EBUSY)
	dulge-install -r root --repository=$PWD/repo -yud
	atf_check_equal $? 16

	dulge-install -r root --repository=$PWD/repo -yu dulge
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out dulge-1.1_1

	out=$(dulge-query -r root -p pkgver dulge-dbg)
	atf_check_equal $out dulge-dbg-1.1_1

	out=$(dulge-query -r root -p pkgver baz)
	atf_check_equal $out baz-1.0_1

	# second time, updates everything
	dulge-install -r root --repository=$PWD/repo -yud
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out dulge-1.1_1

	out=$(dulge-query -r root -p pkgver dulge-dbg)
	atf_check_equal $out dulge-dbg-1.1_1

	out=$(dulge-query -r root -p pkgver baz)
	atf_check_equal $out baz-1.1_1
}

atf_test_case update_dulge_with_uptodate_revdeps

update_dulge_with_uptodate_revdeps_head() {
	atf_set "descr" "Tests for pkg updates: dulge updates itself with already up-to-date revdeps"
}

update_dulge_with_uptodate_revdeps_body() {
	mkdir -p repo dulge base-system
	touch dulge/foo base-system/bar

	cd repo
	dulge-create -A noarch -n dulge-1.0_1 -s "dulge pkg" ../dulge
	atf_check_equal $? 0
	dulge-create -A noarch -n base-system-1.0_1 -s "base-system pkg" --dependencies "dulge>=0" ../base-system
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yd base-system
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out "dulge-1.0_1"

	out=$(dulge-query -r root -p pkgver base-system)
	atf_check_equal $out "base-system-1.0_1"

	cd repo
	dulge-create -A noarch -n dulge-1.1_1 -s "dulge pkg" ../dulge
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yud
	atf_check_equal $? 16

	dulge-install -r root --repository=$PWD/repo -yu dulge
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal $out dulge-1.1_1

	out=$(dulge-query -r root -p pkgver base-system)
	atf_check_equal $out base-system-1.0_1
}

atf_test_case update_dulge_with_indirect_revdeps

update_dulge_with_indirect_revdeps_head() {
	atf_set "descr" "Tests for pkg updates: dulge updates itself with indirect revdeps"
}

update_dulge_with_indirect_revdeps_body() {
	mkdir -p repo pkg

	cd repo
	dulge-create -A noarch -n dulge-1.0_1 -s "dulge pkg" --dependencies "libcrypto-1.0_1 cacerts>=0" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n libcrypto-1.0_1 -s "libcrypto pkg" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n libressl-1.0_1 -s "libressl pkg" --dependencies "libcrypto-1.0_1" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n cacerts-1.0_1 -s "cacerts pkg" --dependencies "libressl>=0" ../pkg
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yd dulge-1.0_1
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal "$out" "dulge-1.0_1"

	out=$(dulge-query -r root -p pkgver libcrypto)
	atf_check_equal "$out" "libcrypto-1.0_1"

	out=$(dulge-query -r root -p pkgver libressl)
	atf_check_equal "$out" "libressl-1.0_1"

	out=$(dulge-query -r root -p pkgver cacerts)
	atf_check_equal "$out" "cacerts-1.0_1"

	cd repo
	dulge-create -A noarch -n dulge-1.1_1 -s "dulge pkg" --dependencies "libcrypto-1.1_1 ca-certs>=0" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n libcrypto-1.1_1 -s "libcrypto pkg" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n libressl-1.1_1 -s "libressl pkg" --dependencies "libcrypto-1.1_1" ../pkg
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repository=$PWD/repo -yu dulge
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal "$out" "dulge-1.1_1"

	out=$(dulge-query -r root -p pkgver libcrypto)
	atf_check_equal "$out" "libcrypto-1.1_1"

	out=$(dulge-query -r root -p pkgver libressl)
	atf_check_equal "$out" "libressl-1.0_1"

	out=$(dulge-query -r root -p pkgver cacerts)
	atf_check_equal "$out" "cacerts-1.0_1"

	dulge-install -r root --repository=$PWD/repo -yu
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver dulge)
	atf_check_equal "$out" "dulge-1.1_1"

	out=$(dulge-query -r root -p pkgver libcrypto)
	atf_check_equal "$out" "libcrypto-1.1_1"

	out=$(dulge-query -r root -p pkgver libressl)
	atf_check_equal "$out" "libressl-1.1_1"

	out=$(dulge-query -r root -p pkgver cacerts)
	atf_check_equal "$out" "cacerts-1.0_1"
}

atf_init_test_cases() {
	atf_add_test_case update_dulge
	atf_add_test_case update_dulge_with_revdeps
	atf_add_test_case update_dulge_with_indirect_revdeps
	atf_add_test_case update_dulge_with_uptodate_revdeps
}
