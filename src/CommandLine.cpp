#include "CommandLine.h"
#include "globals.h"

using namespace std;

CommandLine::CommandLine(table_type table):_table(table){

    //builtin commands here
    _table.push_back(make_pair("echo",[](const vector<string>& args){
                for(auto s : args){
                    fb.printf("%s ",s.c_str());
                }
                fb.printf("\n");
            }));
    _table.push_back(make_pair("help",[](const vector<string>& args){
                (void)args;
                fb.printf(
                    "Help of Super OS (tm) : \n"
                    "Builtin Commands :\n"
                    "  echo <arg> : print argument <arg>\n"
                    );
            }));

}
void CommandLine::run(){
    while(true) {
        bool found = false;
        fb.printf("$ ");
        auto input = readCommand();
        if(input.size() == 0) continue;
        for(auto p : _table){
            if(p.first == input[0]){
                //fb.printf("executing command : %s\n",p.first.c_str());
                input.erase(input.begin());
                p.second(input);
                found = true;
                break;
            }
        }
        if(!found){
            fb.printf("Command %s not found\n",input[0].c_str());
        }
    }
}

std::string CommandLine::readCommandText(){
    std::string res;
    //fb.printf("hey ! \n");
    while(true) {
        Keycode kc = kbd.poll();
        //fb.printf("Reading Command");
        if(!kc.isRelease && kc.symbol > 0) {
            if(kc.symbol == '\b') {
                fb.puts("\b \b");
            } else {
                fb.putc(kc.symbol);
            }
        }
        else continue;
        if(kc.symbol == '\n') break;
        if(kc.symbol == '\b'){
            res.pop_back();
            continue;
        }
        if (kc.symbol != -1) res.push_back(kc.symbol);
    }
    //fb.printf("Command : %s",res.c_str());
    return res;
}
std::vector<std::string> CommandLine::readCommand(){
    return split(readCommandText(),' '); // may be more complicated later.
}

void CommandLine::add(std::string name,command_func func){
    for(auto p : _table){
        if(p.first == name){
            return;
        }
    }
    _table.push_back(make_pair(name,func));
}
