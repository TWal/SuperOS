#include "CommandLine.h"
#include "FrameBuffer.h"
#include "Keyboard.h"
#include "../log.h"

using namespace std;

void CommandLine::init(table_type table){
    _table = table;

    //builtin commands here
    _table.insert(make_pair("echo", [](CommandLine*, const vector<string>& args) {
        for(auto s : args) {
            printf("%s ", s.c_str());
        }
        printf("\n");
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
        void* d = cl->pwd->open();
        dirent* dir;
        while((dir = cl->pwd->read(d)) != nullptr) {
            printf("%s ", dir->d_name);
        }
        cl->pwd->close(d);
        printf("\n");
    }));

    _table.insert(make_pair("cd", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        HDD::File* f = (*cl->pwd)[args[0]];
        if(f == nullptr) {
            printf("%s doesn't exist in current directory\n",args[0].c_str());
            return;
        }
        HDD::Directory* d = static_cast<HDD::Directory*>(f);
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
            HDD::File* f = (*cl->pwd)[file];
            if(f == nullptr) {
                printf("cat: %s: No such file or directory\n", file.c_str());
                continue;
            }
            if(f->getType() != HDD::FileType::RegularFile) {
                printf("cat: %s: Is not a regular file\n", file.c_str());
                continue;
            }
            HDD::RegularFile* rf = static_cast<HDD::RegularFile*>(f);
            char buffer[1025];
            size_t size = rf->getSize();
            for(size_t i = 0; i < size; i += 1024) {
                size_t count = min((size_t)1024, size-i);
                rf->readaddr(i, buffer, count);
                buffer[count] = 0;
                printf("%s", buffer);
            }
            printf("\n");
        }
    }));

    _table.insert(make_pair("rm", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        for(const string& file : args) {
            HDD::File* f = (*cl->pwd)[file];
            if(f == nullptr) {
                printf("rm: %s: No such file or directory\n", file.c_str());
                continue;
            }
            if(f->getType() == HDD::FileType::Directory) {
                printf("rm: %s: Is a directory\n", file.c_str());
                continue;
            }
            cl->pwd->removeFile(file);
        }
    }));


    _table.insert(make_pair("rmdir", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        for(const string& file : args) {
            HDD::File* f = (*cl->pwd)[file];
            if(f == nullptr) {
                printf("rmdir: %s: No such file or directory\n", file.c_str());
                continue;
            }
            if(f->getType() != HDD::FileType::Directory) {
                printf("rmdir: %s: Is not a directory\n", file.c_str());
                continue;
            }
            HDD::Directory* d = static_cast<HDD::Directory*>(f);
            if(!d->isEmpty()) {
                printf("rmdir: %s: Is not empty\n", file.c_str());
                continue;
            }
            cl->pwd->removeDirectory(file);
        }
    }));

    _table.insert(make_pair("touch", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        for(const string& file : args) {
            HDD::File* f = (*cl->pwd)[file];
            if(f != nullptr) {
                continue;
            }
            cl->pwd->addEntry(file, 0, 0, S_IFREG | S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
        }
    }));


    _table.insert(make_pair("mkdir", [](CommandLine* cl, const vector<string>& args) {
        if(!cl->pwd) {
            printf("fatal error : no pwd set\n");
            return;
        }
        if(args.empty()) return;
        for(const string& file : args) {
            HDD::File* f = (*cl->pwd)[file];
            if(f != nullptr) {
                printf("mkdir: file %s already exists\n", file.c_str());
                continue;
            }
            cl->pwd->addEntry(file, 0, 0, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
        }
    }));

    _table.insert(make_pair("reboot", [](CommandLine*, const vector<string>&) {
        reboot();
    }));
    printf("$ ");
}

void CommandLine::run(){
    while(true) {
        auto input = readCommand();
        if(input.size() == 0) continue;
        if(input.size() == 1 and input[0].size() == 1 and input[0][0] == EOF) break;
        auto it = _table.find(input[0]);
        if(it == _table.end()){
            printf("Command %s not found\n",input[0].c_str());
            printf("$ ");
            continue;
        }
        input.erase(input.begin());
        it->second(this,input);
        printf("$ ");
    }
}

std::string CommandLine::readCommandText(){
    std::string res;
    int c = getchar();
    //debug(CmdLine,"Command Line recieved %d",c);
    res.push_back(c);
    if(c == EOF) return res;
    while((c= getchar()) != EOF){
        if (c == '\n'){
            debug(CmdLine,"Command Line recieved %s",res.c_str());
            return res;
        }
        res.push_back(c);
    }
    assert(false);
}
std::vector<std::string> CommandLine::readCommand(){
    return split(readCommandText(),' ',false); // may be more complicated later.
}

void CommandLine::add(std::string name,command_func func){
    _table[name] = func;
}

CommandLine cl;
