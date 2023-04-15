#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"
#include <list>

class Command {
  private:
      char *name;
      char *input;
      char *output;
      bool in_pipeline;
      InputOutputType input_rt;
      InputOutputType output_rt;
      std::list <std::string> args;
  
  public:
    Command(std::string name);
    Command(std::string name, std::list<std::string> args);
    Command(std::string name, std::list<std::string> args, std::string input, InputOutputType input_rt,
            std::string output, InputOutputType output_rt, bool in_pipeline);
    ~Command();
    
    // void set_name(char *name);
    // void set_input(char *input);
    // void set_output(char *output);
    // inline void set_input_rt(InputOutputType i_rt) { input_rt = i_rt; }
    // inline void set_output_rt(InputOutputType o_rt) { output_rt = o_rt; }
    inline char* get_name() { return name; }
    inline char* get_input() { return input; }
    inline char* get_output() {return output; }
    inline InputOutputType get_input_rt() { return input_rt; }
    inline InputOutputType get_output_rt() { return output_rt; }
    inline std::list <std::string> get_args() { return args; }

    void print();
};

#endif // COMMAND_H