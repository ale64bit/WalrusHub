http = static_library(
    'http',
    include_directories: include_directories('include'),
    sources: [
        'src/http.cc',
    ],
    dependencies: [
        log_dep,
        dependency('glib-2.0'),
        dependency('libcurl'),
    ],
)

http_dep = declare_dependency(
    include_directories: include_directories('include'),
    link_with: http,
)