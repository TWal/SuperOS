#ifndef SYSCALLUTILS_H
#define SYSCALLUTILS_H

#include "../utility.h"
#include "../log.h"
#include <utility>
#include "../Streams/Stream.h"
#include "../HDD/FileSystem.h"

std::unique_ptr<Stream> file2Stream(std::unique_ptr<HDD::File>&& f);
std::pair<std::string,std::string> splitFileName(std::string s);

#endif
