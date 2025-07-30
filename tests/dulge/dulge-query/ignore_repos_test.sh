#! /usr/bin/env atf-sh
# Test that dulge-query(1) -i works as expected

atf_test_case ignore_system

ignore_system_head() {
	atf_set "descr" "dulge-query(1) -i: ignore repos defined in the system directory (sharedir/dulge.d)"
}

ignore_system_body() {
	mkdir -p repo pkg_A/bin
	touch pkg_A/bin/file
	ln -s repo repo1
	cd repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	rm -f *.dulge
	cd ..
	systemdir=$(dulge-uhelper getsystemdir)
	mkdir -p root/${systemdir}
	echo "repository=$PWD/repo1" > root/${systemdir}/myrepo.conf
	out="$(dulge-query -C empty.conf --repository=$PWD/repo -i -L|wc -l)"
	atf_check_equal "$out" 1
}

atf_test_case ignore_conf

ignore_conf_head() {
	atf_set "descr" "dulge-query(1) -i: ignore repos defined in the configuration directory (dulge.d)"
}

ignore_conf_body() {
	mkdir -p repo pkg_A/bin
	touch pkg_A/bin/file
	ln -s repo repo1
	cd repo
	dulge-create -A noarch -n foo-1.0_1 -s "foo pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	rm -f *.dulge
	cd ..
	mkdir -p root/dulge.d
	echo "repository=$PWD/repo1" > root/dulge.d/myrepo.conf
	out="$(dulge-query -r root -C dulge.d --repository=$PWD/repo -i -L|wc -l)"
	atf_check_equal "$out" 1
}

atf_init_test_cases() {
	atf_add_test_case ignore_conf
	atf_add_test_case ignore_system
}
