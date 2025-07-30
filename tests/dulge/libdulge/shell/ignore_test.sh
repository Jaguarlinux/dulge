#!/usr/bin/env atf-sh

atf_test_case install_with_ignored_dep

install_with_ignored_dep_head() {
	atf_set "descr" "Tests for pkg install: with ignored dependency"
}

install_with_ignored_dep_body() {
	mkdir -p repo root/dulge.d pkg_A pkg_B
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" -D "B-1.0_1" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	echo "ignorepkg=B" > root/dulge.d/ignore.conf
	out=$(dulge-install -r root -C dulge.d --repository=$PWD/repo -n A)
	set -- $out
	exp="$1 $2 $3 $4"
	atf_check_equal "$exp" "A-1.0_1 install noarch $PWD/repo"
	dulge-install -r root -C dulge.d --repository=$PWD/repo -yd A
	atf_check_equal $? 0
	dulge-query -r root A
	atf_check_equal $? 0
	dulge-query -r root B
	atf_check_equal $? 2
}

atf_test_case update_with_ignored_dep

update_with_ignored_dep_head() {
	atf_set "descr" "Tests for pkg update: with ignored dependency"
}

update_with_ignored_dep_body() {
	mkdir -p repo root/dulge.d pkg_A pkg_B
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" -D "B-1.0_1" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	echo "ignorepkg=B" > root/dulge.d/ignore.conf
	dulge-install -r root -C dulge.d --repository=$PWD/repo -yd A
	atf_check_equal $? 0
	dulge-query -r root B
	atf_check_equal $? 2
	cd repo
	dulge-create -A noarch -n A-1.1_1 -s "A pkg" -D "B-1.0_1" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	out=$(dulge-install -r root -C dulge.d --repository=$PWD/repo -un)
	set -- $out
	exp="$1 $2 $3 $4"
	atf_check_equal "$exp" "A-1.1_1 update noarch $PWD/repo"
	dulge-install -r root -C dulge.d --repository=$PWD/repo -yuvd
	atf_check_equal $? 0
	out=$(dulge-query -r root -p pkgver A)
	atf_check_equal $out A-1.1_1
	dulge-query -r root B
	atf_check_equal $? 2
}

atf_test_case remove_with_ignored_dep

remove_with_ignored_dep_head() {
	atf_set "descr" "Tests for pkg remove: with ignored dependency"
}

remove_with_ignored_dep_body() {
	mkdir -p repo root/dulge.d pkg_A pkg_B
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" -D "B-1.0_1" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	echo "ignorepkg=B" > root/dulge.d/ignore.conf
	dulge-install -r root -C dulge.d --repository=$PWD/repo -yd A
	atf_check_equal $? 0
	dulge-query -r root B
	atf_check_equal $? 2
	out=$(dulge-remove -r root -C dulge.d -Rn A)
	set -- $out
	exp="$1 $2 $3 $4"
	atf_check_equal "$exp" "A-1.0_1 remove noarch $PWD/repo"
	dulge-remove -r root -C dulge.d -Ryvd A
	atf_check_equal $? 0
	dulge-query -r root A
	atf_check_equal $? 2
	dulge-query -r root B
	atf_check_equal $? 2
}

atf_test_case remove_ignored_dep

remove_ignored_dep_head() {
	atf_set "descr" "Tests for pkg remove: pkg is dependency but ignored"
}

remove_ignored_dep_body() {
	mkdir -p repo root/dulge.d pkg_A pkg_B
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" -D "B-1.0_1" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C dulge.d --repository=$PWD/repo -yd A
	atf_check_equal $? 0
	echo "ignorepkg=B" > root/dulge.d/ignore.conf
	out=$(dulge-remove -r root -C dulge.d -Rn B)
	set -- $out
	exp="$1 $2 $3 $4"
	atf_check_equal "$exp" "B-1.0_1 remove noarch $PWD/repo"
	dulge-remove -r root -C dulge.d -Ryvd B
	atf_check_equal $? 0
	dulge-query -r root A
	atf_check_equal $? 0
	dulge-query -r root B
	atf_check_equal $? 2
}

atf_init_test_cases() {
	atf_add_test_case install_with_ignored_dep
	atf_add_test_case update_with_ignored_dep
	atf_add_test_case remove_with_ignored_dep
	atf_add_test_case remove_ignored_dep
}
