#include "Redirection.h"
//#include <fcntl.h>
#include <unistd.h>

static const int STDIN_FD  = 0;
static const int STDOUT_FD = 1;

Redirection::Redirection(const std::string& file) :
    _file(file) {}

Input::Input(const std::string& file) :
    Redirection(file) {}


#if 0
        List.iter (function
            | Input file -> begin
                let fd = Unix.openfile file [Unix.O_RDONLY] 0o640 in
                Unix.dup2 fd Unix.stdin
            end
            | Output file -> begin
                let fd = Unix.openfile file [Unix.O_WRONLY; Unix.O_CREAT] 0o640 in
                Unix.dup2 fd Unix.stdout
            end
            | Output_append file -> begin
                let fd = Unix.openfile file [Unix.O_WRONLY; Unix.O_APPEND; Unix.O_CREAT] 0o640 in
                Unix.dup2 fd Unix.stdout
            end
        ) scmd.cmd_redirects;
#endif

void Input::doRedirection() {
    int fd = open(_file.c_str(), O_RDONLY);
    dup2(fd, STDIN_FD);
    close(fd);
}

Output::Output(const std::string& file) :
    Redirection(file) {}

void Output::doRedirection() {
    int fd = open(_file.c_str(), O_WRONLY | O_CREAT);
    dup2(fd, STDOUT_FD);
    close(fd);
}

OutputAppend::OutputAppend(const std::string& file) :
    Redirection(file) {}

void OutputAppend::doRedirection() {
    int fd = open(_file.c_str(), O_WRONLY | O_APPEND);
    dup2(fd, STDOUT_FD);
    close(fd);
}

