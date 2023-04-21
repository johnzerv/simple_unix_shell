#include <iostream>

#include "history.h"

History::History(uint size) {
    this->size = size;
}

History::~History() {
    std::list<std::list <Command*>>::iterator it1;
    std::list<Command *>::iterator it2;

    for (it1 = history.begin(); it1 != history.end(); it1++) {
        for (it2 = (*it1).begin(); it2 != (*it1).end(); it2++) {
            delete(*it2);
        }

        (*it1).clear();
    }
}

void History::add(std::list<Command*> cmd_given) {
    // Deep copy of given command before pushing it onto list
    std::list<Command*> new_cmd;

    for (std::list<Command *>::iterator it = cmd_given.begin(); it != cmd_given.end(); it++) {
        std::string name = (*it)->get_name();
        std::list<std::string> args = (*it)->get_args();
        std::string input = (*it)->get_input();
        InputOutputType input_rt = (*it)->get_input_rt();
        std::string output = (*it)->get_output();
        InputOutputType output_rt = (*it)->get_output_rt();
        bool in_background = (*it)->is_in_background();

        new_cmd.push_front(new Command(name, args, input, input_rt, output, output_rt, in_background));
    } 

    history.push_front(new_cmd);
}

std::list<Command *> History::get_at(uint cmd_index) {
    assert(cmd_index < size);

    std::list<std::list <Command*>>::iterator it;

    for (int i = 0; i < cmd_index; i++) {
        it++;
    }

    return *it;
}

void History::print_history() {
    std::list<std::list <Command*>>::iterator it1;

    for (it1 = history.begin(); it1 != history.end(); it1++) {
        std::list<Command *>::iterator it2;
        int i = 0;

        for (it2 = (*it1).begin(); it2 != (*it1).end(); it2++, i++) {
            if (i > 0) {    // Not sure, maybe Command:print() should take care of it
                std::cout << " | ";
            }

            (*it2)->print();    // TODO: change the print function of Command class
        }
    }
}