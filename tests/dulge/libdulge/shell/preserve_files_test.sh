#!/usr/bin/env atf-sh

atf_test_case tc1

tc1_head() {
	atf_set "descr" "Tests for pkg install/upgrade with preserved files: preserve on-disk files with globs"
}

tc1_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin
	echo "blahblah" > pkg_A/usr/bin/blah
	echo "foofoo" > pkg_A/usr/bin/foo
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	mkdir -p root/usr/bin
	mkdir -p root/dulge.d
	echo "modified blahblah" > root/usr/bin/blah
	echo "modified foofoo" > root/usr/bin/foo

	echo "preserve=/usr/bin/*" > root/dulge.d/foo.conf

	dulge-install -C dulge.d -r root --repository=$PWD/some_repo -yd A
	atf_check_equal $? 0

	rv=1
	if [ "$(cat root/usr/bin/blah)" = "modified blahblah" -a "$(cat root/usr/bin/foo)" = "modified foofoo" ]; then
		rv=0
	fi
	atf_check_equal $rv 0
}

atf_test_case tc2

tc2_head() {
	atf_set "descr" "Tests for pkg install/upgrade with preserved files: preserve on-disk files without globs"
}

tc2_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin
	echo "blahblah" > pkg_A/usr/bin/blah
	echo "foofoo" > pkg_A/usr/bin/foo
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	mkdir -p root/usr/bin
	mkdir -p root/dulge.d
	echo "modified blahblah" > root/usr/bin/blah
	echo "modified foofoo" > root/usr/bin/foo

	printf "preserve=/usr/bin/blah\npreserve=/usr/bin/foo\n" > root/dulge.d/foo.conf

	echo "foo.conf" >&2
	cat foo.conf >&2

	dulge-install -C dulge.d -r root --repository=$PWD/some_repo -yd A
	atf_check_equal $? 0

	rv=1
	if [ "$(cat root/usr/bin/blah)" = "modified blahblah" -a "$(cat root/usr/bin/foo)" = "modified foofoo" ]; then
		rv=0
	fi

	echo "root/usr/bin/blah" >&2
	cat root/usr/bin/blah >&2
	echo "root/usr/bin/foo" >&2
	cat root/usr/bin/foo >&2

	atf_check_equal $rv 0
}

atf_init_test_cases() {
	atf_add_test_case tc1
	atf_add_test_case tc2
}
