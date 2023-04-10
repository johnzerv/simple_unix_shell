#include <iostream>

#include "common.h"
#include "command.h"
#include "parser.h"

using namespace std;

int main(void) {

    Parser my_parser(stdin); 
    my_parser.parse_command();

    Command *cmd = my_parser.get_cmd();

    cmd->print();

    return 0;
}