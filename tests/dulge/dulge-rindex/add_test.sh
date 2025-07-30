#! /usr/bin/env atf-sh
# Test that dulge-rindex(1) -a (add mode) works as expected.

# 1st test: test that update mode work as expected.
atf_test_case update

update_head() {
	atf_set "descr" "dulge-rindex(1) -a: update test"
}

update_body() {
	mkdir -p some_repo pkg_A
	touch pkg_A/file00
	cd some_repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	dulge-create -A noarch -n foo-1.1_1 -s "foo pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	result="$(dulge-query -r root -C empty.conf --repository=some_repo -s '')"
	expected="[-] foo-1.1_1 foo pkg"
	rv=0
	if [ "$result" != "$expected" ]; then
		echo "result: $result"
		echo "expected: $expected"
		rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case revert

revert_head() {
	atf_set "descr" "dulge-rindex(1) -a: revert version test"
}

revert_body() {
	mkdir -p some_repo pkg_A
	touch pkg_A/file00
	cd some_repo
	atf_check -o ignore -e ignore -- dulge-create -A noarch -n foo-1.1_1 -s "foo pkg" ../pkg_A
	atf_check -o ignore -e ignore -- dulge-rindex -d -a $PWD/*.dulge
	atf_check -o ignore -e ignore -- dulge-create -A noarch -n foo-1.0_1 -r "1.1_1" -s "foo pkg" ../pkg_A
	atf_check -o ignore -e ignore -- dulge-rindex -d -a $PWD/*.dulge
	cd ..
	atf_check -o "inline:[-] foo-1.0_1 foo pkg\n" -e empty -- \
		dulge-query -r root -C empty.conf --repository=some_repo -Rs ''
}

atf_test_case stage

stage_head() {
	atf_set "descr" "dulge-rindex(1) -a: commit package to stage test"
}

stage_body() {
	mkdir -p some_repo pkg_A pkg_B
	touch pkg_A/file00 pkg_B/file01
	cd some_repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" --shlib-provides "libfoo.so.1" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    1 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	dulge-create -A noarch -n foo-1.1_1 -s "foo pkg" --shlib-provides "libfoo.so.2" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    1 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	dulge-create -A noarch -n bar-1.0_1 -s "foo pkg" --shlib-requires "libfoo.so.2" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    2 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	dulge-create -A noarch -n foo-1.2_1 -s "foo pkg" --shlib-provides "libfoo.so.3" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    2 $PWD (Staged) (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	dulge-create -A noarch -n bar-1.1_1 -s "foo pkg" --shlib-requires "libfoo.so.3" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    2 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L
}

atf_test_case stage_resolve_bug

stage_resolve_bug_head() {
	atf_set "descr" "dulge-rindex(1) -a: commit package to stage test"
}

stage_resolve_bug_body() {
	# Scanario: provides a shlib, then it is moved (for example for splitting the
	# pkg) to libprovider. afterwards requirer is added resulting in
	mkdir provider libprovider requirer stage-trigger some_repo
	touch provider/1 libprovider/2 requirer/3 stage-trigger/4
	cd some_repo

	# first add the provider and the requirer to the repo
	dulge-create -A noarch -n provider-1.0_1 -s "foo pkg" --shlib-provides "libfoo.so.1 libbar.so.1" ../provider
	atf_check_equal $? 0
	dulge-create -A noarch -n require-1.0_1 -s "foo pkg" --shlib-requires "libfoo.so.1" ../requirer
	atf_check_equal $? 0
	dulge-create -A noarch -n stage-trigger-1.0_1 -s "foo pkg" --shlib-requires "libbar.so.1" ../stage-trigger
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0

	# then add libprovider that also provides the library
	dulge-create -A noarch -n libprovider-1.0_2 -s "foo pkg" --shlib-provides "libfoo.so.1" ../libprovider
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    4 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	# trigger staging
	dulge-create -A noarch -n provider-1.0_2 -s "foo pkg" --shlib-provides "libfoo.so.1" ../provider
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    4 $PWD (Staged) (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	# then add a new provider not containing the provides field. This resulted in
	# a stage state despites the library is resolved through libprovides
	dulge-create -A noarch -n provider-1.0_3 -s "foo pkg" ../provider
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    4 $PWD (Staged) (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L

	# resolve staging
	# the actual bug appeared here: libfoo.so.1 is still provided by libprovider, but
	# dulge-rindex fails to register that.
	dulge-create -A noarch -n stage-trigger-1.0_2 -s "foo pkg" ../stage-trigger
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	atf_check -o inline:"    4 $PWD (RSA unsigned)\n" -- \
		dulge-query -r ../root -i --repository=$PWD -L
}

atf_init_test_cases() {
	atf_add_test_case update
	atf_add_test_case revert
	atf_add_test_case stage
	atf_add_test_case stage_resolve_bug
}
