wq = static_library(
    'wq',
    include_directories: include_directories('include'),
    sources: [
        'src/wq.cc',
    ],
)

wq_dep = declare_dependency(
    include_directories: include_directories('include'),
    link_with: wq,
)