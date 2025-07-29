

#pragma once

typedef struct {
	char* message;
	bool dynamic;
} MessageStruct;

#define STATIC_MESSAGE_STRUCT(error) \
	((MessageStruct){ .message = (char*)(error), .dynamic = false })

#define DYNAMIC_MESSAGE_STRUCT(error) ((MessageStruct){ .message = (error), .dynamic = true })

#define EMPTY_MESSAGE_STRUCT() STATIC_MESSAGE_STRUCT(NULL)

void free_message_struct(MessageStruct msg);
