libtap_sources = [
  'src/libtap.c',
]
libtap_headers = [
  'include/libtap.h',
]
libtap_manpages = [
  'man/tap.5',
]

libtap_include = include_directories('include')
libtap = shared_library(
  'tap',
  libtap_sources,
  include_directories: libtap_include,
  gnu_symbol_visibility: 'hidden',
  install: true,
)
libtap_dep = declare_dependency(
  link_with: libtap,
  include_directories: libtap_include,
)

install_headers(libtap_headers, subdir : 'libtap')
install_man(libtap_manpages)

if get_option('tests')
  subdir('tests')
endif
