#include<set>
#include<string>
#include<stdio.h>

using namespace std;

void unittest(){
    set<string> s;
    s.insert("foo");
    s.insert("bar");
    s.insert("test");
    s.insert("42");
    s.insert("bad");
    for(auto t : s){
        printf("%s ",t.c_str());
    }printf("\n");
    s.erase("bad");
    for(auto t : s){
        printf("%s ",t.c_str());
    }printf("\n");
    s.erase("test'");
    for(auto t : s){
        printf("%s ",t.c_str());
    }printf("\n");
}
