#include<string>
#include<map>
#include"../src/utility.h"

using namespace std;

void unittest(){
    map<string,int> m;
    m["zero"] = 0;
    m["one"] = 1;
    m["two"] = 2;
    for(auto p : m){
        printf("%s, %d\n",p.first.c_str(),p.second);
    }
    printf("%d\n",m["two"]);
    pbool(m.find("three") == m.end(), "Do not crash on find invalid elements");
    m.insert(pair<string,int>("three",3));
    pbool(m.find("three") != m.end(), "find valid elements");
    printf("%d\n",m["three"]);
    m["four"] = 4;
    m["five"] = 5;
    m["six"] = 6;
    m["seven"] = 7;
    m["eight"] = 8;
    m.erase("zero");
    m.erase("six");
    for(auto p : m){
        printf("%s, %d\n",p.first.c_str(),p.second);
    }
    pbool(!m.count("six"), "Do not find deleted elements");

}
