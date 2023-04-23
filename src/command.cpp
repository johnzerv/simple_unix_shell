#include <iostream>
#include <cstdlib>
#include <cstring>

#include "command.h"

Command::Command(std::string name) {
  this->name = new char(name.length() + 1);
  if(this->name != nullptr) {
    strcpy(this->name, name.c_str());
  }
  else {
    // ERROR
    this->name = NULL;
  }

  this->input_str = this->output_str = nullptr;
}

Command::Command(std::string name, std::list<std::string> args) {
  this->name = new char(name.length() + 1);
  if(this->name != nullptr) {
    strcpy(this->name, name.c_str());
  }
  else {
    // ERROR
    this->name = NULL;
  }

  this->args = args;
  this->input_str = this->output_str = nullptr;
}

Command::Command(std::string name, std::list<std::string> args, std::string input, InputOutputType input_rt,
                 std::string output, InputOutputType output_rt, bool in_background) {
  assert(!name.empty());
  this->name = new char(name.length() + 1);
  strcpy(this->name, name.c_str());

  if (!input.empty()) {
    this->input_str = new char(input.length() + 1);
    strcpy(this->input_str, input.c_str());
    this->input_rt = input_rt;
  }
  else if (input_rt == PIPELINE) {
    this->input_str = nullptr;
    this->input_rt = PIPELINE;
  }
  else {
    this->input_str = nullptr;
    this->input_rt = NO_REDIRECTION;
  }

  if (!output.empty()) {
    this->output_str = new char(output.length() + 1);
    strcpy(this->output_str, output.c_str());
    this->output_rt = output_rt;
  }
  else if (output_rt == PIPELINE) {
    this->output_str = nullptr;
    this->output_rt = PIPELINE;
  }
  else {
    this->output_str = nullptr;
    this->output_rt = NO_REDIRECTION;
  }

  this->args = args;
  this->in_background = in_background;
}

Command::~Command() {
  if (name != nullptr) {
    delete name;
  }

  if (input_str != nullptr) {
    delete input_str;
  }
  if (output_str != nullptr) {
    delete output_str;
  }
}

// void Command::set_name(char *name) {
//   this->name = new char(strlen(name) + 1);
//   strcpy(this->name, name);
// }

// void Command::set_input(char *input) {
//   this->input_str = new char(strlen(input) + 1);
//   strcpy(this->input, input);
// }

// void Command::set_output(char *output) {
//   this->output_str = new char(strlen(output) + 1);
//   strcpy(this->output, output);
// }

void Command::print(std::ostream &stream) {
  stream << name;

  if (!args.empty()) {
    for (std::list<std::string>::iterator it = args.begin(); it != args.end(); it++) {
      stream << " " << *it;
    }
  }

  if (input_str != nullptr) {
    stream << " < " << input_str;
  }

  std::string in_bg_str = (in_background) ? "&" : "";

  if (output_str != nullptr) {
    stream << ((output_rt == IO) ? " > " : " >> ") << output_str;

    stream << in_bg_str << std::endl;
  }
  else if (output_rt == PIPELINE) {
    stream << " | ";
  }
  else {
    stream << in_bg_str << std::endl;
  }
}