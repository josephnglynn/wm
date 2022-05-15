
go_comp = find_program('../../scripts/build_go.sh')

go_src = [ 'main.go', 'go.mod', 'go.sum' ]

server_scanner = custom_target(
    'server_scanner',
    build_by_default: true,
    output: 'server_scanner',
    input: go_src,
    command: [ go_comp],
    install: true,
    install_dir: get_option('bindir'),
)