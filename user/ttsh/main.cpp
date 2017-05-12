#include "Commands.h"

int main() {
    std::vector<Redirection*> redirects;
    std::vector<std::string> cmdline1;
    cmdline1.push_back("cat");
    cmdline1.push_back("main.cpp");
    Atomic cmd1(cmdline1, redirects, false);

    std::vector<std::string> cmdline2;
    cmdline2.push_back("head");
    cmdline2.push_back("-n");
    cmdline2.push_back("5");
    Atomic cmd2(cmdline2, redirects, false);

    Pipe cmd(&cmd1, &cmd2);

    printf("%d\n", cmd.run());

    return 0;
}
