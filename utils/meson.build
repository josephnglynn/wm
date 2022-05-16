extra_program_src = []

foreach p: src
    extra_program_src += '../' + p
endforeach


server_data_gen = executable('server_data_gen', extra_program_src + 'server_data_gen/server_data_gen.cpp', dependencies: dep)
fake_client_linux = executable('fake_client_linux', extra_program_src + 'fake_client_linux/fake_client_linux.cpp', dependencies: dep)
subdir('server_scanner')