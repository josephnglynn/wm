//
// Created by joseph on 10/05/22.
//

#ifndef WM_SERVER_DATA_HPP
#define WM_SERVER_DATA_HPP
#include <cstdint>
#include <string>

namespace flow::server
{
#define ADD_OPERATOR_COPY(operand) \
    template <typename V> \
    inline constexpr vec3<T> operator operand(const vec3<V>& new_value) { \
       return vec3<T>(this->x operand new_value.x, this->y operand new_value.y, this->z operand new_value.z); \
    } \
    template<typename V> \
    inline constexpr vec3<T> operator operand(const V v) \
    { \
        return vec3<T>(this->x operand v, this->y operand v, this->z operand v); \
    }



#define ADD_OPERATOR(operand) \
    template <typename V> \
    inline constexpr vec3<T> operator operand(const vec3<V>& new_value) { \
    this->x = static_cast<T>(new_value.x); \
    this->y = static_cast<T>(new_value.y); \
    this->z = static_cast<T>(new_value.z); \
    return *this; \
    }


    template <typename T>
    struct vec3 
    {
       vec3(T x_component, T y_component, T z_component) : x(x_component), y(y_component), z(z_component)
       {
       }


        ADD_OPERATOR(=)

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

       
       inline constexpr vec3<T> operator++(int) 
       {
           vec3<T> tmp(*this);
           ++*this;
           return tmp;
       }

       inline constexpr vec3<T> operator--(int)
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


       T x;
       T y;
       T z;
    };

    using server_location_t = vec3<int32_t>;

	struct server_data_t
	{
		server_data_t();
        ~server_data_t();

		uint64_t uid;
		std::string_view machine_name;
        server_location_t location;
    };
} // namespace flow::server

#endif //WM_SERVER_DATA_HPP
