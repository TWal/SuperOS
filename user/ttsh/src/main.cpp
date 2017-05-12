#include <iostream>
#include "Commands.h"
#include "Scanner.h"

using namespace std;

int main() {
    /*std::vector<Redirection*> redirects;
    std::vector<std::string> cmdline1;
    cmdline1.push_back("cat");
    cmdline1.push_back("main.cpp");
    Atomic cmd1(cmdline1, redirects, false);

    std::vector<std::string> cmdline2;
    cmdline2.push_back("head");
    cmdline2.push_back("-n");
    cmdline2.push_back("5");
    Atomic cmd2(cmdline2, redirects, false);

    Pipe cmd(&cmd1, &cmd2);*/

    // printf("%d\n", cmd.run());
    auto& input = cin;
    Scanner scan(&input,"test"); // load scanner
    Command* cmd;
    yy::parser parser(scan,cmd); //load parser
    parser.parse(); //parse

    //Atomic cmd({"echo", "prout"},{},false);

    cmd->run();

    return 0;
}
