#!/usr/bin/env atf-sh

atf_test_case downgrade_hold

downgrade_hold_head() {
	atf_set "descr" "Tests for pkg downgrade: pkg is on hold mode"
}

downgrade_hold_body() {
	mkdir -p repo pkg_A
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/repo -yd A
	atf_check_equal $? 0
	dulge-pkgdb -r root -m hold A
	atf_check_equal $? 0
	out=$(dulge-query -r root -H)
	atf_check_equal $out A-1.0_1
	cd repo
	dulge-create -A noarch -n A-0.1_1 -s "A pkg" -r "1.0_1" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/repo -yuvd
	atf_check_equal $? 0
	out=$(dulge-query -r root -p pkgver A)
	atf_check_equal $out A-1.0_1
}

atf_test_case hold_shlibs

hold_shlibs_head() {
	atf_set "descr" "Tests for pkgs on hold mode: verify shlibs"
}

hold_shlibs_body() {
	mkdir -p repo pkg
	cd repo
	dulge-create -A noarch -n sway-1.2_1 -s "sway" --dependencies "wlroots>=0.9" --shlib-requires "libwlroots.so.4" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n wlroots-0.9.0_1 -s "wlroots" --shlib-provides "libwlroots.so.4" ../pkg
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root --repo=repo -yd sway
	atf_check_equal $? 0
	dulge-pkgdb -r root -m hold sway
	atf_check_equal $? 0
	out=$(dulge-query -r root -H)
	atf_check_equal $out sway-1.2_1

	cd repo
	dulge-create -A noarch -n sway-1.4_1 -s "sway" --dependencies "wlroots>=0.10" --shlib-requires "libwlroots.so.5" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n wlroots-0.10.0_1 -s "wlroots" --shlib-provides "libwlroots.so.5" ../pkg
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	# returns ENOEXEC because sway can't be upgraded
	dulge-install -r root --repo=repo -yud
	atf_check_equal $? 8

	out=$(dulge-query -r root -p pkgver sway)
	atf_check_equal "$out" "sway-1.2_1"

	out=$(dulge-query -r root -p pkgver wlroots)
	atf_check_equal "$out" "wlroots-0.9.0_1"

	dulge-pkgdb -r root -a
	atf_check_equal $? 0

	# unhold to verify
	dulge-pkgdb -r root -m unhold sway

	dulge-install -r root --repo=repo -yud
	atf_check_equal $? 0

	out=$(dulge-query -r root -p pkgver sway)
	atf_check_equal "$out" "sway-1.4_1"

	out=$(dulge-query -r root -p pkgver wlroots)
	atf_check_equal "$out" "wlroots-0.10.0_1"

	dulge-pkgdb -r root -a
	atf_check_equal $? 0
}

atf_test_case keep_on_update

keep_on_update_head() {
	atf_set "descr" "Tests for pkgs on hold: keep prop on update"
}

keep_on_update_body() {
	mkdir -p repo pkg_A
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/repo -yd A
	atf_check_equal $? 0
	dulge-pkgdb -r root -m hold A
	atf_check_equal $? 0
	out=$(dulge-query -r root -H)
	atf_check_equal $out A-1.0_1
	cd repo
	dulge-create -A noarch -n A-1.1_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	# no update
	dulge-install -r root --repository=$PWD/repo -yuvd
	atf_check_equal $? 0
	out=$(dulge-query -r root -p pkgver A)
	atf_check_equal $out A-1.0_1
	# no update without -f
	dulge-install -r root --repository=$PWD/repo -yuvd A
	atf_check_equal $? 0
	out=$(dulge-query -r root -p pkgver A)
	atf_check_equal $out A-1.0_1
	# no update with -fu
	dulge-install -r root --repository=$PWD/repo -yuvdf
	atf_check_equal $? 0
	out=$(dulge-query -r root -p pkgver A)
	atf_check_equal $out A-1.0_1
	# update with -f
	dulge-install -r root --repository=$PWD/repo -yuvdf A
	atf_check_equal $? 0
	out=$(dulge-query -r root -p pkgver A)
	atf_check_equal $out A-1.1_1
	out=$(dulge-query -r root -p hold A)
	atf_check_equal $out yes
}

atf_init_test_cases() {
	atf_add_test_case downgrade_hold
	atf_add_test_case hold_shlibs
	atf_add_test_case keep_on_update
}
