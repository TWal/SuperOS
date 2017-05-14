#ifndef FILEDESCRIPTOR_H
#define FILEDESCRIPTOR_H

#include "../utility.h"
#include "../Streams/Stream.h"
#include "../HDD/FileSystem.h"
#include "../Graphics/Window.h"
#include "../Graphics/GraphWindow.h"
#include "../Graphics/TextWindow.h"

/**
   @brief This class represent a file descriptor.

   A file descriptor can be a stream, a directory, or a window.

   The file descriptor act as a smart pointer and delete it content when the
   last owner delete it.
 */

class FileDescriptor{
public:
    enum FDtype{EMPTY, STREAM, DIRECTORY, GWINDOW, TWINDOW};
private:
    u64* _owners;
    Stream* _str;
    union{
        HDD::Directory* _dir;
        video::Window* _win;
    };
    FDtype _type;
    void free();
    void drop();
public :
    /// Create an empty file descriptor
    FileDescriptor();
    /**
       @brief Create a file descriptor on a stream.

       Take ownership of the pointer str.
     */
    FileDescriptor(Stream* str);

    /**
       @brief Create a file descriptor on a directory.

       Take ownership of the pointer dir.
    */
    FileDescriptor(HDD::Directory* dir);

    /**
       @brief Create a file descriptor on a graphical window.

       Take ownership of the pointer win.
    */
    FileDescriptor(video::GraphWindow* gwin);

    /**
       @brief Create a file descriptor on a text window.

       Take ownership of the pointer win.
    */
    FileDescriptor(video::TextWindow* twin);

    /**
       @brief Get the type of file descriptor
     */
    FDtype getType(){return _type;}

    ~FileDescriptor();
    FileDescriptor(const FileDescriptor& other);

    FileDescriptor& operator=(const FileDescriptor& fd);


    // Act on underlying stream, crash if there is no underlying stream.
    Stream* operator->(){
        assert(_str && "File Descriptor on non-Stream");
        return _str;
    }

    /// Get pointer to the contained directory or nullptr if there is no contained directory
    HDD::Directory * getDir(){
        if(_type == DIRECTORY) return _dir;
        else return nullptr;
    }

    bool isWin() { return _type == GWINDOW or _type == TWINDOW;}

    video::Window* getWin(){
        if(isWin()) return _win;
        else return nullptr;
    }

    bool empty(){return _type == EMPTY;}
};




#endif
