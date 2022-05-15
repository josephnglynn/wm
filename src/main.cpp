#include <X11/Xlib.h>
#include <cstdlib>
#define USE_HOOKS
/*
#define HOOK_LIST(H)                            \
	H(manage_client, clients::client_node_t*)   \
	H(unmanage_client, clients::client_node_t*) \
	H(wm_setup_complete, )                      \
	H(wm_running, )                             \
	H(wm_stopping, )
#define UNUSED_HOOK_LIST(H) \
	H(button_event, xcb_button_press_event_t*)
*/
#include "server/server.hpp"
#include <filesystem>
#include <logger/logger.hpp>
#include <thread>
#include <wm/flow_wm.hpp>

class custom_shell_t : public lib_wm::shells::Shell
{
public:
	custom_shell_t() = default;
	~custom_shell_t() override = default;

	void init(lib_wm::WindowManager* p_wm) override { this->wm = p_wm; }

	lib_wm::shells::ShellInfo get_shell_info() override
	{
		return {
			"Example Shell",
			"Joseph Glynn",
			"v0.0.1",
			"An example shell to showcase some cool stuff",
		};
	}

	xcb_window_t create_back_window() override { return 0; }
	lib_wm::shells::FrameInfo create_frame_window(xcb_window_t client_window) override { return {0, lib_wm::shapes::Rectangle::zero()}; }
	[[nodiscard]] const std::vector<std::string>& get_tag_names() const override { return tag_names; }

	void handle_back_window_event(xcb_window_t back_window, xcb_generic_event_t* event) override {}
	void handle_frame_window_event(xcb_window_t frame_window, xcb_generic_event_t* event) override {}

	void destroy_back_window(xcb_window_t back_window) override {}
	void destroy_frame_window(xcb_window_t frame_window) override {}

	[[nodiscard]] std::vector<lib_wm::keybindings::client_key_press_bind_t>& get_client_key_binds() override { return ckb; }
	[[nodiscard]] std::vector<lib_wm::keybindings::global_key_press_bind_t>& get_global_key_binds() override { return gkb; }
	[[nodiscard]] std::vector<lib_wm::keybindings::client_button_press_bind_t>& get_client_button_binds() override { return cbb; }
	[[nodiscard]] std::vector<lib_wm::keybindings::global_button_press_bind_t>& get_global_button_binds() override { return gbb; }

private:
	lib_wm::WindowManager* wm = nullptr;
	const std::vector<std::string> tag_names = {"TAG-1", "TAG-2", "TAG-3", "TAG-4", "TAG-5", "TAG-6", "TAG-7", "TAG-8", "TAG-9", "TAG-10"};

	std::vector<lib_wm::keybindings::client_key_press_bind_t> ckb = {

	};

	std::vector<lib_wm::keybindings::global_key_press_bind_t> gkb = {
		{lib_wm::ih::create_spawn_handler("rofi -show run"), 64, "r", 0},
		{lib_wm::ih::create_spawn_handler("alacritty"), 64, "t", 0},
		{lib_wm::ih::create_kill_client_handler(), 64, "q", 0},
		{lib_wm::ih::create_change_tag_handler(0), 64, "1", 0},
		{lib_wm::ih::create_change_tag_handler(1), 64, "2", 0},
		{lib_wm::ih::create_change_tag_handler(2), 64, "3", 0},
		{lib_wm::ih::create_change_tag_handler(3), 64, "4", 0},
		{lib_wm::ih::create_change_tag_handler(4), 64, "5", 0},
		{lib_wm::ih::create_change_tag_handler(5), 64, "6", 0},
		{lib_wm::ih::create_change_tag_handler(6), 64, "7", 0},
		{lib_wm::ih::create_change_tag_handler(7), 64, "8", 0},
		{lib_wm::ih::create_change_tag_handler(8), 64, "9", 0},
		{lib_wm::ih::create_change_tag_handler(9), 64, "0", 0},
		{lib_wm::ih::create_increment_tag_handler(), 64, "=", 0},
		{lib_wm::ih::create_increment_tag_handler(), 64, "-", 0},
		{lib_wm::ih::create_set_opacity_handler(0.8), 64, "o", 0},
		{lib_wm::ih::create_increase_opacity_handler(0.01), 64, ".", 0},
		{lib_wm::ih::create_decrease_opacity_handler(0.01), 64, ",", 0},
	};

	std::vector<lib_wm::keybindings::client_button_press_bind_t> cbb = {
		{lib_wm::ih::create_focus_client_handler(), XCB_MOD_MASK_ANY, 1},
		{lib_wm::ih::create_move_mouse_handler(), 64, 1},
		{lib_wm::ih::create_resize_mouse_handler(), 64, 3},
	};

	std::vector<lib_wm::keybindings::global_button_press_bind_t> gbb = {

	};
};

int main()
{
	logger::init();

	const auto home = std::string(std::getenv("HOME"));
	const auto wm_config_location = home + "/CLionProjects/libwm/config/default_config.json";
	const auto server_data_location = home + "/.config/flow_wm/server_config";
	std::filesystem::create_directories(home + "/config/flow_wm");

	auto* wm = new lib_wm::WindowManager(lib_wm::configs::get_custom_config(wm_config_location), new custom_shell_t());

	char* name = XDisplayName(nullptr);

	flow::server::flow_wm_server_t server(*wm, flow::server::get_server_data_from_file(server_data_location), 16812 + atoi(name));
	server.run();

	/*
	 * TO TEST SERVER
	 * using namespace std::chrono_literals;
	 * std::this_thread::sleep_for(1000min);
	 */

	wm->run();

	free(name);
}
