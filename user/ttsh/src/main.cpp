#include <iostream>
#include "Commands.h"
#include "Scanner.h"

using namespace std;

int main() {
    auto& input = cin;
    while(true) {
        printf("$ ");
        Scanner scan(&input,"test"); // load scanner
        Command* cmd;
        yy::parser parser(scan,cmd); //load parser
        parser.parse(); //parse

        cmd->run();

        delete cmd;
    }

    return 0;
}
