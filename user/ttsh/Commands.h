#ifndef COMMANDS_H
#define COMMANDS_H

#include <vector>
#include <string>
#include "Redirection.h"

class Command {
    public:
        virtual int run() = 0;
};

class Atomic : public Command {
    public:
        Atomic(const std::vector<std::string>& cmdline, const std::vector<Redirection*>& redirections, bool _background);
        virtual int run();
    protected:
        std::vector<std::string> _cmdline;
        std::vector<Redirection*> _redirections;
        bool _background;
};

class If : public Command {
    public:
        If(Command* cond, Command* ifcommand, Command* elsecommand);
        virtual int run();
    protected:
        Command* _cond;
        Command* _ifcommand;
        Command* _elsecommand;
};

class Pipe : public Command {
    public:
        Pipe(Command* producer, Command* consumer);
        virtual int run();
    protected:
        Command* _producer;
        Command* _consumer;
};

class And : public Command {
    public:
        And(Command* command1, Command* command2);
        virtual int run();
    protected:
        Command* _command1;
        Command* _command2;
};

class Or : public Command {
    public:
        Or(Command* command1, Command* command2);
        virtual int run();
    protected:
        Command* _command1;
        Command* _command2;
};

class Seq : public Command {
    public:
        Seq(Command* command1, Command* command2);
        virtual int run();
    protected:
        Command* _command1;
        Command* _command2;
};

#endif

