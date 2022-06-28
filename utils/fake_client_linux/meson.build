extra_program_src = []

foreach p: src
    extra_program_src += '../../' + p
endforeach


fake_client_linux = executable('fake_client_linux', extra_program_src + 'fake_client_linux.cpp', dependencies: dep)