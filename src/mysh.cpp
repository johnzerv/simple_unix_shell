#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "common.h"
#include "command.h"
#include "parser.h"

using namespace std;

int main(void) {

    while (true) {
        cout << "in-mysh-now:>";
        Parser *my_parser = new Parser(stdin); 
        my_parser->parse_command();

        if (my_parser->exit_requested()) {
            delete my_parser;
            break;
        }

        Command *cmd = my_parser->get_cmd();

        pid_t pid;
        if ((pid = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // int fd[2];       // for pipeline
        // if (pipe(fd) == -1) {
        //     perror("pipe");
        //     exit(EXIT_FAILURE);
        // }

        if (pid == 0) { // Child-Process
            // Prepare arguments for execvp()
            // Form : {<program_name>, arg1, arg2, ..., NULL}
            size_t no_args = cmd->get_args().size(); 
            char **args = new char*[no_args + 2];   // +2 for command name, and NULL

            args[0] = new char[strlen(cmd->get_name() + 1)];
            strcpy(args[0], cmd->get_name());

            size_t i;
            list<string>::iterator it;
            for (i = 1, it = cmd->get_args().begin(); i < no_args + 1 && it != cmd->get_args().end(); i++, it++) {
                args[i] = new char[(*it).length() + 1];
                strcpy(args[i], (*it).c_str());
            }
            args[no_args + 1] = nullptr;

            FILE *input_fp = nullptr, *output_fp = nullptr;
            int input_fd = -1, output_fd = -1;
            char *input_stream = cmd->get_input(), *output_stream = cmd->get_output();
            if (input_stream != nullptr) {
                input_fd = open(input_stream, O_RDONLY);
                close(0);
                dup2(input_fd, 0);
                close(input_fd);
            }

            if (output_stream != nullptr) {
                int flags = (cmd->get_output_rt() == IO) ? O_RDWR | O_CREAT | O_TRUNC
                                                      : O_RDWR | O_CREAT | O_APPEND;

                output_fd = open(output_stream, flags, 0666);
                close(1);
                dup2(output_fd, 1);
                close(output_fd);
            }

            execvp(args[0], (char * const *)args);

            for (i = 0; i < no_args + 1; i++) {
                delete args[i];
            }
            delete args;
        }

        // close(fd[WRITE]);
        
        waitpid(pid, nullptr, WUNTRACED);

        delete my_parser;
    }

    return 0;
}