
go_comp = find_program('go')

go_src = [ 'main.go' ]

server_scanner = custom_target(
    'server_scanner',
    build_by_default: true,
    output: 'server_scanner',
    input: go_src,
    command: [ go_comp, 'build', '-o', '@OUTPUT@', join_paths(meson.current_source_dir(), 'main.go')],
    install: true,
    install_dir: get_option('bindir'),
)