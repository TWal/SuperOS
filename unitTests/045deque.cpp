#include<deque>
#include<string>

using namespace std;

void unittest(){
    deque<string> d;

    d.push_back("un");
    d.push_back("deux");
    d.push_back("trois");
    d.push_back("quatre");
    d.push_back("cinq");
    d.push_back("six");
    for(auto s : d){
        printf("%s ", s.c_str());
    } printf("\n");
    printf("size : %d\n",d.size());
    d.pop_front();
    for(auto s : d){
        printf("%s ", s.c_str());
    } printf("\n");
    d.push_back("ty");
    d.pop_front();
    d.push_back("ta");
    d.pop_front();
    d.push_back("tb");
    d.pop_front();
    d.push_back("tc");
    d.pop_front();
    for(auto s : d){
        printf("%s ", s.c_str());
    } printf("\n");

}
