tests = {}

foreach test_name, sources : tests
  exe = executable(test_name, sources, dependencies: [libtap_dep])
  test(test_name, exe)
endforeach
