project('wm', 'cpp',
  version : '0.0.3',
  default_options : [
          'warning_level=3',
          'cpp_std=c++17'
  ]
)

src = [
    'src/main.cpp'
]

dep = [
    dependency('libwm', fallback: ['libwm', 'libwm_dep']),
]

exe = executable(
    'flow_wm',
    src,
    dependencies: dep,
)