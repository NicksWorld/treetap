tests = {
  # Placeholder test to ensure linkage
  'links': ['links.c'],
  # Tests to ensure functional io buffer interfaces
  'buffer io': ['buffer.c'],
}

foreach test_name, sources : tests
  exe = executable(test_name, sources, dependencies: [libtap_dep])
  test(test_name, exe)
endforeach
