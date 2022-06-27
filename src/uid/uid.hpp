#ifndef WM_UID_HPP
#define WM_UID_HPP

#include <cstdint>
namespace flow::uid
{

	class uid_generator
	{
	public:
		typedef int32_t uid_t;

		inline uid_t get_next_uid() { return current_uid++; }

	private:
		uid_t current_uid = 1;
	};

} // namespace flow::uid

#endif
