#ifndef COMMAND_H
#define COMMAND_H

#include "common.h"
#include <list>

class Command {
  private:
      char *name;
      char *input;
      char *output;
      RedirectionType input_rt;
      RedirectionType output_rt;
      std::list <std::string> args;
  
  public:
    Command(std::string name);
    Command(std::string name, std::list<std::string> args);
    Command(std::string name, std::list<std::string> args, std::string input, RedirectionType input_rt, std::string output, RedirectionType output_rt);
    ~Command();
    
    inline char* get_name() { return name; }
    inline char* get_input() { return input; }
    inline char* get_output() {return output; }
    inline RedirectionType get_input_rt() { return input_rt; }
    inline RedirectionType get_output_rt() { return output_rt; }

    void print();
};

#endif // COMMAND_H