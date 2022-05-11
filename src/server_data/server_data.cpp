//
// Created by joseph on 10/05/22.
//

#include "server_data.hpp"
#include <climits>
#include <cstdlib>
#include <unistd.h>

namespace flow::server
{

	inline uint64_t generate_uid() {
		srand(time(nullptr));
		return rand();
	}

    server_data_t::server_data_t()
	{
		uid = generate_uid();
		char* hostname = static_cast<char*>(malloc(sizeof(char) * HOST_NAME_MAX));
		gethostname(hostname, HOST_NAME_MAX);
		machine_name = hostname;
	}

    server_data_t::~server_data_t() 
    {
        free((void*)reinterpret_cast<const void*>(machine_name.data()));
    }

} // namespace flow::server
