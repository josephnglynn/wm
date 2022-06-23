#ifndef WM_UID_HPP
#define WM_UID_HPP

#include <cstdint>
namespace flow::uid
{

	class uid_generator
	{
	public:
		typedef uint32_t uid;

		inline uid get_next_uid() { return current_uid++; }

	private:
		uid current_uid = 1;
	};

} // namespace flow::uid

#endif
