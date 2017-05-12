#include "Commands.h"
#include <unistd.h>
#include <sys/wait.h>

static const int STDIN_FD  = 0;
static const int STDOUT_FD = 1;

Atomic::Atomic(const std::vector<std::string>& cmdline, const std::vector<Redirection*>& redirections, bool _background) :
    _cmdline(cmdline), _redirections(redirections), _background(_background) {}

int Atomic::run() {
    if(_cmdline.empty()) {
        return 0;
    //} else if(_cmdline.front() == "cd") {
    } else {
        int oldstdin = dup(STDIN_FD);
        int oldstdout = dup(STDOUT_FD);
        for(Redirection* r : _redirections) {
            r->doRedirection();
        }

        pid_t pid = fork();
        if(pid == 0) {
            std::vector<char*> argv;
            argv.reserve(_cmdline.size()+1);
            bool hasSlash = false;
            for(size_t i = 0; i < _cmdline[0].size(); ++i) {
                hasSlash |= (_cmdline[0][i] == '/');
            }
            std::string commandPath = hasSlash ? _cmdline[0] : ("/usr/bin/" + _cmdline[0]);
            for(size_t i = 0; i < _cmdline.size(); ++i) {
                argv.push_back(const_cast<char*>(_cmdline[i].data()));
            }
            argv.push_back(nullptr);
            if(execv(commandPath.data(), reinterpret_cast<char * const *>(argv.data()))) {
                perror("execv");
                return -1;
            }
            return 42; //make gcc happy
        } else {
            int res;
            wait(&res);
            dup2(oldstdin, STDIN_FD);
            dup2(oldstdout, STDOUT_FD);
            close(oldstdin);
            close(oldstdout);
            return res;
        }
    }
}

If::If(Command* cond, Command* ifcommand, Command* elsecommand) :
    _cond(cond), _ifcommand(ifcommand), _elsecommand(elsecommand) {}

int If::run() {
    int condres = _cond->run();
    if(condres == 0) {
        return _ifcommand->run();
    } else {
        return _elsecommand->run();
    }
}

Pipe::Pipe(Command* producer, Command* consumer) :
    _producer(producer), _consumer(consumer) {}

int Pipe::run() {
    int pipes[2];
    if(pipe(pipes) != 0) {
        perror("pipe");
        return -1;
    }
    int pipeout = pipes[0];
    int pipein = pipes[1];

    pid_t pid = fork();
    if(pid == 0) {
        //producer
        dup2(pipein, STDOUT_FD);
        close(pipeout);
        close(pipein);
        _producer->run();
        exit(0);
    } else {
        //consumer
        int oldstdin = dup(STDIN_FD);
        dup2(pipeout, STDIN_FD);
        close(pipein);
        close(pipeout);
        int res = _consumer->run();
        wait(nullptr);
        dup2(oldstdin, STDIN_FD);
        close(oldstdin);
        return res;
    }
}

And::And(Command* command1, Command* command2) :
    _command1(command1), _command2(command2) {}

int And::run() {
    int res1 = _command1->run();
    if(res1 == 0) {
        return _command2->run();
    } else {
        return res1;
    }
}

Or::Or(Command* command1, Command* command2) :
    _command1(command1), _command2(command2) {}

int Or::run() {
    int res1 = _command1->run();
    if(res1 != 0) {
        return _command2->run();
    } else {
        return res1;
    }
}

Seq::Seq(Command* command1, Command* command2) :
    _command1(command1), _command2(command2) {}

int Seq::run() {
    _command1->run();
    return _command2->run();
}

