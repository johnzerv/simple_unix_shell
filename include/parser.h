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

		inline bool parse_command() { return command(false); }
		inline std::list<Command*> get_commands() { return commands; }	
		inline bool exit_requested() { return exit_keyword_appeared; };

	private:
		std::list<Command*> commands;
		char lookahead;
		FILE *input_stream;
		bool exit_keyword_appeared;

		void consume(char symbol);
		bool is_valid_symbol(char symbol);
		bool is_invalid_symbol(char symbol);


		bool command(bool is_in_pipeline);	// Returns false if an error occured, else true
		std::list<std::string> arguments();
		Redirection input_file();
		Redirection output_file();
		bool pipeline();	// Returns true if there is pipeline

		char *identifier();

		void skip_whitespaces();
};

#endif // PARSER_H