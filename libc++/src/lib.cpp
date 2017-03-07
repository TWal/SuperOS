#include "../cstdlib"
#include "../vector"
#include "../string"
#include "../memory"
#include "../new"
#include "../iterator"
#include "../algorithm"
#include "../map"

using namespace std;

vector<string> split(std::string str,char separator,bool keepEmpty){
    vector<string> res ;
    size_t pos = 0;
    while (pos < str.size()){
        size_t pos2 = str.find_first_of(separator,pos);
        if (pos2 == string::npos){
            res.push_back(str.substr(pos));
            break;
        }
        if (keepEmpty || pos2 > pos){
            res.push_back(str.substr(pos,pos2 - pos));
        }
        pos = pos2 +1;
    }
    return res;
}

std::string concat(std::vector<std::string> strs,char separator){
    string res;
    for(auto s : strs){
        res.append(s);
        res.push_back(separator);
    }
    res.pop_back();
    return res;
}


