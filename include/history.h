#ifndef HISTORY_H
#define HISTORY_H

#include "common.h"
#include "command.h"

class History {
    public:
        History(uint size);
        ~History();
        
        void add(std::list<Command*> new_command);
        std::list<Command *> get_at(uint cmd_index);
        void print_history();


    private:
        std::list<std::list <Command*>> history;
        uint size;
};

#endif // HISTORY_H