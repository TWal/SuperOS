#ifndef COMMANDS_H
#define COMMANDS_H

#include <vector>
#include <string>
#include "Redirection.h"

class Command {
    public:
        virtual ~Command();
        virtual int run() = 0;
};

class Atomic : public Command {
    public:
        Atomic(const std::vector<std::string>& cmdline, const std::vector<Redirection*>& redirections, bool _background);
        virtual int run();
        virtual ~Atomic();
    protected:
        std::vector<std::string> _cmdline;
        std::vector<Redirection*> _redirections;
        bool _background;
};

class If : public Command {
    public:
        If(Command* cond, Command* ifcommand, Command* elsecommand);
        virtual int run();
        virtual ~If();
    protected:
        Command* _cond;
        Command* _ifcommand;
        Command* _elsecommand;
};

class Pipe : public Command {
    public:
        Pipe(Command* producer, Command* consumer);
        virtual int run();
        virtual ~Pipe();
    protected:
        Command* _producer;
        Command* _consumer;
};

class And : public Command {
    public:
        And(Command* command1, Command* command2);
        virtual int run();
        virtual ~And();
    protected:
        Command* _command1;
        Command* _command2;
};

class Or : public Command {
    public:
        Or(Command* command1, Command* command2);
        virtual int run();
        virtual ~Or();
    protected:
        Command* _command1;
        Command* _command2;
};

class Seq : public Command {
    public:
        Seq(Command* command1, Command* command2);
        virtual int run();
        virtual ~Seq();
    protected:
        Command* _command1;
        Command* _command2;
};

#endif

