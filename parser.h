#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "command.h"

class Parser {
	public:
		Command* parse(char *input);
	
};



#endif // PARSER_H