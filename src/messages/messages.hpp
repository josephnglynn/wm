#ifndef WM_MESSAGES_HPP
#define WM_MESSAGES_HPP
#include "../config/config.hpp"
#include "../uid/uid.hpp"
#include <string>

/*
 * MESSAGES BEING SENT TO HOST SERVER SHOULD HAVE 'host'
 * MESSAGES BEING SENT TO ANOTHER GUEST SERVER SHOULD HAVE 'guest'
 */
#define HOST_UID = 0
#define GUEST_UID
#define MESSAGE_TYPES_REQ(H)          \
	H(host, HOST_UID, connect_test, ) \
	H(host, HOST_UID, initial_connect, config::server_config* server_config;)

#define MESSAGE_TYPES_RES(H)                    \
	H(host, HOST_UID, connect_test, bool good;) \
	H(host, HOST_UID, initial_connect, config::server_config* server_config;)

#define PLACE_MESSAGE_TYPE_AS_ENUM_REQ(recipient, uid, name, ...) recipient##_##name##_request,
#define PLACE_MESSAGE_TYPE_AS_ENUM_RES(recipient, uid, name, ...) recipient##_##name##_response,
#define CREATE_STRUCTURE_REQ(recipient, uid_bit, name, ...)                                              \
	struct message_##recipient##_##name##_request_t                                                      \
	{                                                                                                    \
		message_##recipient##_##name##_request_t(const uid::uid_generator::uid_t uid uid_bit) : uid(uid) \
		{}                                                                                               \
                                                                                                         \
		const message_type type = recipient##_##name##_request;                                          \
		const uid::uid_generator::uid_t uid;                                                             \
		__VA_ARGS__                                                                                      \
	};

#define CREATE_STRUCTURE_RES(recipient, uid_bit, name, ...)                                               \
	struct message_##recipient##_##name##_response_t                                                      \
	{                                                                                                     \
		message_##recipient##_##name##_response_t(const uid::uid_generator::uid_t uid uid_bit) : uid(uid) \
		{}                                                                                                \
                                                                                                          \
		const message_type type = recipient##_##name##_response;                                          \
		const uid::uid_generator::uid_t uid;                                                              \
		__VA_ARGS__                                                                                       \
	};

namespace flow::messages
{

	enum message_type
	{
		MESSAGE_TYPES_REQ(PLACE_MESSAGE_TYPE_AS_ENUM_REQ)
		MESSAGE_TYPES_RES(PLACE_MESSAGE_TYPE_AS_ENUM_RES)
			_number_of_message_types
	};

	struct message_base_request_t
	{
		const message_type type;
		const uid::uid_generator::uid_t uid;
	};

	struct message_base_response_t
	{
		const message_type type;
		const uid::uid_generator::uid_t uid;
	};

	MESSAGE_TYPES_REQ(CREATE_STRUCTURE_REQ)
	MESSAGE_TYPES_RES(CREATE_STRUCTURE_RES)

} // namespace flow::messages

#endif
