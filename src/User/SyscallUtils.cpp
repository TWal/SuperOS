#include "SyscallUtils.h"
#include "../Streams/BytesStream.h"

using namespace std;

Stream* file2Stream(HDD::File* f)
{
    switch(f->getType()){
        case HDD::FileType::Directory:
            return nullptr;
        case HDD::FileType::RegularFile:
            return new BytesStream(static_cast<HDD::RegularFile*>(f));
        case HDD::FileType::BlockDevice:
            return new BytesStream(static_cast<HDD::BlockDevice*>(f));
        case HDD::FileType::CharacterDevice:
            return static_cast<HDD::CharacterDevice*>(f);
    }
    return nullptr;
}

pair<string,string> splitFileName(std::string s){
    for(int i = s.size() -1 ; i >=0 ; --i){
        if(s[i] == '/'){
            string name = s.c_str() + i + 1;
            s.resize(i);
            return make_pair(s,name);
        }
    }
    return make_pair(string(),s);
}
