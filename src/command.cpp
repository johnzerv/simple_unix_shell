#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cassert>

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

  this->input = this->output = nullptr;
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
  this->input = this->output = nullptr;
}

Command::Command(std::string name, std::list<std::string> args, std::string input, RedirectionType input_rt, std::string output, RedirectionType output_rt) {
  assert(!name.empty());
  this->name = new char(name.length() + 1);
  strcpy(this->name, name.c_str());

  if (!input.empty()) {
    this->input = new char(input.length() + 1);
    strcpy(this->input, input.c_str());
    this->input_rt = input_rt;
  }
  else {
    this->input = nullptr;
    this->input_rt = NO_REDIRECTION;
  }

  if (!output.empty()) {
    this->output = new char(output.length() + 1);
    strcpy(this->output, output.c_str());
    this->output_rt = output_rt;
  }
  else {
    this->output = nullptr;
    this->output_rt = NO_REDIRECTION;
  }

  this->args = args;
}

Command::~Command() {
  delete name;

  if (input != nullptr) {
    delete input;
  }
  if (output != nullptr) {
    delete output;
  }
}

// void Command::set_name(char *name) {
//   this->name = new char(strlen(name) + 1);
//   strcpy(this->name, name);
// }

// void Command::set_input(char *input) {
//   this->input = new char(strlen(input) + 1);
//   strcpy(this->input, input);
// }

// void Command::set_output(char *output) {
//   this->output = new char(strlen(output) + 1);
//   strcpy(this->output, output);
// }

void Command::print() {
  std::cout << "------- Printing a Command -------" << std::endl;
  std::cout << "Program Name : " << name << std::endl;

  std::cout << "Arguments :";
  if (!args.empty()) {
    for (std::list<std::string>::iterator it = args.begin(); it != args.end(); it++) {
      std::cout << " " << *it;
    }
  }
  else {
    std::cout << "No Arguments";
  }

  std::cout << std::endl << "Input : ";
  if (input != nullptr) {
    std::cout << input;
  }
  else {
    std::cout << "stdin";
  }

  std::cout << "\nOutput : ";
  if (output != nullptr) {
    std::cout << ((output_rt == IO) ? "> " : ">> ") << output;
  }
  else {
    std::cout << "stdout";
  }

  std::cout << std::endl;
}