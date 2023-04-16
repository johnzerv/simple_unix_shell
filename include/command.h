#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"
#include <list>

class Command {
  private:
      char *name;
      char *input_str;
      char *output_str;
      InputOutputType input_rt;
      InputOutputType output_rt;
      std::list <std::string> args;
      bool in_background;
  
  public:
    Command(std::string name);
    Command(std::string name, std::list<std::string> args);
    Command(std::string name, std::list<std::string> args, std::string input, InputOutputType input_rt,
            std::string output, InputOutputType output_rt, bool in_background);
    ~Command();
    
    inline char* get_name() { return name; }
    inline char* get_input() { return input_str; }
    inline char* get_output() {return output_str; }
    inline InputOutputType get_input_rt() { return input_rt; }
    inline InputOutputType get_output_rt() { return output_rt; }
    inline std::list <std::string> get_args() { return args; }
    inline bool is_in_background() { return in_background; }

    void print();
};

#endif // COMMAND_H