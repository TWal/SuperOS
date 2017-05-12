#ifndef REDIRECTION_H
#define REDIRECTION_H

#include <string>

class Redirection {
    public:
        Redirection(const std::string& file);
        virtual void doRedirection() = 0;
    protected:
        std::string _file;
};

class Input : public Redirection {
    public:
        Input(const std::string& file);
        virtual void doRedirection();
};

class Output : public Redirection {
    public:
        Output(const std::string& file);
        virtual void doRedirection();
};

class OutputAppend : public Redirection {
    public:
        OutputAppend(const std::string& file);
        virtual void doRedirection();
};

#endif

