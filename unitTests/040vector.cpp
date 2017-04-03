#include<vector>
#include"../src/utility.h"

using namespace std;

void unittest(){
    vector<string>v;
    v.push_back("first");
    v.push_back("second");
    v.push_back("third");
    for(auto s : v){
        printf("%s ",s.c_str());
    }printf("\n");
    v.insert(v.begin()+1,"1.5");
    for(auto s : v){
        printf("%s ",s.c_str());
    }printf("\n");
    v.erase(v.begin()+2);
    for(auto s : v){
        printf("%s ",s.c_str());
    }printf("\n");
    v.pop_back();
    for(auto s : v){
        printf("%s ",s.c_str());
    }printf("\n");
    for(int i = 0 ; i <= 9 ; ++i){
        char s[2] = "0";
        s[0]+=i;
        v.insert(v.begin()+1,s);
    }
    for(auto s : v){
        printf("%s ",s.c_str());
    }printf("\n");
    vector<string> v2 = v;
    for(auto s : v2){
        printf("%s ",s.c_str());
    }printf("\n");
    v.clear();
    pbool(v.empty(),"vector is empty");
    for(auto s : v){
        printf("%s ",s.c_str());
    }printf("\n");
    for(auto s : v2){
        printf("%s ",s.c_str());
    }printf("\n");



}
