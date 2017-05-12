#include "utils.h"
#include "mbed.h"

struct token_t {
	char * start;
	int length;
};

static bool findValue(char * key, char * content, token_t * token) {
	int length = strlen(key);
	char * start = strstr(content, key);
	
	if(start != NULL && length != 0) {
		short span = strspn(start + length, "\":");
		if (span == 0) return false;
		char * last = strpbrk(start + length + span, "\":,\r\n}");
		if (last <= (start + length + span) && last != NULL) return false;
		token->start = start + length + span;
		if (last != NULL) {
			token->length = last - token->start;
		} else {
			token->length = (content + strlen(content)) - token->start;
		}
		return true;
	}
	return false;
}

bool jgetBool(char * key, char * content, bool * value) {	
	token_t token;
	token.length = 0;
	token.start = NULL;
	
	if (!findValue(key, content, &token)) return false;
	
	if (token.length == 4) {
		if (strncmp(token.start, "true", token.length) == 0) {
			*value = true;
			return true;
		}
	}
	
	if (token.length == 5) {
		if (strncmp(token.start, "false", token.length) == 0) {
			*value = false;
			return true;
		}
	}
	
	return false;
}
