#!/usr/bin/env atf-sh

atf_test_case tc1

tc1_head() {
	atf_set "descr" "Tests for pkg orphans: https://github.com/void-linux/dulge/issues/234"
}

tc1_body() {
	mkdir -p repo pkg_A
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A>=0" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n C-1.0_1 -s "C pkg" --dependencies "B>=0" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n D-1.0_1 -s "D pkg" --dependencies "C>=0" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repo=repo -yd D
	atf_check_equal $? 0
	out="$(dulge-query -r root -m)"
	atf_check_equal $? 0
	atf_check_equal "$out" "D-1.0_1"
	dulge-remove -r root -Ryd D
	atf_check_equal $? 0
	out="$(dulge-query -r root -l|wc -l)"
	atf_check_equal $? 0
	atf_check_equal "$out" "0"

	dulge-install -r root --repo=repo -yd A
	atf_check_equal $? 0
	out="$(dulge-query -r root -m)"
	atf_check_equal $? 0
	atf_check_equal "$out" "A-1.0_1"
	dulge-install -r root --repo=repo -yd D
	atf_check_equal $? 0
	dulge-remove -r root -Ryd D
	atf_check_equal $? 0
	out="$(dulge-query -r root -m)"
	atf_check_equal $? 0
	atf_check_equal "$out" "A-1.0_1"

	dulge-install -r root --repo=repo -yd D
	atf_check_equal $? 0
	dulge-pkgdb -r root -m auto A
	atf_check_equal $? 0
	dulge-remove -r root -Ryd D
	atf_check_equal $? 0
	out="$(dulge-query -r root -l|wc -l)"
	atf_check_equal $? 0
	atf_check_equal "$out" "0"

}

atf_init_test_cases() {
	atf_add_test_case tc1
}
