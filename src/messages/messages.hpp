#ifndef WM_MESSAGES_HPP
#define WM_MESSAGES_HPP
#include "../uid/uid.hpp"
#include <string>

// EXAMPLE ?
enum DebugInfoRequest
{
	ServerVersion
};

#define MESSAGE_TYPES_REQ(H) \
	H(initial_connect, )     \
	H(debug_message, DebugInfoRequest requested_information;)

#define MESSAGE_TYPES_RES(H) \
	H(initial_connect, )     \
	H(debug_message, std::string contents;)

#define PLACE_MESSAGE_TYPE_AS_ENUM_REQ(name, ...) name##_request,
#define PLACE_MESSAGE_TYPE_AS_ENUM_RES(name, ...) name##_response,
#define CREATE_STRUCTURE_REQ(name, ...)                                          \
	struct message_##name##_request_t                                            \
	{                                                                            \
		message_##name##_request_t(const uid::uid_generator::uid uid) : uid(uid) \
		{}                                                                       \
                                                                                 \
		const message_type_request type = name##_request;                        \
		const uid::uid_generator::uid uid;                                       \
		__VA_ARGS__                                                              \
	};

#define CREATE_STRUCTURE_RES(name, ...)                                           \
	struct message_##name##_response_t                                            \
	{                                                                             \
		message_##name##_response_t(const uid::uid_generator::uid uid) : uid(uid) \
		{}                                                                        \
                                                                                  \
		const message_type_response type = name##_response;                       \
		const uid::uid_generator::uid uid;                                        \
		__VA_ARGS__                                                               \
	};

namespace flow::messages
{

	enum message_type_request
	{
		MESSAGE_TYPES_REQ(PLACE_MESSAGE_TYPE_AS_ENUM_REQ)
		_number_of_request_types
	};

	enum message_type_response
	{
		MESSAGE_TYPES_RES(PLACE_MESSAGE_TYPE_AS_ENUM_RES)
		_number_of_response_types
	};

	struct message_base_request_t
	{
		const message_type_request type;
		const uid::uid_generator::uid uid;
	};

	struct message_base_response_t
	{
		const message_type_response type;
		const uid::uid_generator::uid uid;
	};

	MESSAGE_TYPES_REQ(CREATE_STRUCTURE_REQ)
	MESSAGE_TYPES_RES(CREATE_STRUCTURE_RES)

} // namespace flow::messages

#endif
