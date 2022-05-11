//
// Created by joseph on 10/05/22.
//

#ifndef WM_SERVER_DATA_HPP
#define WM_SERVER_DATA_HPP
#include <string>

namespace flow::server
{

	struct server_data_t
	{
		server_data_t();
        ~server_data_t();

		uint64_t uid;
		std::string_view machine_name;
	};
} // namespace flow::server

#endif //WM_SERVER_DATA_HPP
