#! /usr/bin/env atf-sh

# 1st test: make sure that base symlinks on rootdir are not removed.
atf_test_case keep_base_symlinks

keep_base_symlinks_head() {
	atf_set "descr" "Tests for package removal: keep base symlinks test"
}

keep_base_symlinks_body() {
	mkdir -p root/usr/bin root/usr/lib root/run root/var
	ln -sf usr/bin root/bin
	ln -sf usr/lib root/lib
	ln -sf lib root/usr/lib32
	ln -sf lib root/usr/lib64
	ln -sf ../../run root/var/run

	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_A/bin pkg_A/usr/lib pkg_A/var
	touch -f pkg_A/usr/bin/foo
	ln -sf lib pkg_A/usr/lib32
	ln -sf lib pkg_A/usr/lib64
	ln -sf ../../run pkg_A/var/run

	cd some_repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y foo
	atf_check_equal $? 0
	dulge-remove -r root -y foo
	atf_check_equal $? 0
	if [ -h root/bin -a -h root/lib -a -h root/usr/lib32 -a -h root/usr/lib64 -a -h root/var/run ]; then
		rv=0
	else
		rv=1
	fi
	atf_check_equal $rv 0
}

# 2nd test: make sure that all symlinks are removed.
atf_test_case remove_symlinks

remove_symlinks_head() {
	atf_set "descr" "Tests for package removal: symlink cleanup test"
}

remove_symlinks_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/lib pkg_B/usr/lib
	touch -f pkg_A/usr/lib/libfoo.so.1.2.0
	ln -sf libfoo.so.1.2.0 pkg_A/usr/lib/libfoo.so.1
	ln -sf libfoo.so.1 pkg_B/usr/lib/libfoo.so

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 --dependencies "A>=0" -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/some_repo -y B
	atf_check_equal $? 0
	dulge-pkgdb -r root -m manual A
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd B
	atf_check_equal $? 0
	rv=0
	if [ -h root/usr/lib/libfoo.so ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

remove_symlinks_dangling_head() {
	atf_set "descr" "Tests for package removal: dangling symlink cleanup test"
}

remove_symlinks_dangling_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/lib
	touch -f pkg_A/usr/lib/libfoo.so.1.2.0
	ln -sf libfoo.so.2 pkg_A/usr/lib/libfoo.so.1
	ln -sf libfoo.so.2 pkg_A/usr/lib/libfoo.so
	ln -s ./libfoo.so pkg_A/usr/lib/libfoo.3
	ln -s ./libfoo.so.4 pkg_A/usr/lib/libfoo.4
	ln -s ../../../libfoo.so.4.1 pkg_A/usr/lib/libfoo.4.1

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd A
	atf_check_equal $? 0
	rv=0
	if [ -h root/usr/lib/libfoo.so.1 -o -h root/usr/lib/libfoo.so -o -f root/usr/lib/libfoo.so.1.2.o -o -d root/usr/lib ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_symlinks_relative

remove_symlinks_relative_head() {
	atf_set "descr" "Tests for package removal: relative symlink cleanup test"
}

remove_symlinks_relative_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/lib pkg_A/usr/share/lib
	touch -f pkg_A/usr/lib/libfoo.so.1.2.0
	ln -sfr pkg_A/usr/lib/libfoo.so.1.2.0 pkg_A/usr/share/lib/libfoo.so.1

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd A
	atf_check_equal $? 0
	rv=0
	if [ -h root/usr/share/lib/libfoo.so.1 -o -f root/usr/lib/libfoo.so.1.2.0 -o -d root/usr/share/lib -o -d root/usr/lib ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_symlinks_relative2

remove_symlinks_relative2_head() {
	atf_set "descr" "Tests for package removal: 2nd relative symlink cleanup test"
}

remove_symlinks_relative2_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/lib
	touch -f pkg_A/usr/lib/libfoo.so.1.2.0
	ln -s libfoo.so.1.2.0 pkg_A/usr/lib/libfoo.so.1

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd A
	atf_check_equal $? 0
	rv=0
	if [ -h root/usr/lib/libfoo.so.1 -o -f root/usr/lib/libfoo.so.1.2.0 -o -d root/usr/lib ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_symlinks_abs

remove_symlinks_abs_head() {
	atf_set "descr" "Tests for package removal: absolute symlink cleanup test"
}

remove_symlinks_abs_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/lib
	touch -f pkg_A/usr/lib/libfoo.so.1.2.0
	ln -s /usr/lib/libfoo.so.1.2.0 pkg_A/usr/lib/libfoo.so.1

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd A
	atf_check_equal $? 0
	rv=0
	if [ -h root/usr/lib/libfoo.so.1 -o -f root/usr/lib/libfoo.so.1.2.0 -o -d root/usr/lib ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

# 3rd test: make sure that symlinks to the rootfs are also removed.
atf_test_case remove_symlinks_from_root

remove_symlinks_from_root_head() {
	atf_set "descr" "Tests for package removal: symlink from root test"
}

remove_symlinks_from_root_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	ln -sf /bin/bash pkg_A/bin/bash
	ln -sf /bin/bash pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd A
	atf_check_equal $? 0
	rv=0
	if [ -h root/bin/bash -o -h root/bin/sh ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case keep_modified_symlinks

keep_modified_symlinks_head() {
	atf_set "descr" "Tests for package removal: keep modified symlinks in rootdir"
}

keep_modified_symlinks_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	ln -sf /bin/bash pkg_A/bin/ash
	ln -sf /bin/bash pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	ln -sf /bin/foo root/bin/sh
	ln -sf foo root/bin/ash

	dulge-remove -r root -yvd A
	atf_check_equal $? 0
	rv=1
	if [ -h root/bin/sh -a -h root/bin/ash ]; then
	        rv=0
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_symlinks_modified

remove_symlinks_modified_head() {
	atf_set "descr" "Tests for package removal: force remove modified symlinks in rootdir"
}

remove_symlinks_modified_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	ln -sf /bin/bash pkg_A/bin/ash
	ln -sf /bin/bash pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	ln -sf /bin/foo root/bin/sh

	dulge-remove -r root -yvfd A
	atf_check_equal $? 0
	rv=0
	if [ -h root/bin/sh -o -h root/bin/ash ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_readonly_files

remove_readonly_files_head() {
	atf_set "descr" "Tests for package removal: read-only files"
}

remove_readonly_files_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin
	echo kjaskjas > pkg_A/usr/bin/foo
	echo sskjas > pkg_A/usr/bin/blah
	chmod 444 pkg_A/usr/bin/foo
	chmod 444 pkg_A/usr/bin/blah

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -yv A
	atf_check_equal $? 0
	dulge-remove -r root -yv A
	atf_check_equal $? 0
	rv=0
	if [ -r root/usr/bin/foo -a -r root/usr/bin/blah ]; then
	        rv=1
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_dups

remove_dups_head() {
	atf_set "descr" "Tests for package removal: remove pkg multiple times"
}

remove_dups_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -yv A
	atf_check_equal $? 0
	out=$(dulge-remove -r root -yvn A A A|wc -l)
	atf_check_equal $out 1
}

atf_test_case remove_with_revdeps

remove_with_revdeps_head() {
	atf_set "descr" "Tests for package removal: remove a pkg with revdeps"
}

remove_with_revdeps_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A-1.0_1" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=some_repo -yvd B
	atf_check_equal $? 0
	dulge-remove -r root -yvd A
	# ENODEV == unresolved dependencies
	atf_check_equal $? 19
}

atf_test_case remove_with_revdeps_force

remove_with_revdeps_force_head() {
	atf_set "descr" "Tests for package removal: remove a pkg with revdeps (force)"
}

remove_with_revdeps_force_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A-1.0_1" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=some_repo -yvd B
	atf_check_equal $? 0
	dulge-remove -r root -Fyvd A
	atf_check_equal $? 0
	out=$(dulge-query -r root -l|wc -l)
	atf_check_equal $out 1
	out=$(dulge-query -r root -ppkgver B)
	atf_check_equal $out B-1.0_1
}

atf_test_case remove_with_revdeps_in_trans

remove_with_revdeps_in_trans_head() {
	atf_set "descr" "Tests for package removal: remove a pkg with its revdeps in transaction"
}

remove_with_revdeps_in_trans_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A-1.0_1" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=some_repo -yvd B
	atf_check_equal $? 0
	dulge-remove -r root -yvd A B
	atf_check_equal $? 0
}

atf_test_case remove_with_revdeps_in_trans_inverted

remove_with_revdeps_in_trans_inverted_head() {
	atf_set "descr" "Tests for package removal: remove a pkg with its revdeps in transaction (inverted)"
}

remove_with_revdeps_in_trans_inverted_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A-1.0_1" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=some_repo -yvd B
	atf_check_equal $? 0
	dulge-remove -r root -yvd B A
	atf_check_equal $? 0
}

atf_test_case remove_with_revdeps_in_trans_recursive

remove_with_revdeps_in_trans_recursive_head() {
	atf_set "descr" "Tests for package removal: remove a pkg with its revdeps in transaction (recursive)"
}

remove_with_revdeps_in_trans_recursive_body() {
	mkdir some_repo
	mkdir -p pkg_A/usr/bin pkg_B/usr/bin

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" --dependencies "A-1.0_1" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root --repository=some_repo -yvd B
	atf_check_equal $? 0
	dulge-remove -r root -Ryvd B
	atf_check_equal $? 0
	out=$(dulge-query -r root -l|wc -l)
	atf_check_equal $out 0
}

atf_test_case remove_directory

remove_directory_head() {
	atf_set "descr" "dulge-remove(1): remove nested directories"
}

remove_directory_body() {
	mkdir -p some_repo pkg_A/B/C
	touch pkg_A/B/C/file00
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C empty.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	dulge-remove -r root -C empty.conf -y A
	atf_check_equal $? 0
	test -d root/B
	atf_check_equal $? 1
}

atf_test_case keep_modified_files

keep_modified_files_head() {
	atf_set "descr" "Tests for package removal: keep modified files in rootdir"
}

keep_modified_files_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	echo "1234" >pkg_A/bin/ash
	echo "1234" >pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	echo "keep" >root/bin/sh
	echo "keep" >root/bin/ash

	dulge-remove -r root -yvd A
	atf_check_equal $? 0
	rv=1
	if [ -e root/bin/sh -a -e root/bin/ash ]; then
	        rv=0
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_modified_files

remove_modified_files_head() {
	atf_set "descr" "Tests for package removal: force remove modified files in rootdir"
}

remove_modified_files_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	echo "1234" >pkg_A/bin/ash
	echo "1234" >pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	echo "don't keep" >root/bin/sh
	echo "don't keep" > root/bin/ash

	dulge-remove -r root -yvdf A
	atf_check_equal $? 0
	rv=0
	[ -e root/bin/sh ] && rv=1
	[ -e root/bin/ash ] && rv=1
	atf_check_equal $rv 0
}

atf_test_case keep_modified_conf_files

keep_modified_conf_files_head() {
	atf_set "descr" "Tests for package removal: keep modified conf files in rootdir"
}

keep_modified_conf_files_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	echo "1234" >pkg_A/bin/ash
	echo "1234" >pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" --config-files "/bin/ash /bin/sh" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	echo "keep" >root/bin/sh
	echo "keep" >root/bin/ash

	dulge-remove -r root -yvd A
	atf_check_equal $? 0
	rv=1
	if [ -e root/bin/sh -a -e root/bin/ash ]; then
	        rv=0
	fi
	atf_check_equal $rv 0
}

atf_test_case remove_modified_conf_files

remove_modified_conf_files_head() {
	atf_set "descr" "Tests for package removal: force remove modified conf files in rootdir"
}

remove_modified_conf_files_body() {
	mkdir some_repo
	mkdir -p pkg_A/bin
	echo "1234" >pkg_A/bin/ash
	echo "1234" >pkg_A/bin/sh

	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" --config-files "/bin/ash /bin/sh" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C null.conf --repository=$PWD/some_repo -y A
	atf_check_equal $? 0
	echo "don't keep" >root/bin/sh
	echo "don't keep" > root/bin/ash

	dulge-remove -r root -yvdf A
	atf_check_equal $? 0
	rv=0
	[ -e root/bin/sh ] && rv=1
	[ -e root/bin/ash ] && rv=1
	atf_check_equal $rv 0
}

atf_init_test_cases() {
	atf_add_test_case keep_base_symlinks
	atf_add_test_case keep_modified_symlinks
	atf_add_test_case remove_readonly_files
	atf_add_test_case remove_symlinks
	atf_add_test_case remove_symlinks_abs
	atf_add_test_case remove_symlinks_dangling
	atf_add_test_case remove_symlinks_relative
	atf_add_test_case remove_symlinks_relative2
	atf_add_test_case remove_symlinks_from_root
	atf_add_test_case remove_symlinks_modified
	atf_add_test_case remove_dups
	atf_add_test_case remove_with_revdeps
	atf_add_test_case remove_with_revdeps_force
	atf_add_test_case remove_with_revdeps_in_trans
	atf_add_test_case remove_with_revdeps_in_trans_inverted
	atf_add_test_case remove_with_revdeps_in_trans_recursive
	atf_add_test_case remove_directory
	atf_add_test_case keep_modified_files
	atf_add_test_case remove_modified_files
	atf_add_test_case keep_modified_conf_files
	atf_add_test_case remove_modified_conf_files
}
