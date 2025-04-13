tests = {
  # Placeholder test to ensure linkage
  'links': ['links.c'],
}

foreach test_name, sources : tests
  exe = executable(test_name, sources, dependencies: [libtap_dep])
  test(test_name, exe)
endforeach
