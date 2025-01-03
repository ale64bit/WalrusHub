project(
  'WalrusHub',
  'c',
  'cpp',
  version: '0.1',
  default_options: ['warning_level=3', 'cpp_std=c++17'],
)

subproject('curl')
subproject('sqlite3')
subproject('nlohmann_json')

library_path = []
project_sources = []
project_header_files = []
project_test_sources = []
project_benchmark_sources = []
include_dirs = [include_directories('include')]

subdir('include')
subdir('lib')
subdir('src')

exe = executable(
  'WalrusHub',
  vcs_dep,
  sources: main_source,
  include_directories: include_dirs,
  install: true,
  cpp_args: '-D_USE_MATH_DEFINES',
  dependencies: [
    dependency('gtk4'),
    dependency('libcurl'),
    dependency('nlohmann_json'),
    dependency('sqlite3'),
    http_dep,
    log_dep,
    wq_dep,
  ],
)

test('basic', exe)