#include<string>
#include"../src/utility.h"

using namespace std;

void unittest(){
    auto v = split("dsjkflqsd,dsfjklsfjsq,fsfjsdfl,sqdjflk,f,sd,,,,fksdjqfls",',');
    for(string& s : v){
        printf("%s ; ",s.c_str());
    }
    printf("\n\nconcat : %s",concat(v,';').c_str());
}
