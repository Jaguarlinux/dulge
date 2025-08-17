#!/usr/bin/env atf-sh
#
atf_test_case incorrect_dep

incorrect_dep_head() {
	atf_set "descr" "Tests for package deps: pkg depends on itself"
}

incorrect_dep_body() {
	mkdir some_repo
	mkdir -p pkg_B/usr/bin
	echo "B-1.0_1" > pkg_B/usr/bin/foo
	cd some_repo
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "B>=0" ../pkg_B
	atf_check_equal $? 0

	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -C empty.conf -r root --repository=$PWD/some_repo -dy B
	atf_check_equal $? 0
}

atf_test_case incorrect_dep_vpkg

incorrect_dep_vpkg_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin
	echo "A-1.0_1" > pkg_A/usr/bin/foo
	echo "B-1.0_1" > pkg_B/usr/bin/foo
	cd some_repo
	dulge-create -A noarch -n A-10.1.1_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A>=7.11_1" --provides "A-331.67_1" ../pkg_B
	atf_check_equal $? 0

	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -C empty.conf -r root --repository=$PWD/some_repo -dy A
	atf_check_equal $? 0
	dulge-install -C empty.conf -r root --repository=$PWD/some_repo -dy B
	atf_check_equal $? 0
}

atf_test_case incorrect_dep_issue45

incorrect_dep_issue45_head() {
	atf_set "descr" "Test for package deps: pkg depends on itself (issue #45: https://github.com/jaguarlinux/dulge/issues/45)"
}

incorrect_dep_issue45_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin pkg_C/usr/bin pkg_D/usr/bin
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" --dependencies "A>=0" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A>=0" ../pkg_B
	atf_check_equal $? 0

	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -C empty.conf -r root --repository=$PWD/some_repo -dy B
	atf_check_equal $? 0
	dulge-query -C empty.conf -r root -l
	atf_check_equal $? 0
	dulge-pkgdb -C empty.conf -r root -a
	atf_check_equal $? 0
}

atf_test_case incorrect_dep_dups

incorrect_dep_dups_head() {
	atf_set "descr" "Test for package deps: duplicated deps in fulldeptree"
}

incorrect_dep_dups_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A>=0 A>=0" ../pkg_B
	atf_check_equal $? 0

	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -C empty.conf -r root --repository=$PWD/some_repo -dy B
	atf_check_equal $? 0

	out=$(dulge-query -C empty.conf -r root --fulldeptree -x B)
	set -- $out
	atf_check_equal $# 1
	atf_check_equal "$1" "A-1.0_1"
}

atf_test_case missing_deps

missing_deps_head() {
	atf_set "descr" "Test for package deps: pkg depends on a missing dependency in fulldeptree"
}

missing_deps_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A>=0 C>=0" ../pkg_B
	atf_check_equal $? 0

	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	# reverse deps
	dulge-query -C empty.conf -r root --repository=some_repo -dX B
	atf_check_equal $? 2 # ENOENT

	# fulldeptree repo
	dulge-query -C empty.conf --repository=some_repo -r root --fulldeptree -dx B
	atf_check_equal $? 19 # ENODEV

	# fulldeptree pkgdb
	dulge-query -C empty.conf -r root --fulldeptree -x C
	atf_check_equal $? 2 # ENOENT

}

atf_test_case multiple_versions

multiple_versions_head() {
	atf_set "descr" "Test for package deps: two versions of the same pkg in multiple repos"
}

multiple_versions_body() {
	mkdir -p repo repo2 pkg_A/usr/bin pkg/usr/bin
	touch pkg_A/usr/bin/foo
	cd repo
	dulge-create -A noarch -n kconfig-5.66_1 -s "kconfig" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a *.dulge
	atf_check_equal $? 0
	cd ../repo2
	dulge-create -A noarch -n kconfigwidgets-1.0_1 -s "kconfigwidgets" --dependencies "kconfig>=5.60_1" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n kconfig-5.67_1 -s "kconfig" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n kconfig-devel-5.67_1 -s "kconfig-devel" --dependencies "kconfig>=5.67_1" ../pkg
	atf_check_equal $? 0
	dulge-create -A noarch -n foo-1.0_1 -s "foo" --dependencies "kconfig-devel>=5.67_1" ../pkg
	atf_check_equal $? 0
	dulge-rindex -d -a *.dulge
	atf_check_equal $? 0
	cd ..

	out=$(dulge-install -r root --repo=repo --repo=repo2 -n kconfigwidgets foo|wc -l)
	atf_check_equal $? 0
	atf_check_equal "$out" 4

	dulge-install -r root --repo=repo --repo=repo2 -yd kconfigwidgets foo
	atf_check_equal $? 0

	dulge-pkgdb -r root -a
	atf_check_equal $? 0

	out=$(dulge-query -r root -l|wc -l)
	atf_check_equal $? 0
	atf_check_equal "$out" 4
}

atf_init_test_cases() {
	atf_add_test_case incorrect_dep
	atf_add_test_case incorrect_dep_vpkg
	atf_add_test_case incorrect_dep_issue45
	atf_add_test_case incorrect_dep_dups
	atf_add_test_case missing_deps
	atf_add_test_case multiple_versions
}
