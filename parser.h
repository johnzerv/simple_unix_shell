#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "command.h"

class Parser {
	public:
		Parser(FILE *input_stream);
		Command* parse(char *input);


	private:
		int lookahead;


	
};



#endif // PARSER_H