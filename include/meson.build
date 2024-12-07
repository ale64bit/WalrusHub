project_header_files += files(
    'app_context.h',
    'board.h',
    'color.h',
    'editor_window.h',
    'game_window.h',
    'gtk_eval_bar.h',
    'gtkgoban.h',
    'http.h',
    'katago_client.h',
    'log.h',
    'main_window.h',
    'play_ai_preset_window.h',
    'play_ai_window.h',
    'settings_window.h',
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