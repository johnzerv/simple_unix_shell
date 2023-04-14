#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

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
            char *input_stream = cmd->get_input(), *output_stream = cmd->get_output();
            if (input_stream != nullptr) {
                input_fp = freopen(input_stream, "r", stdin);
            }
            
            if (output_stream != nullptr) {
                if (cmd->get_output_rt() == IO) {
                    output_fp = freopen(output_stream, "w", stdout);
                }
                else {
                    output_fp = freopen(output_stream, "a", stdout);
                }
            }

            execvp(args[0], (char * const *)args);

            for (i = 0; i < no_args + 1; i++) {
                delete args[i];
            }
            delete args;

            if (input_fp != nullptr) {
                fclose(input_fp);
            }
            if (output_fp != nullptr) {
                fclose(output_fp);
            }
        }

        // close(fd[WRITE]);
        
        waitpid(pid, nullptr, WUNTRACED);

        delete my_parser;
    }

    return 0;
}