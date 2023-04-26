System Programming Course -- Assignment I
April 2023
Author : Yannis Zervakis

Abstract :
    In this project we are trying to build a shell, that gets commands, parses them and executes them. First of all we had to make a parser that parses simple commands with : arguments, I/O redirections, pipeline, background character. After the parser's building, for each command the goal was to call an exec* function for executing the command, connect pipes for pipelining, handle signals (^C, ^Z), etc.

Parser :
    The first part of the project was to make a parser which can recognize simple commands. In order to build that parser, we created the following grammar :

    Command <- <identifier> Args InputFile OutputFile Pipeline Background
    Args <- <identifier> Args | ε
    InputFile <- < <identifier> | ε
    Outputfile -> > <identifier> | >> <identifier> | ε
    Pipeline -> '|' Command | ε
    Background -> & | ε

    So, parser class is the type of Parser objects, who are recursive descent parsers following the grammar above. Parser scans the input stream character by character (lookahead), and stores a number of commands in a list that belongs to a pipeline (if there isn't pipeline, size of the list is equal to 1). For parsing command from input stream, we only have to call parse_command routine, which calls all the correspondings rules-routines of grammar to recognize the given command.

a) Commands with redirections :

b) Commands in pipeline :

c) Run commands in background :

d) Support wild characters :

e) Handle aliases :

f) Handle signals : 