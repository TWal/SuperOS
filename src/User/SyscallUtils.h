#ifndef SYSCALLUTILS_H
#define SYSCALLUTILS_H

#include "../utility.h"
#include "../log.h"
#include <utility>
#include "../Streams/Stream.h"
#include "../HDD/FileSystem.h"

Stream* file2Stream(HDD::File* f);
std::pair<std::string,std::string> splitFileName(std::string s);

#endif
