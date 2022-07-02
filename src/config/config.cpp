#include "config.hpp"
#include <fstream>

namespace flow::config
{
	server_config::server_config()
	{
	}

	nullable::Nullable<server_config> server_config::get_local_config()
	{
		const auto home = std::string(std::getenv("HOME"));
		std::ifstream local_config(home + "/.config/flow_wm/local_server_config.json");

		if (local_config.bad()) return nullable::Nullable<server_config>::null();

		json j;
		local_config >> j;
		return {from_json(j)};
	}

	json server_config::to_json()
	{
		json j;

		j["server_location"] = {
			{"x", server_location.x},
			{"y", server_location.y},
			{"z", server_location.z},
		};

		j["known_hosts"] = known_hosts;

		return j;
	}

	server_config* server_config::from_json(json& j)
	{
		auto* sc = new server_config;
		sc->server_location = server_location_t(j["x"], j["y"], j["z"]);
		sc->known_hosts = ( std::vector<std::string> ) j["known_hosts"];
		return sc;
	}
} // namespace flow::config
