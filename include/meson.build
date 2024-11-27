project_header_files += files(
    'app_context.h',
    'board.h',
    'color.h',
    'editor_window.h',
    'gtkgoban.h',
    'http.h',
    'log.h',
    'main_window.h',
    'solve_preset_window.h',
    'solve_window.h',
    'stats.h',
    'stats_window.h',
    'task.h',
    'window.h',
)

vcs_dep = vcs_tag(
    input: 'git_version.h.in',
    output: 'git_version.h',
    replace_string: '@GIT_VERSION@',
)