#ifndef WM_MESSAGES_HPP
#define WM_MESSAGES_HPP

#define MESSAGE_TYPES_REQ(H) \
	H(successful_connection, )

#define MESSAGE_TYPES_RES(H) \
	H(successful_connection, )

#define PLACE_MESSAGE_TYPE_AS_ENUM_REQ(name, ...) name##_request,
#define PLACE_MESSAGE_TYPE_AS_ENUM_RES(name, ...) name##_response,
#define CREATE_STRUCTURE_REQ(name, ...)                   \
	struct message_##name##_request_t                     \
	{                                                     \
		const message_type_request type = name##_request; \
		__VA_ARGS__                                       \
	};

#define CREATE_STRUCTURE_RES(name, ...)                     \
	struct message_##name##_response_t                      \
	{                                                       \
		const message_type_response type = name##_response; \
		__VA_ARGS__                                         \
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
	};

	struct message_base_response_t

	{
		const message_type_response type;
	};

	MESSAGE_TYPES_REQ(CREATE_STRUCTURE_REQ)
	MESSAGE_TYPES_RES(CREATE_STRUCTURE_RES)

} // namespace flow::messages

#endif
