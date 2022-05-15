//
// Created by joseph on 10/05/22.
//

#include "server_data.hpp"
#include "../buffer/buffer.hpp"
#include "../deserialize/deserialize.hpp"
#include <cstdlib>
#include <fstream>

namespace flow::server
{

	void server_data_t::write_to(const std::string& file_name)
	{
		buffers::server_buffer_t buffer(sizeof(*this));
		buffer.write(*this);

		std::ofstream file(file_name, std::ofstream::trunc);
		file.write(buffer.get_data(), buffer.get_location());
		file.close();
	}

	server_data_t get_server_data_from_file(const std::string& file_name)
	{
		std::ifstream file(file_name, std::ios::binary | std::ios::ate);
		if (!file.good()) return {};
		auto size = file.tellg();
		file.seekg(0, std::ios::beg);

		static buffers::server_buffer_t buffer(size); // NEEDS TO BE KEPT ALIVE
		file.read(buffer.get_data(), size);

		return serialization::deserialize<server_data_t>(buffer);
	}

} // namespace flow::server
