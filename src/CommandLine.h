#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <utility>
#include <vector>
#include <string>
#include <map>

class CommandLine{
public :
    typedef void(*command_func)(const std::vector<std::string>&);
    typedef std::map<std::string,command_func> table_type;
private:
    table_type _table;
public :
    explicit CommandLine(table_type table = table_type());
    void run();
    std::string readCommandText();
    std::vector<std::string> readCommand();
    void add(std::string name,command_func func);
};


#endif
