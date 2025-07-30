#! /usr/bin/env atf-sh
# Test that dulge-rindex(1) -c (clean mode) works as expected.

# 1st test: make sure that nothing is removed if there are no changes.
atf_test_case noremove

noremove_head() {
	atf_set "descr" "dulge-rindex(1) -c: dont removing anything test"
}

noremove_body() {
	mkdir -p some_repo pkg_A
	cd some_repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-rindex -c some_repo
	atf_check_equal $? 0
	result=$(dulge-query -r root -C empty.conf --repository=some_repo -s foo|wc -l)
	atf_check_equal ${result} 1
}

# dulge issue #19.
# How to reproduce it:
#	Generate pkg foo-1.0_1.
#	Add it to the index of a local repo.
#	Remove the pkg file from the repo.
#	Run dulge-rindex -c on the repo.

atf_test_case issue19

issue19_head() {
	atf_set "descr" "dulge issue #19 (https://github.com/xtraeme/dulge/issues/19)"
}

issue19_body() {
	mkdir -p some_repo
	cd some_repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" .
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	rm some_repo/*.dulge
	dulge-rindex -c some_repo
	atf_check_equal $? 0
	result=$(dulge-query -r root -C empty.conf --repository=some_repo -s foo)
	test -z "${result}"
	atf_check_equal $? 0
}

atf_test_case remove_from_stage

remove_from_stage_head() {
	atf_set "descr" "dulge-rindex(1) -r: don't removing if there's staging test"
}

remove_from_stage_body() {
	mkdir -p some_repo pkg_A pkg_B
	cd some_repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" --shlib-provides "foo.so.1" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n bar-1.0_1 -s "foo pkg" --shlib-requires "foo.so.1" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	dulge-create -A noarch -n foo-1.1_1 -s "foo pkg" --shlib-provides "foo.so.2" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check -o inline:"    2 $PWD (Staged) (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L
	atf_check_equal $? 0
	rm foo-1.1_1*
	dulge-rindex -c .
	atf_check -o inline:"    1 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L
	cd ..
}

atf_init_test_cases() {
	atf_add_test_case noremove
	atf_add_test_case issue19
	atf_add_test_case remove_from_stage
}
