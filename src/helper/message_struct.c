

#include "./message_struct.h"

#include <stdlib.h>

void free_message_struct(MessageStruct msg) {
	if(msg.dynamic) {
		free(msg.message);
	}
}
