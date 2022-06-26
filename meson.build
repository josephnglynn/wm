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
    'src/server/server.cpp',
    'src/handlers/handlers.cpp',
    'src/buffer/buffer.cpp',
    'src/uid/uid.cpp',
    'src/config/config.cpp'
]

dep = [
    dependency('xcb'),
    dependency('xcb-ewmh'),
    dependency('xcb-icccm'),
    dependency('LibDataChannel'),
    dependency('logger', fallback: ['logger', 'logger_dep']),
    dependency('libwm', fallback: ['libwm', 'libwm_dep']),
    dependency('arg_parser', fallback: ['arg_parser', 'arg_parser_dep']),
    dependency('Poco', method: 'cmake', components: ['Net']),
    dependency('threads'),
]

exe = executable(
    'flow_wm',
    src  + 'src/main.cpp',
    dependencies: dep,
)

