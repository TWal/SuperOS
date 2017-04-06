#include "FileSystem.h"


FileType File::getType(){
    return FileType::File;
}

FileType Directory::getType(){
    return FileType::Directory;
}


FileSystem::FileSystem(Partition* part) : _part(part){

}
