//
// Created by joseph on 01/07/22.
//

#ifndef WM_NULLABLE_HPP
#define WM_NULLABLE_HPP
#include <logger/logger.hpp>

namespace nullable
{

	template <typename T>
	class Nullable
	{
	public:
		Nullable() = default;
		Nullable(T* t) : t(t) {}
		~Nullable() { delete t; }
		static Nullable null() { return Nullable(nullptr); }

		inline T& get_value()
		{
#ifdef DEBUG
			if (is_null()) logger::error("Calling null value lol!");
#endif

			return *t;
		}
		inline bool is_null()
		{
			return t == nullptr;
		}

	private:
		T* t;
	};

} // namespace nullable

#endif //WM_NULLABLE_HPP
