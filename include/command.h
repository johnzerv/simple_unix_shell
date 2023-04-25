#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"

class Command {
  private:
      char *name;
      char *input_str;
      char *output_str;
      InputOutputType input_rt;   // Redirection type of input
      InputOutputType output_rt;  // Redirection type of output
      std::list <std::string> args;
      bool in_background;
  
  public:
    Command(std::string name, std::list<std::string> args, std::string input, InputOutputType input_rt,
            std::string output, InputOutputType output_rt, bool in_background);
    ~Command();
    
    // Getters
    inline char* get_name() { return name; }
    inline char* get_input() { return input_str; }
    inline char* get_output() {return output_str; }
    inline InputOutputType get_input_rt() { return input_rt; }
    inline InputOutputType get_output_rt() { return output_rt; }
    inline std::list <std::string> get_args() { return args; }
    inline bool is_in_background() { return in_background; }

    void print(std::ostream &stream);
};

#endif // COMMAND_H