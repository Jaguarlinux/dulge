#! /usr/bin/env atf-sh
# Test that dulge-uhelper arch works as expected.

atf_test_case native

native_head() {
	atf_set "descr" "dulge-uhelper arch: native test"
}

native_body() {
	unset DULGE_ARCH DULGE_TARGET_ARCH
	atf_check -o "inline:$(uname -m)\n" -- dulge-uhelper -r "$PWD" arch
}

atf_test_case env

env_head() {
	atf_set "descr" "dulge-uhelper arch: envvar override test"
}
env_body() {
	export DULGE_ARCH=foo
	atf_check_equal $(dulge-uhelper -r $PWD arch) foo
}

atf_test_case conf

conf_head() {
	atf_set "descr" "dulge-uhelper arch: configuration override test"
}
conf_body() {
	mkdir -p dulge.d root
	unset DULGE_ARCH DULGE_TARGET_ARCH
	echo "architecture=foo" > dulge.d/arch.conf
	atf_check -o inline:"foo\n" -- dulge-uhelper -C $PWD/dulge.d -r root arch
}

atf_init_test_cases() {
	atf_add_test_case native
	atf_add_test_case env
	atf_add_test_case conf
}
