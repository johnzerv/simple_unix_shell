#include <iostream>
#include <ostream>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <stdio.h>
#include <signal.h>

#include "common.h"
#include "command.h"
#include "parser.h"

using namespace std;

// Signal handler for ignoring signals
void ignore_signal(int signo) {
    struct sigaction act_signal;

    act_signal.sa_handler = SIG_IGN;

    sigaction(signo, &act_signal, NULL);
}

// Signal handler for setting signal behavior to default
void set_signal_to_default(int signo) {
    struct sigaction act_signal;

    act_signal.sa_handler = SIG_DFL;

    sigaction(signo, &act_signal, NULL);
}

int main(void) {
    ignore_signal(SIGTTIN); // for tcsetpgrp
    ignore_signal(SIGTTOU); // for tcsetpgrp
    // Only children processes get stopped or suspended
    ignore_signal(SIGINT);
    ignore_signal(SIGTSTP);

    // Create a group process for shell and connect it to the terminal
    pid_t shell_pid = getpid();
    setpgid(shell_pid, shell_pid);

    if (tcsetpgrp(STDIN_FILENO, shell_pid) != 0){
        perror("tcsetpgrp");
        exit(EXIT_FAILURE);
    }

    list<string> history;
    map<string, string> aliases;
    bool history_cmd, alias_cmd, cd_cmd;

    istream *current_stream = &cin;         // Streaming from which parser gets input
    stringstream *intermediate_stringstream = nullptr; // Auxiliary stream

    while (true) {
        
        if (current_stream == &cin) {
            cout << "in-mysh-now:>";
        }
        history_cmd = alias_cmd = cd_cmd = false;   // Initialize bools for history, alias and cd command

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
        stringstream hist_stringstream; // Auxiliary stream for saving command in history container
     
        for (cmd_it = commands.begin(), no_cmd = 0; cmd_it != commands.end(); cmd_it++, no_cmd++) {
            Command *cmd = *cmd_it;
            cmd->print(hist_stringstream);
            
            // Handle history commands
            if (!strcmp(cmd->get_name(), "hist")) {
                history_cmd = true;

                list<string> args = cmd->get_args();

                if (args.size() > 1) {
                    cout << "syntax error, hist command mustn't have over 1 argument" << endl;
                    break;
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
                        
                        intermediate_stringstream = new stringstream;
                        *intermediate_stringstream << *hist_it;
                        current_stream = intermediate_stringstream;
                    }
                    else {
                        cout << "hist: Wrong argument" << endl;
                    }
                }

                break;
            }

            // Handle aliase's commands
            if (!strcmp(cmd->get_name(), "createalias")) {
                alias_cmd = true;

                if (cmd->get_args().size() != 2) {
                    cout << "syntax error, createalias must have two arguments" << endl;
                    break;
                }

                aliases.erase(cmd->get_args().front());
                aliases.insert({cmd->get_args().front(), cmd->get_args().back()});
            } 
            else if (!strcmp(cmd->get_name(), "destroyalias")) {
                alias_cmd = true;

                if (cmd->get_args().size() != 1) {
                    cout << "syntax error, destroy alias must have one argument" << endl;
                    break;
                }
                aliases.erase(cmd->get_args().front());

            } else {
                map<string, string>::iterator alias_it = aliases.find(cmd->get_name());

                if (alias_it != aliases.end()) {
                    intermediate_stringstream = new stringstream;
                    *intermediate_stringstream << alias_it->second;
                    current_stream = intermediate_stringstream;
                    alias_cmd = true;
                }
                else {
                    if (intermediate_stringstream != nullptr) {
                        delete intermediate_stringstream;
                        intermediate_stringstream = nullptr;
                    }
                    current_stream = &cin;
                    alias_cmd = false;
                }
            }
            if (alias_cmd) {
                break;
            }

            // Handle special command 'cd'
            if (!strcmp(cmd->get_name(), "cd")) {
                assert(cmd->get_args().size() <= 1);

                string target_dir = (cmd->get_args().size() == 0) ? getenv("HOME") : cmd->get_args().front();
                if (chdir((const char *)target_dir.c_str()) == -1) {
                    perror("cd");
                }

                cd_cmd = true;
                break;
            }

            //  Create a new pipe
            if (no_cmd + 1 != commands.size()) {    // If isn't the last command in a pipeline
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
                // Children processes mustn't ignore signals SIGINT and SIGTSTP
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
            }

            // Parent's code -- Two things TODO :
            // 1) Close pipes and update file descriptors
            // 2) Work on group id's of children

            // Close old pipe's ends from parent process
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
            
            // If this is the first command in a pipeline (or the only one)
            if (no_cmd == 0) {
                childern_gpid = pid;

                // Create a group for all children with group id equal to the pid of first child
                if (setpgid(childern_gpid, childern_gpid) < 0) {
                    perror("setpgid");
                    exit(EXIT_FAILURE);
                }

                // And if the command is foreground, connect the terminal (stdin)
                if (!is_bg_cmd) {
                    if (tcsetpgrp(STDIN_FILENO, childern_gpid) != 0) {
                        perror("tcsetpgrp");
                        exit(EXIT_FAILURE);
                    }
                }
            }
            else {
                // Add child to group process with first child's pid
                if (setpgid(pid, childern_gpid) < 0) {
                    perror("setpgid");
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        // If command wasn't about history, change directory and aliases
        if (!history_cmd && !alias_cmd && !cd_cmd) {
            if (is_bg_cmd == false) {   // Only for foreground commands
                int status;

                // Wait for all children forked
                for (unsigned long i = 0; i < commands.size(); i++ ) {
                    if (waitpid(-childern_gpid, &status, WUNTRACED) == -1) {
                        perror("waitpid");
                        exit(EXIT_FAILURE);
                    }
                }

                // Connect back the terminal to shell group process (stdin)
                if (tcsetpgrp(STDIN_FILENO, shell_pid) != 0) {
                    perror("waitpid");
                    exit(EXIT_FAILURE);
                }   
            }
            
            // Reading again from stdin
            if (intermediate_stringstream != nullptr) {
                delete intermediate_stringstream;
                intermediate_stringstream = nullptr;
            }
            current_stream = &cin;
            if (!history_cmd) {
                history.push_back(hist_stringstream.str());
            }
        }

        delete my_parser;
    }

    if (intermediate_stringstream != nullptr) {
        delete intermediate_stringstream;
    }

    return 0;
}