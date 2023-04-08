#include <iostream>

#include "common.h"
#include "command.h"

using namespace std;

int main(void) {

    list<string> args;
    args.push_back("file1");
    args.push_back("file2");
    args.push_back("file3");
    Command cmd("ls", args, "dir1", IO, "dir2", IO);


    cmd.print();

    return 0;
}