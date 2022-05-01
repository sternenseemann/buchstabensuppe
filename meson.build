project(
  'buchstabensuppe', 'c',
  version : '0.0.0',
  license : 'BSD-3-Clause',
  default_options: ['c_std=c99'],
)

cc = meson.get_compiler('c')
pc = import('pkgconfig')

# TODO: version constraints
utf8proc = dependency('libutf8proc')
harfbuzz = dependency('harfbuzz')
# TODO: no pkg-config upstream, maybe ask for it?
schrift = cc.find_library('schrift')
math = cc.find_library('m')

incdir = include_directories('include')
lib = library(
  'buchstabensuppe',
  'bitmap.c',
  'buchstabensuppe.c',
  'flipdot.c',
  soversion : '0',
  include_directories : incdir,
  dependencies : [ utf8proc, harfbuzz, schrift, math ],
  install : true,
)
install_headers('include/buchstabensuppe.h')
install_headers(
  'include/buchstabensuppe/flipdot.h',
  'include/buchstabensuppe/bitmap.h',
  subdir : 'buchstabensuppe',
)
pc.generate(
  lib,
  description : 'A toy font rendering library for high contrast, low pixel count displays',
)

executable(
  'bs-renderflipdot',
  'bs-renderflipdot.c',
  link_with : lib,
  include_directories : incdir,
  install : true,
)

install_man('doc/man/bs-renderflipdot.1')

unittests = executable(
  'unittests',
  'test.c',
  include_directories : incdir,
  link_with : lib,
)
test('unit test suite', unittests)
