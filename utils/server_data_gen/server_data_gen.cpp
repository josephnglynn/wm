//
// Created by joseph on 11/05/22.
//
#include "../src/server/server.hpp"
#include <arg_parser/parser.hpp>
#include <string>

int main(int argc, char* argv[])
{
	const auto server_data_location = std::string(std::getenv("HOME")) += "/.config/server_config_t";
	flow::server::server_data_t data = flow::server::get_server_data_from_file(server_data_location);

	std::string machine_name;
	arg_parser::parser("server_data_gen")
		.with_about("Simple program to set the wm server data")
		.with_author("Joseph Glynn")
		.with_exec("server_data_gen")
		.with_version("v0.0.1")
		.with_arg(arg_parser::argument<uint64_t>("uids")
					  .with_description("Server uid")
					  .with_long("--unique_identifier")
					  .with_short("-uid")
					  .with_value(data.uid))
		.with_arg(arg_parser::argument<std::string>("Machine name")
					  .with_description("Identifier which other servers see this machine as")
					  .with_short("-mn")
					  .with_long("--mac_name")
					  .with_value(machine_name))
		.with_arg(arg_parser::argument<flow::server::server_location_t>("Monitor location (x,y,z)")
					  .with_description("Location of monitor in virtual space")
					  .with_short("-ml")
					  .with_long("--mon_loc")
					  .with_value(data.location))
		.parse(argc, argv);

	data.machine_name = machine_name;
	data.write_to(server_data_location);
}