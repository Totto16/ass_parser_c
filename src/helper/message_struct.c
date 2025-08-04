

#include "./message_struct.h"
#include "./macros.h"

#include <stdlib.h>

[[nodiscard]] bool is_empty_message_struct(MessageStruct message) {
	return message.message_internal == NULL;
}

[[nodiscard]] char* get_message(MessageStruct message) {
	ASSERT(message.message_internal != NULL, "empty message");
	return message.message_internal;
}

void free_message_struct(MessageStruct msg) {
	if(msg.dynamic) {
		free(msg.message_internal);
	}
}
