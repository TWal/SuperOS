#include<string>
#include"../src/utility.h"

using namespace std;

void unittest(){
    printf("libc++ test :\n");
    string s = "hello world!";
    for(auto v : s){
        printf("%c",v);
    }
    printf(" = %s of size %llu\n", s.c_str(),s.size());
    pbool(s == "hello world!","String equality");
    string s2;
    s2 = s;
    s = "123456789";
    s.erase(s.begin()+3,s.end() -4);
    printf("%s and %s and %d\n",s.c_str(),s2.c_str(),s.find_first_of('8'));
    s+= s2;
    printf("hello = %s neq %s\n",s2.substr(0,5).c_str(), (s += s2).c_str());
    printf("s = %s\n",s.c_str());
    pbool(s < s2,"String comparaison");

}
