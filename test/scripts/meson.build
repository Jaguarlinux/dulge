tests = [
  'parseopts_test.sh',
  'dulge-build-template_test.sh',
]

foreach tst : tests
  test(tst,
       BASH,
       protocol : 'tap',
       env : TEST_ENV,
       args : [
         join_paths(meson.current_source_dir(), tst),
       ])
endforeach
