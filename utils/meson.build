extra_program_src = []

foreach p: src
    extra_program_src += '../' + p
endforeach


server_data_gen = executable('server_data_gen', extra_program_src + 'server_data_gen/server_data_gen.cpp', dependencies: dep)

subdir('server_scanner')