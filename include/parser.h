#ifndef PARSER_H
#define PARSER_H

#include "common.h"
#include "command.h"

// Class that describes Parser objects. Every Parser object is a recursive descent parser based on following grammar :

// Command <- <identifier> Args InputFile OutputFile Pipeline Background
// Args <- <identifier> Args | ε
// InputFile <- < <identifier> | ε
// Outputfile -> > <identifier> | >> <identifier> | ε
// Pipeline -> '|' Command | ε
// Background -> & | ε

class Parser {
	public:
		Parser(std::istream *input_stream);
		~Parser();

		inline bool parse_command() { return command(false); }
		inline bool exit_requested() { return exit_keyword_appeared; };		// For exit checking
		
		// Getter
		inline std::list<Command*> get_commands() { return commands; }	

	private:
		char lookahead;
		std::istream *input_stream;
		std::list<Command*> commands;	// List of commands in pipeline (size == 1 if there isn't pipeline)
		bool exit_keyword_appeared;
		
		// Some helpful routines
		void consume(char symbol);
		bool is_valid_symbol(char symbol);
		bool is_invalid_symbol(char symbol);
		void skip_whitespaces();
		bool is_exit_keyword_given();

		// Grammar Rules/Targets
		bool command(bool is_in_pipeline);	// Returns false if an error occured, else true
		std::list<std::string> arguments();
		IOPacket input_file();
		IOPacket output_file();
		bool pipeline();	// Returns true if there is pipeline
		bool background();	// Returns true if a command must be executed in background

		char *identifier();

};

#endif // PARSER_H