#! /usr/bin/env atf-sh

atf_test_case remove_directory

remoe_directory_head() {
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

atf_test_case remove_orphans

remove_orphans_head() {
	atf_set "descr" "dulge-remove(1): remove orphaned packages"
}

remove_orphans_body() {
	mkdir -p some_repo pkg_A/B/C
	touch pkg_A/
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	dulge-install -r root -C empty.conf --repository=$PWD/some_repo -yA A
	atf_check_equal $? 0
	dulge-remove -r root -C empty.conf -yvdo
	atf_check_equal $? 0
	dulge-query -r root A
	atf_check_equal $? 2
}

atf_test_case clean_cache

clean_cache_head() {
	atf_set "descr" "dulge-remove(1): clean cache"
}

clean_cache_body() {
	mkdir -p repo pkg_A/B/C pkg_B
	touch pkg_A/
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n A-1.0_2 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	mkdir -p root/etc/dulge.d root/var/db/dulge/https___localhost_ root/var/cache/dulge
	cp repo/*-repodata root/var/db/dulge/https___localhost_
	atf_check_equal $? 0
	cp repo/*.dulge root/var/cache/dulge
	atf_check_equal $? 0
	echo "repository=https://localhost/" >root/etc/dulge.d/localrepo.conf
	dulge-install -r root -C etc/dulge.d -R repo -dvy B
	dulge-remove -r root -C etc/dulge.d -dvO
	atf_check_equal $? 0
	test -f root/var/cache/dulge/A-1.0_2.noarch.dulge
	atf_check_equal $? 0
	test -f root/var/cache/dulge/A-1.0_1.noarch.dulge
	atf_check_equal $? 1
	test -f root/var/cache/dulge/B-1.0_1.noarch.dulge
	atf_check_equal $? 0
}

atf_test_case clean_cache_dry_run

clean_cache_dry_run_head() {
	atf_set "descr" "dulge-remove(1): clean cache dry run"
}

clean_cache_dry_run_body() {
	mkdir -p repo pkg_A/B/C
	touch pkg_A/
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n A-1.0_2 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	mkdir -p root/etc/dulge.d root/var/db/dulge/https___localhost_ root/var/cache/dulge
	cp repo/*-repodata root/var/db/dulge/https___localhost_
	atf_check_equal $? 0
	cp repo/*.dulge root/var/cache/dulge
	atf_check_equal $? 0
	echo "repository=https://localhost/" >root/etc/dulge.d/localrepo.conf
	ls -lsa root/var/cache/dulge
	out="$(dulge-remove -r root -C etc/dulge.d -dvnO)"
	atf_check_equal $? 0
	atf_check_equal "$out" "Removed A-1.0_1.noarch.dulge from cachedir (obsolete)"
}

atf_test_case clean_cache_dry_run_perm

clean_cache_dry_run_perm_head() {
	atf_set "descr" "dulge-remove(1): clean cache dry run without read permissions"
}

clean_cache_dry_run_perm_body() {
	# this should print an error instead of dry deleting the files it can't read
	mkdir -p repo pkg_A/B/C
	touch pkg_A/
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n A-1.0_2 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	mkdir -p root/etc/dulge.d root/var/db/dulge/https___localhost_ root/var/cache/dulge
	cp repo/*-repodata root/var/db/dulge/https___localhost_
	atf_check_equal $? 0
	cp repo/*.dulge root/var/cache/dulge
	atf_check_equal $? 0
	chmod 0000 root/var/cache/dulge/*.dulge
	echo "repository=https://localhost/" >root/etc/dulge.d/localrepo.conf
	out="$(dulge-remove -r root -C etc/dulge.d -dvnO)"
	atf_check_equal $? 0
	atf_check_equal "$out" "Removed A-1.0_1.noarch.dulge from cachedir (obsolete)"
}

clean_cache_uninstalled_head() {
	atf_set "descr" "dulge-remove(1): clean uninstalled package from cache"
}

clean_cache_uninstalled_body() {
	mkdir -p repo pkg_A/B/C pkg_B
	touch pkg_A/
	cd repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n A-1.0_2 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-create -A noarch -n B-1.0_1 -s "B pkg" ../pkg_B
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..
	mkdir -p root/etc/dulge.d root/var/db/dulge/https___localhost_ root/var/cache/dulge
	cp repo/*-repodata root/var/db/dulge/https___localhost_
	atf_check_equal $? 0
	cp repo/*.dulge root/var/cache/dulge
	atf_check_equal $? 0
	echo "repository=https://localhost/" >root/etc/dulge.d/localrepo.conf
	dulge-install -r root -C etc/dulge.d -R repo -dvy B
	atf_check_equal $? 0
	dulge-remove -r root -C etc/dulge.d -dvOO
	atf_check_equal $? 0
	test -f root/var/cache/dulge/A-1.0_2.noarch.dulge
	atf_check_equal $? 1
	test -f root/var/cache/dulge/A-1.0_1.noarch.dulge
	atf_check_equal $? 1
	test -f root/var/cache/dulge/B-1.0_1.noarch.dulge
	atf_check_equal $? 0
}

atf_test_case remove_msg

remove_msg_head() {
	atf_set "descr" "dulge-rmeove(1): show remove message"
}

remove_msg_body() {
	mkdir -p some_repo pkg_A

	cat <<-EOF >pkg_A/REMOVE.msg
	foobar-remove-msg
	EOF
	cd some_repo
	dulge-create -A noarch -n A-1.0_1 -s "A pkg" ../pkg_A
	atf_check_equal $? 0
	dulge-rindex -d -a $PWD/*.dulge
	atf_check_equal $? 0
	cd ..

	dulge-install -r root -C empty.conf -R some_repo -dvy A
	atf_check_equal $? 0

	atf_check -s exit:0 \
		-o 'match:foobar-remove-msg' \
		-e ignore \
		-- dulge-remove -r root -C empty.conf -y A
}

atf_init_test_cases() {
	atf_add_test_case remove_directory
	atf_add_test_case remove_orphans
	atf_add_test_case clean_cache
	atf_add_test_case clean_cache_dry_run
	atf_add_test_case clean_cache_dry_run_perm
	atf_add_test_case clean_cache_uninstalled
	atf_add_test_case remove_msg
}
