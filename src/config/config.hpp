#ifndef WM_CONFIG_HPP
#define WM_CONFIG_HPP
#include <iostream>
#include "../nullable/nullable.hpp"
#include <nlohmann/json.hpp>


namespace flow::config
{

	using json = nlohmann::json;

#define ADD_OPERATOR_COPY(operand)                                                                             \
	template <typename V>                                                                                      \
	inline constexpr vec3<T> operator operand(const vec3<V>& new_value)                                        \
	{                                                                                                          \
		return vec3<T>(this->x operand new_value.x, this->y operand new_value.y, this->z operand new_value.z); \
	}                                                                                                          \
	template <typename V>                                                                                      \
	inline constexpr vec3<T> operator operand(const V v)                                                       \
	{                                                                                                          \
		return vec3<T>(this->x operand v, this->y operand v, this->z operand v);                               \
	}

#define ADD_OPERATOR(operand)                                           \
	template <typename V>                                               \
	inline constexpr vec3<T> operator operand(const vec3<V>& new_value) \
	{                                                                   \
		this->x = static_cast<T>(new_value.x);                          \
		this->y = static_cast<T>(new_value.y);                          \
		this->z = static_cast<T>(new_value.z);                          \
		return *this;                                                   \
	}

	template <typename T>
	struct vec3
	{
		vec3() = default;

		explicit vec3(T v)
			: x(static_cast<T>(v)), y(static_cast<T>(v)), z(static_cast<T>(v))
		{
		}

		template <typename V>
		explicit vec3(V v)
			: x(static_cast<T>(v)), y(static_cast<T>(v)), z(static_cast<T>(v))
		{
		}

		vec3(T x_component, T y_component, T z_component)
			: x(static_cast<T>(x_component)), y(static_cast<T>(y_component)), z(static_cast<T>(z_component))
		{
		}

		template <typename V>
		vec3(V x_component, V y_component, V z_component)
			: x(static_cast<T>(x_component)), y(static_cast<T>(y_component)), z(static_cast<T>(z_component))
		{
		}

		template <typename V>
		inline constexpr vec3<T>& operator=(const vec3<V>& new_value)
		{
			this->x = static_cast<T>(new_value.x);
			this->y = static_cast<T>(new_value.y);
			this->z = static_cast<T>(new_value.z);
			return *this;
		}

		inline constexpr vec3<T> operator++()
		{
			++this->x;
			++this->y;
			++this->z;
			return *this;
		}

		inline constexpr vec3<T> operator--()
		{
			--this->x;
			--this->y;
			--this->z;
			return *this;
		}

		inline constexpr const vec3<T> operator++(int)
		{
			vec3<T> tmp(*this);
			++*this;
			return tmp;
		}

		inline constexpr const vec3<T> operator--(int)
		{
			vec3<T> tmp(*this);
			--*this;
			return tmp;
		}

		ADD_OPERATOR(*=)
		ADD_OPERATOR(/=)
		ADD_OPERATOR(+=)
		ADD_OPERATOR(-=)
		ADD_OPERATOR(<<=)
		ADD_OPERATOR(>>=)
		ADD_OPERATOR(%=)
		ADD_OPERATOR(&=)
		ADD_OPERATOR(|=)
		ADD_OPERATOR(^=)

		ADD_OPERATOR_COPY(*)
		ADD_OPERATOR_COPY(/)
		ADD_OPERATOR_COPY(+)
		ADD_OPERATOR_COPY(-)
		ADD_OPERATOR_COPY(<<)
		ADD_OPERATOR_COPY(>>)
		ADD_OPERATOR_COPY(%)
		ADD_OPERATOR_COPY(&)
		ADD_OPERATOR_COPY(|)
		ADD_OPERATOR_COPY(^)

		friend std::ostream& operator<<(std::ostream& os, const vec3<T> vec)
		{
			os << "vec3 - x=" << vec.x << " - y=" << vec.y << " - z=" << vec.z;
			return os;
		}

		friend std::istream& operator>>(std::istream& in, vec3<T>& vec)
		{
			char comma;
			in >> vec.x;
			in >> comma;
			in >> vec.y;
			in >> comma;
			in >> vec.z;
			return in;
		}

		T x;
		T y;
		T z;
	};

	using server_location_t = vec3<int32_t>;

	struct server_config
	{
		server_config();

		json to_json();
		static server_config* from_json(json& j);
		static nullable::Nullable<server_config> get_local_config();


		server_location_t server_location;
		std::vector<std::string> known_hosts;
	};
} // namespace flow::config

#endif
