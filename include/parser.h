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
		inline bool exit_requested() { return exit_keyword_appeared; };		// For exit checking

		inline std::list<Command*> get_commands() { return commands; }	

	private:
		std::list<Command*> commands;
		char lookahead;
		FILE *input_stream;
		bool exit_keyword_appeared;
		
		// Some helpful routines
		void consume(char symbol);
		bool is_valid_symbol(char symbol);
		bool is_invalid_symbol(char symbol);
		void skip_whitespaces();

		// Grammar Rules/Targets
		bool command(bool is_in_pipeline);	// Returns false if an error occured, else true
		std::list<std::string> arguments();
		RedirectionPacket input_file();
		RedirectionPacket output_file();
		bool pipeline();	// Returns true if there is pipeline
		bool background();	// Returns true if a command must be executed in background

		char *identifier();

};

#endif // PARSER_H