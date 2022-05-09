project('wm', 'cpp',
  version : '0.0.3',
  default_options : [
          'warning_level=3',
          'cpp_std=c++17'
  ]
)


if get_option('buildtype').startswith('debug')
   add_project_arguments('-DDEBUG', language: 'cpp')
endif

src = [
    'src/main.cpp',
    'src/server/server.cpp',
]

dep = [
    dependency('xcb'),
    dependency('xcb-ewmh'),
    dependency('xcb-icccm'),
    dependency('Poco', method: 'cmake', components: ['Net']),
    dependency('logger', fallback: ['logger', 'logger_dep']),
    dependency('libwm', fallback: ['libwm', 'libwm_dep']),
]

exe = executable(
    'flow_wm',
    src,
    dependencies: dep,
)
