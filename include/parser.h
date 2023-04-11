#ifndef PARSER_H
#define PARSER_H

#include <list>
#include <string>

#include "common.h"
#include "command.h"

class Parser {
	public:
		Parser(FILE *input_stream);
		~Parser();
		Command* parse(char *input);

		inline void parse_command() { command(); }
		inline Command* get_cmd() { return cmd; }	

	private:
		Command *cmd;
		char lookahead;
		FILE *input_stream;

		void consume(char symbol);
		bool is_valid_symbol(char symbol);
		bool is_end_of_identifier(char symbol);

		void command();
		std::list<std::string> arguments();
		Redirection input_file();
		Redirection output_file();
		void pipeline();

		char *identifier();

};

#endif // PARSER_H