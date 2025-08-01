

#pragma once

typedef struct {
	char* message_internal;
	bool dynamic;
} MessageStruct;

#define STATIC_MESSAGE_STRUCT(error) \
	((MessageStruct){ .message_internal = (char*)(error), .dynamic = false })

#define DYNAMIC_MESSAGE_STRUCT(error) \
	((MessageStruct){ .message_internal = (error), .dynamic = true })

#define EMPTY_MESSAGE_STRUCT() STATIC_MESSAGE_STRUCT(NULL)

[[nodiscard]] bool is_empty_message_struct(MessageStruct message);

[[nodiscard]] char* get_message(MessageStruct message);

void free_message_struct(MessageStruct msg);
