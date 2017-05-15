#include "SyscallUtils.h"
#include "../Streams/BytesStream.h"

using namespace std;

std::unique_ptr<Stream> file2Stream(std::unique_ptr<HDD::File>&& f)
{
    switch(f->getType()){
        case HDD::FileType::Directory:
            return nullptr;
        case HDD::FileType::RegularFile:
            return std::unique_ptr<Stream>(new BytesStream(std::lifted_static_cast<Bytes>(std::lifted_static_cast<HDD::RegularFile>(std::move(f)))));
        case HDD::FileType::BlockDevice:
            return std::unique_ptr<Stream>(new BytesStream(std::lifted_static_cast<Bytes>(std::lifted_static_cast<HDD::BlockDevice>(std::move(f)))));
        case HDD::FileType::CharacterDevice:
            return std::lifted_static_cast<Stream>(std::lifted_static_cast<HDD::CharacterDevice>(std::move(f)));
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
