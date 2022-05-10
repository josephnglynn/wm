//
// Created by joseph on 10/05/22.
//

#include "server_data.hpp"
#include <climits>
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

} // namespace flow::server