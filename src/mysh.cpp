#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <ostream>

#include "common.h"
#include "command.h"
#include "parser.h"

using namespace std;

void ignore_signal(int signo) {
    struct sigaction act_signal;

    act_signal.sa_handler = SIG_IGN;

    sigaction(signo, &act_signal, NULL);
}

void set_signal_to_default(int signo) {
    struct sigaction act_signal;

    act_signal.sa_handler = SIG_DFL;

    sigaction(signo, &act_signal, NULL);
}

int main(void) {

    ignore_signal(SIGTTIN);
    ignore_signal(SIGTTOU);
    ignore_signal(SIGINT);
    ignore_signal(SIGTSTP);

    pid_t shell_pid = getpid();

    setpgid(shell_pid, shell_pid);

    if (tcsetpgrp(STDIN_FILENO, shell_pid) != 0){
        perror("tcsetpgrp");
        exit(EXIT_FAILURE);
    }

    list<string> history;
    bool history_cmd, alias_cmd;

    map<string, string> aliases;

    istream *current_stream = &cin;
    stringstream intermediate_stringstream;
    // uint hist_index = 0;

    while (true) {

        if (current_stream == &cin) {
            cout << "in-mysh-now:>";
        }
        history_cmd = alias_cmd = false;

        Parser *my_parser = new Parser(current_stream); 

        if (!my_parser->parse_command()) {
            perror("syntax error");
            exit(EXIT_FAILURE);
        }


        if (my_parser->exit_requested()) {
            delete my_parser;
            break;
        }

        list<Command *> commands = my_parser->get_commands();
        list<Command *>::iterator cmd_it;
        pid_t pid, childern_gpid;
        uint no_cmd;
        int new_fd[2], old_fd[2];   // fd's that hold multiple pipes
        bool is_bg_cmd = commands.back()->is_in_background();
        stringstream hist_stringstream;
     
        for (cmd_it = commands.begin(), no_cmd = 0; cmd_it != commands.end(); cmd_it++, no_cmd++) {
            Command *cmd = *cmd_it;
            cmd->print(hist_stringstream);
            
            // Handle history commands
            if (!strcmp(cmd->get_name(), "hist")) {
                history_cmd = true;

                list<string> args = cmd->get_args();

                if (args.size() > 1) {
                    perror("syntax error, hist command mustn't have over 1 argument");
                    exit(EXIT_FAILURE);
                }
                else if (args.size() == 0) {
                    cout << "--------- History ---------" << endl;;

                    int i = 0;
                    for (list<string>::iterator it = history.begin(); it != history.end() && i < 20; it++, i++) {
                        cout << i << ". " << *it;
                    }
                    current_stream = &cin;
                }
                else {
                    list<string>::iterator hist_it;
                    int i = 0, requested = stoi(*(args.begin()));

                    if (requested >= 0 && requested <= 20) {
                        for (hist_it = history.begin(); hist_it != history.end() && i < requested; hist_it++, i++);
                        
                        intermediate_stringstream << *hist_it;
                        current_stream = &intermediate_stringstream;
                    }
                    else {
                        cout << "hist: Wrong argument" << endl;
                    }
                }

                break;
            }

            // Handle aliase's commands
            if (!strcmp(cmd->get_name(), "createalias")) {
                assert(cmd->get_args().size() == 2);

                aliases.erase(cmd->get_args().front());
                aliases.insert({cmd->get_args().front(), cmd->get_args().back()});

                alias_cmd = true;
            } 
            else if (!strcmp(cmd->get_name(), "destroyalias")) {
                assert(cmd->get_args().size() == 1);
                aliases.erase(cmd->get_args().front());

                alias_cmd = true;
            } else {
                map<string, string>::iterator alias_it = aliases.find(cmd->get_name());


                if (alias_it != aliases.end()) {
                    intermediate_stringstream << alias_it->second;
                    current_stream = &intermediate_stringstream;
                    alias_cmd = true;
                }

                else {
                    current_stream = &cin;
                    alias_cmd = false;
                }
            }
            if (alias_cmd) {
                break;
            }
    
            if (no_cmd + 1 != commands.size()) {    // If isn't the last command in a pipeline, make a new pipe
                if (pipe(new_fd) < 0) {
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }
            }

            if ((pid = fork()) == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            if (pid == 0) {
                set_signal_to_default(SIGINT);
                set_signal_to_default(SIGTSTP);

                // Prepare arguments for execvp()
                // Form : {<program_name>, arg1, arg2, ..., NULL}
                size_t no_args = cmd->get_args().size();
                list<string> arguments = cmd->get_args();
                char **args = new char*[no_args + 2];   // +2 for command name and NULL string

                args[0] = new char[strlen(cmd->get_name() + 1)];
                strcpy(args[0], cmd->get_name());

                size_t i;
                list<string>::iterator it;
                for (i = 1, it = arguments.begin(); i < no_args + 1 && it != arguments.end(); i++, it++) {
                    args[i] = new char[(*it).length() + 1];
                    strcpy(args[i], (*it).c_str());
                }
                args[no_args + 1] = nullptr;

                // Fix input and output file descriptors
                int input_fd = 0, output_fd = 1;    // 0 for stdin, 1 for stdout (default)
                if (cmd->get_input() != nullptr) {
                    input_fd = open(cmd->get_input(), O_RDONLY);
                }
                else if (cmd->get_input_rt() == PIPELINE) {
                    input_fd = old_fd[READ];
                }

                if (cmd->get_output() != nullptr) {
                    int flags = (cmd->get_output_rt() == IO) ? O_RDWR | O_CREAT | O_TRUNC
                                                             : O_RDWR | O_CREAT | O_APPEND;
                    output_fd = open(cmd->get_output(), flags, 0666);
                }
                else if (cmd->get_output_rt() == PIPELINE) {
                    output_fd = new_fd[WRITE];
                }

                // Connect input stream if isn't stdin (Pipe Input or Redirection)
                if (input_fd != 0) {
                    if (cmd->get_input_rt() == PIPELINE) {
                        if (close(old_fd[WRITE]) < 0) {
                            perror("close oldfd[write]");
                            exit(EXIT_FAILURE);
                        }
                    }
                    
                    if (close(0) < 0) {
                        perror("close stdin");
                        exit(EXIT_FAILURE);
                    }

                    if (dup2(input_fd, 0) < 0) {
                        perror("dup2 inp");
                        exit(EXIT_FAILURE);
                    }

                    if (close(input_fd) < 0) {
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                }

                // Connect output stream if isn't stdout (Pipe Output or Redirection)
                if (output_fd != 1) {
                    if (cmd->get_output_rt() == PIPELINE) {
                        if (close(new_fd[READ]) < 0) {
                            perror("close newfd[read]");
                            exit(EXIT_FAILURE);
                        }
                    }

                    if (close(1) < 0) {
                        perror("close");
                        exit(EXIT_FAILURE);
                    }

                    if (dup2(output_fd, 1) < 0) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }

                    if (close(output_fd) < 0) {
                        perror("close");
                        exit(EXIT_FAILURE);
                    }
                }

                if (execvp(args[0], (char * const *)args) < 0) {
                    perror("execvp");
                    exit(EXIT_FAILURE);
                }

                for (size_t i = 0; i < no_args + 1; i++) {
                    delete args[i];
                }
                delete args;
            }

            if (no_cmd == 0) {
                childern_gpid = pid;

                if (setpgid(childern_gpid, childern_gpid) < 0) {
                    perror("setpgid");
                    exit(EXIT_FAILURE);
                }

                if (!is_bg_cmd) {
                    if (tcsetpgrp(STDIN_FILENO, childern_gpid) != 0) {
                        perror("tcsetpgrp");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else {
                if (setpgid(pid, childern_gpid) < 0) {
                    perror("setpgid");
                    exit(EXIT_FAILURE);
                }
            }

            // We have to close old pipe's ends after the connection
            if ((no_cmd % 2 == 1 || no_cmd + 1 == commands.size()) && commands.size() > 1) {    
                if (close(old_fd[WRITE]) < 0) {
                    perror("closewrite-parent");
                    exit(EXIT_FAILURE);
                }
                if (close(old_fd[READ]) < 0) {
                    perror("closeread-parent");
                    exit(EXIT_FAILURE);
                }
            }
            
            // Update old_fd for next loop
            if (no_cmd + 1 != commands.size()) {
                old_fd[0] = new_fd[0];
                old_fd[1] = new_fd[1];
            }
        }

        if (!history_cmd && !alias_cmd) {
            if (is_bg_cmd == false) {
                int status;

                for (unsigned long i = 0; i < commands.size(); i++ ) {
                    if (waitpid(-childern_gpid, &status, WUNTRACED) == -1) {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    }
                }
                if (tcsetpgrp(STDIN_FILENO, shell_pid) != 0) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }   
            }

            current_stream = &cin;
            if (!history_cmd) {
                history.push_front(hist_stringstream.str());
            }
        }

        delete my_parser;
    }

    return 0;
}