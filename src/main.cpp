#include <wm/flow_wm.hpp>
#include <list>
#include <thread>
#include <chrono>
#include <cstring>

using namespace std::chrono_literals;

class custom_shell_t : public flow::shells::shell_t
{
public:
	custom_shell_t() = default;
	~custom_shell_t() override = default;

	void init(flow::window_manager_t* p_wm) override
	{
		this->wm = p_wm;
	}

	flow::shells::shell_info_t get_shell_info() override
	{
		return {
			"Example Shell",
			"Joseph Glynn",
			"v0.0.1",
			"An example shell to showcase some cool stuff"
		};
	}

	xcb_window_t create_back_window() override
	{
		xcb_window_t window = 0;
		base_threads.emplace_back([]
		{
		  try
		  {
			  std::this_thread::sleep_for(100ms);
			  std::system("/home/joseph/CLionProjects/flow-wm/build/src/shell/shell");
		  }
		  catch (std::exception& error)
		  {
			  std::cerr << "OH NO AN ERROR" << std::endl;
		  }
		});

		while (true)
		{
			xcb_generic_event_t* event = xcb_wait_for_event(wm->get_connection());

			if (event->response_type == XCB_MAP_REQUEST)
			{
				xcb_icccm_get_wm_class_reply_t hint;
				int result = xcb_icccm_get_wm_class_reply(wm->get_connection(),
					xcb_icccm_get_wm_class_unchecked(wm->get_connection(), window),
					&hint,
					nullptr
				);

				if (result && (strcmp(hint.instance_name, "shell") == 0 || strcmp(hint.class_name, "shell") == 0))
				{
					auto* mre = (xcb_map_request_event_t*)event;
					window = mre->window;
					xcb_map_window(wm->get_connection(), window);
					break;
				}
			}

			wm->get_event_handler(event);
		}

		return window;
	}

	flow::shells::frame_info_t create_frame_window(xcb_window_t client_window) override
	{
		xcb_window_t window = 0;
		std::cout << "CREATING FRAME!" << std::endl;
		frame_threads.emplace_back(std::thread([]
		{
		  try
		  {
			  std::this_thread::sleep_for(100ms);
			  std::system(
				  "/home/joseph/CLionProjects/flow-wm/build/src/window/window -n name -a desktop_name -e exec_path");
		  }
		  catch (std::exception& error)
		  {
			  std::cerr << "OH NO AN ERROR" << std::endl;
		  }
		}));

		while (true)
		{
			xcb_generic_event_t* event = xcb_wait_for_event(wm->get_connection());

			if (event->response_type == XCB_MAP_REQUEST)
			{
				xcb_icccm_get_wm_class_reply_t hint;
				int result = xcb_icccm_get_wm_class_reply(wm->get_connection(),
					xcb_icccm_get_wm_class_unchecked(wm->get_connection(), window),
					&hint,
					nullptr
				);

				if (result && (strcmp(hint.instance_name, "window") == 0 || strcmp(hint.class_name, "window") == 0))
				{
					auto* mre = (xcb_map_request_event_t*)event;
					window = mre->window;
					xcb_map_window(wm->get_connection(), window);
					break;
				}
			}

			wm->get_event_handler(event);
		}

		return { window, flow::shapes::rectangle_t(50, 0, 10, 0) };
	}

	void handle_back_window_event(xcb_window_t back_window, xcb_generic_event_t* event) override
	{
		wm->get_event_handler(event);
	}

	void handle_frame_window_event(xcb_window_t frame_window, xcb_generic_event_t* event) override
	{
		wm->get_event_handler(event);
	}

	void destroy_back_window(xcb_window_t back_window) override
	{
		xcb_ewmh_request_close_window(wm->get_ewmh_connection(),
			0,
			back_window,
			XCB_CURRENT_TIME,
			XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL
		);
	}

	void destroy_frame_window(xcb_window_t frame_window) override
	{
		xcb_ewmh_request_close_window(wm->get_ewmh_connection(),
			0,
			frame_window,
			XCB_CURRENT_TIME,
			XCB_EWMH_CLIENT_SOURCE_TYPE_NORMAL
		);
	}

private:
	flow::window_manager_t* wm = nullptr;
	std::list<std::thread> base_threads;
	std::list<std::thread> frame_threads;
	std::list<std::thread> shell_elements;
};

int main()
{
	logger::init();

#ifdef DEBUG
	auto* wm = new flow::window_manager_t(
		flow::configs::get_custom_config(
			std::string(std::getenv("HOME")) += "/CLionProjects/libwm/config/default_config.json"
		),
		new custom_shell_t()
	);
#else
	auto* wm = new flow::window_manager_t(flow::configs::get_default_config(),	new custom_shell_t());
#endif

	logger::notify("Starting wm");
	wm->run();
}