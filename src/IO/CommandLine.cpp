#include "CommandLine.h"
#include "FrameBuffer.h"
#include "Keyboard.h"

using namespace std;

CommandLine::CommandLine(table_type table):_table(table){

    //builtin commands here
    _table.insert(make_pair("echo", [](CommandLine*, const vector<string>& args) {
        for(auto s : args) {
            printf("%s ", s.c_str());
        }
        fb.printf("\n");
    }));

    _table.insert(make_pair("help",[](CommandLine*, const vector<string>&) {
        printf(
            "Help of Super OS (tm) : \n"
            "Builtin Commands :\n"
            "  echo <arg> : print argument <arg>\n"
        );
    }));

    _table.insert(make_pair("ls", [](CommandLine* cl, const vector<string>&) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        auto v = cl->pwd->getFilesName();
        for(auto f : v) printf("%s ",f.c_str());
        printf("\n");
    }));

    _table.insert(make_pair("cd", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        File* f = (*cl->pwd)[args[0]];
        if(f == nullptr) {
            printf("%s doesn't exist in current directory\n",args[0].c_str());
            return;
        }
        Directory* d = f->dir();
        if(d == nullptr) {
            printf("%s is not a directory\n",args[0].c_str());
            return;
        }
        cl->pwd = d;
    }));

    _table.insert(make_pair("cat", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        for(const string& file : args) {
            File* f = (*cl->pwd)[file];
            if(f == nullptr) {
                printf("cat: %s: No such file or directory\n", file.c_str());
                continue;
            }
            if(f->getType() == FileType::Directory) {
                printf("cat: %s: Is a directory\n", file.c_str());
                continue;
            }
            char buffer[1025];
            size_t size = f->getSize();
            for(size_t i = 0; i < size; i += 1024) {
                size_t count = min((size_t)1024, size-i);
                f->readaddr(i, buffer, count);
                buffer[count] = 0;
                printf("%s", buffer);
            }
            printf("\n");
        }
    }));
    _table.insert(make_pair("reboot", [](CommandLine*, const vector<string>&) {
        reboot();
    }));
}

void CommandLine::run(){
    while(true) {
        printf("$ ");
        auto input = readCommand();
        if(input.size() == 0) continue;
        auto it = _table.find(input[0]);
        if(it == _table.end()){
            printf("Command %s not found\n",input[0].c_str());
            continue;
        }
        input.erase(input.begin());
        it->second(this,input);
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
                if(!res.empty()) {
                    fb.puts("\b \b");
                }
            } else {
                fb.putc(kc.symbol);
            }
        }
        else continue;
        if(kc.symbol == '\n') break;
        if(kc.symbol == '\b'){
            if(!res.empty()) {
                res.pop_back();
            }
            continue;
        }
        if (kc.symbol != -1) res.push_back(kc.symbol);
    }
    //fb.printf("Command : %s",res.c_str());
    return res;
}
std::vector<std::string> CommandLine::readCommand(){
    return split(readCommandText(),' ',false); // may be more complicated later.
}

void CommandLine::add(std::string name,command_func func){
    _table[name] = func;
}
