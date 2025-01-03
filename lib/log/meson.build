log = static_library(
    'log',
    include_directories: include_directories('include'),
    sources: [
        'src/log.cc',
    ],
)

log_dep = declare_dependency(
    include_directories: include_directories('include'),
    link_with: log,
)