#include "Syscalls.h"
#include "Syscall.h"
#include "SyscallUtils.h"
#include "../Processes/Scheduler.h"
#include "../HDD/VFS.h"
#include "../Streams/BytesStream.h"
#include <errno.h>
#include "../Streams/PipeStream.h"
#include <unistd.h>
#include "../Graphics/GraphWindow.h"
#include "../Graphics/Workspace.h"

using namespace std;


void syscallFill(){
    handlers[SYSREAD] = sysread;
    handlers[SYSWRITE] = syswrite;
    handlers[SYSOPEN] = sysopen;
    handlers[SYSCLOSE] = sysclose;
    handlers[SYSSEEK] = sysseek;
    handlers[SYSBRK] = sysbrk;
    handlers[SYSPIPE] = syspipe;

    handlers[SYSOPENWIN] = sysopenwin;
    handlers[SYSOPENTWIN] = sysopentwin;
    handlers[SYSRESIZEWIN] = sysresizewin;
    handlers[SYSGETSIZE] = sysgetsize;
    handlers[SYSGETOFF] = sysgetoff;
    handlers[SYSGETWS] = sysgetws;
    handlers[SYSGETEVT] = sysgetevt;

    handlers[SYSDUP] = sysdup;
    handlers[SYSDUP2] = sysdup2;

    handlers[SYSCLONE] = sysclone;
    handlers[SYSFORK] = sysfork;
    handlers[SYSEXIT] = sysexit;
    handlers[SYSEXEC] = sysexec;
    handlers[SYSTEXIT] = systexit;
    handlers[SYSWAIT] = syswait;

    handlers[SYSOPEND] = sysopend;
    handlers[SYSREADD] = sysreadd;
    handlers[SYSCHDIR] = syschdir;
    handlers[SYSRENAME] = sysrename;
    handlers[SYSMKDIR] = sysmkdir;
    handlers[SYSRMDIR] = sysrmdir;
    handlers[SYSLINK] = syslink;
    handlers[SYSUNLINK] = sysunlink;
}

//non blocking read
static u64 nbread(Thread*t, uint fd, void* buf, u64 count){
    auto pro = t->getProcess();
    if(!pro->_usermem.in(buf)) return -EFAULT;
    // TODO first and last bytes in does not mean all.
    if(!pro->_usermem.in((char*)buf + count)) return -EFAULT;
    if(pro->_fds.size() <= fd) return -EBADF;
    FileDescriptor& desc = pro->_fds[fd];
    if(!desc.hasStream()) return -EBADF;
    if(!desc.check(Stream::READABLE)) return -EBADF;
    size_t res = desc->read(buf,count); // UserMem must still be active
    if(res == 0){
        if(desc->eof()) return 0;
        else{
            assert(desc.check(Stream::WAITABLE));
            return -EBLOCK;
        }
    }
    return res;
}

u64 sysread(u64 fd, u64 buf, u64 count, u64,u64,u64){
    Thread* t = schedul.enterSys();
    //fprintf(stderr,"sysread by %d on %lld to %p with size %lld\n",
    //       t->getTid(),fd,buf,count);
    auto res = nbread(t,fd,(void*)buf,count);
    if(res == size_t(-EBLOCK)){
        auto pro = t->getProcess();
        FileDescriptor& desc = pro->_fds[fd];
        t->wait({desc.get()},[&desc,fd,buf,count](Waiting* th, Waitable*){
                Thread* t = static_cast<Thread*>(th);
                t->getProcess()->prepare();
                size_t res = nbread(t,fd,(void*)buf,count);
                if(res == size_t(-EBLOCK)){
                    t->refuse();
                }
                else{
                    t->accept();
                    t->context.rax = res;
                }
            });
        schedul.stopCurent();
        schedul.run();

    }
    //fprintf(stderr,"sysread by %d on %lld to %p with size %lld returning %lld\n",
    //       t->getTid(),fd,buf,count,tmp);
    return res;
}

u64 swrite(Thread*t, uint fd, const void* buf, u64 count){
    auto pro = t->getProcess();
    if(!pro->_usermem.in(buf)) return -EFAULT;
    // TODO first and last bytes in does not mean all.
    if(!pro->_usermem.in((char*)buf + count)) return -EFAULT;
    if(pro->_fds.size() <= fd) return -EBADF;
    if(!pro->_fds[fd].hasStream()) return -EBADF;
    if(!pro->_fds[fd].check(Stream::WRITABLE)) return -EBADF;
    /*if(pro->_fds[fd].getType() == FileDescriptor::FDtype::GWINDOW){
        info("debug write %p",t);
        for(int i = 0 ; i < count ; ++i){
            info("%x ",((char*) buf)[i]);
        }
        stop;
        }*/
    return pro->_fds[fd]->write((void*)buf,count); // UserMem must still be active
}

u64 syswrite(u64 fd, u64 buf, u64 count, u64,u64,u64){
    Thread* t = schedul.enterSys();
    /*fprintf(stderr,"syswrite by %d on %lld to %p with size %lld\n",
            t->getTid(),fd,buf,count);*/
    auto tmp = swrite(t,fd,(const void*)buf,count);
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld returning %lld\n",
    //       t->getTid(),fd,buf,count,tmp);
    return tmp;
}

u64 sysopen(u64 upath, u64 flags, u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    char* path = (char*)upath;
    if(!pro->_usermem.in((void*)upath)) return -EFAULT;
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), path);

    if(!f){
        if(flags & O_CREAT){
            auto p = splitFileName(path);
            f = resolvePath(pro->_wd.get(), p.first.c_str());
            if(f->getType() != HDD::FileType::Directory) {
                return -EACCESS;
            }
            std::unique_ptr<HDD::Directory> d = std::lifted_static_cast<HDD::Directory>(std::move(f));
            d->addEntry(p.second,0,0, S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP
                        | S_IRUSR | S_IWUSR | S_IFREG);
            f = (*d)[p.second];
            assert(f);
        }
        else return -EACCESS;
    }

    u32 newfd = pro->getFreeFD();
    if(pro->_fds.size() <= newfd) pro->_fds.resize(newfd+1);
    pro->_fds[newfd] = FileDescriptor(file2Stream(std::move(f)));
    pro->_fds[newfd]._mask = Stream::SEEKABLE;
    if(flags & O_RDONLY) pro->_fds[newfd]._mask |= Stream::READABLE | Stream::WAITABLE;
    if(flags & O_WRONLY) pro->_fds[newfd]._mask |= Stream::WRITABLE | Stream::APPENDABLE;
    return newfd;
}

u64 sysclose(u64 fd, u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(pro->_fds.size() <= fd) return - EBADF;
    if(pro->_fds[fd].empty()) return - EBADF;
    pro->_fds[fd] = FileDescriptor();
    return 0;
}

/*// non-blocking poll
u64 nbpoll(Thread* t, pollfd* pollfds, u64 nbfds){
    auto pro = t->getProcess();
    u64 nbRet = 0;
    for(u64 i = 0 ; i < nbfds ; ++i){
        if(pro->_fds.size() <= pollfds[i].fd){
            ++nbRet;
            pollfds[i].ret = POLLNVAL;
        }
        
    }
}

u64 syspoll(u64 pollfds, u64 nbfds,u64,u64,u64,u64){
   
}*/

u64 sysseek(u64 fd, u64 offset, u64 mode ,u64,u64,u64){
    //debug(Syscalls,"Seek %llu to %lld from %lld", fd, offset, mode);
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(pro->_fds.size() <= fd) return -EBADF;
    FileDescriptor& desc = pro->_fds[fd];
    if(!desc.hasStream()) return -EBADF;
    if(!desc.check(Stream::SEEKABLE)) return -EBADF;
    i64 size = desc->end();
    i64 cur = desc->tell();
    i64 origin;
    if(mode == SEEK_SET){
        origin = 0;
    }
    else if(mode == SEEK_CUR){
        origin = cur;
    }
    else if(mode == SEEK_END){
        origin = size;
    }
    else return - EINVAL;
    origin += offset;
    if(origin < 0) return -EINVAL;
    if(origin > size and !desc.check(Stream::APPENDABLE)) return -EINVAL;
    desc->seek(origin,Stream::mod::BEG);
    return origin;
}

u64 sysbrk(u64 addr,u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    //debug(Syscalls,"sysbrk by %d with 0x%p\n",t->getTid(),addr);
    if((i64)addr < 0x200000 and addr != 0) return - EFAULT;
    auto tmp = t->getProcess()->_heap.brk((void*)addr);
    //debug(Syscalls,"sysbrk by %d with 0x%p returning %p\n",t->getTid(),addr,tmp);
    t->getProcess()->_usermem.DumpTree();
    if(addr !=0) assert(t->getProcess()->_usermem.in((void*)(addr-1)));
    return tmp;
}


u64 syspipe(u64 fd2, u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)fd2)) return -EFAULT;
    int* res = (int*)fd2;
    res[0] = pro->getFreeFD();
    if(pro->_fds.size() <= res[0]) pro->_fds.resize(res[0]+1);
    res[1] = pro->getFreeFD();
    if(pro->_fds.size() <= res[1]) pro->_fds.resize(res[1]+1);
    PipeStream* ps = new PipeStream();
    pro->_fds[res[0]] = FileDescriptor(std::unique_ptr<Stream>(new PipeStreamOut(ps)));
    pro->_fds[res[1]] = FileDescriptor(std::unique_ptr<Stream>(new PipeStreamIn(ps)));
    return 0;
}

/*__        ___           _
  \ \      / (_)_ __   __| | _____      _____
   \ \ /\ / /| | '_ \ / _` |/ _ \ \ /\ / / __|
    \ V  V / | | | | | (_| | (_) \ V  V /\__ \
     \_/\_/  |_|_| |_|\__,_|\___/ \_/\_/ |___/
*/

u64 sysopenwin(u64 size, u64 offset, u64 workspace, u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();

    // size checking
    video::Vec2u vSize(size);
    video::Vec2u vOffset(offset);
    auto botright = vSize + vOffset;
    if(botright.x > video::screen.getSize().x) return -EINVAL;
    if(botright.y > video::screen.getSize().y) return -EINVAL;
    if (workspace >= video::Workspace::_totalNumber) return -EINVAL;

    // getting the file descriptor.
    int fd = pro->getFreeFD();
    if(pro->_fds.size() <= u64(fd)) pro->_fds.resize(fd+1);
    video::GraphWindow* w = new video::GraphWindow(vOffset, vSize);
    pro->_fds[fd] = FileDescriptor(w);
    video::Workspace::get(workspace).addWin(w);
    return fd;
}

u64 sysopentwin(u64 size, u64 offset, u64 workspace, u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();

    // size checking
    video::Vec2u vSize(size);
    video::Vec2u vOffset(offset);
    auto botright = vSize + vOffset;
    if(botright.x > video::screen.getSize().x) return -EINVAL;
    if(botright.y > video::screen.getSize().y) return -EINVAL;
    if (workspace >= video::Workspace::_totalNumber) return -EINVAL;

    // getting the file descriptor.
    int fd = pro->getFreeFD();
    if(pro->_fds.size() <= u64(fd)) pro->_fds.resize(fd+1);
    video::TextWindow* w = new video::TextWindow(vOffset, vSize, video::Font::def);
    pro->_fds[fd] = FileDescriptor(w);
    video::Workspace::get(workspace).addWin(w);
    return fd;
}

u64 sysresizewin(u64 fd, u64 size, u64 offset, u64, u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();

    // size checking
    video::Vec2u vSize(size);
    video::Vec2u vOffset(offset);
    auto botright = vSize + vOffset;
    if(botright.x >= video::screen.getSize().x) return -EINVAL;
    if(botright.y >= video::screen.getSize().y) return -EINVAL;

    // getting the file descriptor.
    if(pro->_fds.size() <= u64(fd)) return -EBADF;
    if(!pro->_fds[fd].isWin()) return - EBADF;
    pro->_fds[fd].getWin()->setSize(vSize);
    pro->_fds[fd].getWin()->setOffset(vOffset);
    return 0;
}

u64 sysgetsize(u64 fd, u64, u64, u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    // getting the file descriptor.
    if(pro->_fds.size() <= u64(fd)) return -EBADF;
    if(!pro->_fds[fd].isWin()) return - EBADF;
    return pro->_fds[fd].getWin()->getSize().to<u64>();
}

u64 sysgetoff(u64 fd, u64, u64, u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    // getting the file descriptor.
    if(pro->_fds.size() <= u64(fd)) return -EBADF;
    if(!pro->_fds[fd].isWin()) return - EBADF;
    return pro->_fds[fd].getWin()->getOffset().to<u64>();
}

u64 sysgetws(u64 fd, u64, u64, u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    // getting the file descriptor.
    if(pro->_fds.size() <= u64(fd)) return -EBADF;
    if(!pro->_fds[fd].isWin()) return - EBADF;
    return pro->_fds[fd].getWin()->getWS();
}

u64 sysgetevt(u64 fd, u64, u64, u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    // getting the file descriptor.
    if(pro->_fds.size() <= u64(fd)) return -EBADF;
    if(pro->_fds[fd].getType() != FileDescriptor::FDtype::GWINDOW) return -EBADF;
    return static_cast<video::GraphWindow*>(pro->_fds[fd].getWin())->getEvent();
}

/* ____
  |  _ \ _   _ _ __
  | | | | | | | '_ \
  | |_| | |_| | |_) |
  |____/ \__,_| .__/
              |_|
*/

u64 sysdup(u64 oldfd,u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    u32 newfd = pro->getFreeFD();
    if(pro->_fds.size() <= oldfd) return - EBADF;
    if(pro->_fds.size() <= newfd) pro->_fds.resize(newfd+1);
    pro->_fds[newfd] = pro->_fds[oldfd];
    return newfd;
}

u64 sysdup2(u64 oldfd, u64 newfd, u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(pro->_fds.size() <= oldfd) return - EBADF;
    if(pro->_fds.size() <= newfd) pro->_fds.resize(newfd+1);
    pro->_fds[newfd] = pro->_fds[oldfd];
    return newfd;
}

/* ____
  |  _ \ _ __ ___   ___ ___  ___ ___  ___  ___
  | |_) | '__/ _ \ / __/ _ \/ __/ __|/ _ \/ __|
  |  __/| | | (_) | (_|  __/\__ \__ \  __/\__ \
  |_|   |_|  \___/ \___\___||___/___/\___||___/
*/

u64 sysclone(u64 rip,u64 rsp,u64,u64,u64,u64){
    return schedul.clone(rip,rsp);
}

u64 sysfork(u64,u64,u64,u64,u64,u64){
    return schedul.fork();
}

u64 sysexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.exit(rc);
}

u64 sysexec(u64 upath, u64 argv, u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    char* path = (char*)upath;
    if(!pro->_usermem.in((void*)upath)) return -EFAULT;
    info(Syscalls,"Exec in %d to %s", t->getTid(), path);


    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), path);

    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::RegularFile) return -EACCESS;
    debug("exec : The file is OK, now computing argc");
    char** targv = (char**)argv;
    debug(Syscalls, "argv = %p",targv);
    //pro->_usermem.DumpTree();
    if(!pro->_usermem.in(targv)) return - EFAULT;
    int argc = 0;
    std::vector<std::string> argvSave;
    while(targv[argc]){
        debug("Testing %p",targv[argc]);
        if(!pro->_usermem.in(targv[argc])) return -EFAULT;
        debug(Syscalls,"argument %d is %s",argc,targv[argc]);
        argvSave.push_back(targv[argc]); // TODO correct security failure.
        ++argc;
    }
    // all is OK, ready to exec
    schedul.stopCurent();
    pro->clear();
    Thread* nt = pro->loadFromBytes(static_cast<HDD::RegularFile*>(f.get()));
    pro->prepare();
    std::vector<void*> argvSave2;
    for(const auto& s : argvSave){
        argvSave2.push_back(nt->push(s.data(),s.size()+1));
    }
    void * p = nullptr;
    nt->push(&p,8);
    for(int i = argvSave2.size()-1 ; i >= 0 ; --i){
        nt->push(&argvSave2[i],8);
    }
    nt->context.rdi = argc;
    nt->context.rsi = nt->context.rsp;

    schedul.run();
}

u64 systexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.texit(rc);
}

u64 syswait(u64 pid,u64 status,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    if(status and !t->getProcess()->_usermem.in((void*)status)) return -EFAULT;
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld\n",
    //        t->getTid(),fd,buf,count);
    if(pid == 0){
        return t->waitp((u64*)status);
    }
    else{
        return t->waitp(schedul.getP(pid),(u64*)status);
    }
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld returning %lld\n",
    //        t->getTid(),fd,buf,count,tmp);
    //return tmp;
}


/* _____ _ _      ____            _
  |  ___(_) | ___/ ___| _   _ ___| |_ ___ _ __ ___
  | |_  | | |/ _ \___ \| | | / __| __/ _ \ '_ ` _ \
  |  _| | | |  __/___) | |_| \__ \ ||  __/ | | | | |
  |_|   |_|_|\___|____/ \__, |___/\__\___|_| |_| |_|
                        |___/
*/

u64 sysopend(u64 path, u64,u64,u64,u64,u64) {
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path)) return -EFAULT;
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), (char*)path);
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -ENOTDIR;
    std::unique_ptr<HDD::Directory> d = std::lifted_static_cast<HDD::Directory>(std::move(f));

    u32 newfd = pro->getFreeFD();
    if(pro->_fds.size() <= newfd) pro->_fds.resize(newfd+1);
    pro->_fds[newfd] = FileDescriptor(std::move(d));
    return newfd;
}

u64 sysreadd(u64 fd, u64 thedirent,u64,u64,u64,u64) {
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)thedirent)) return -EFAULT;
    if(!pro->_usermem.in((void*)(thedirent + sizeof(dirent)))) return -EFAULT;
    if(pro->_fds.size() <= fd) return -EBADF;
    FileDescriptor& filed = pro->_fds[fd];
    if(!filed.isDir()) return -EBADF;
    dirent* res = filed.getDir()->read(filed.getDopen());
    if(res != nullptr) {
        memcpy((void*)thedirent, res, sizeof(dirent));
    } else {
        memset((void*)thedirent, 0, sizeof(dirent));
    }
    return 0;
}

u64 syschdir(u64 path, u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path)) return -EFAULT;
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), (char*)path);
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -ENOTDIR;
    pro->_wd = lifted_static_cast<HDD::Directory>(move(f));
    return 0;
}

//TODO: test this!!!
u64 sysrename(u64 path1, u64 path2,u64,u64,u64,u64) {
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path1) || !pro->_usermem.in((void*)path2)) return -EFAULT;

    // find the directory
    auto sp1 = splitFileName((char*)path1);
    auto sp2 = splitFileName((char*)path2);

    //find directory containing path1
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), sp1.first.c_str());
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -EACCESS;
    auto dir1 = lifted_static_cast<HDD::Directory>(move(f));

    //find directory containing path1
    f = resolvePath(pro->_wd.get(), sp2.first.c_str());
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -EACCESS;
    auto dir2 = lifted_static_cast<HDD::Directory>(move(f));

    // check path2 does not exists.
    f = (*dir2)[sp2.second];
    if(f) return -EEXIST;

    auto fpath1 = (*dir1)[sp1.second];

    stat st;
    fpath1->getStats(&st);
    u32 dev1 = st.st_dev;
    u16 mode = st.st_mode;
    dir2->getStats(&st);
    u32 dev2 = st.st_dev;

    if(dev1 != dev2) return -EXDEV;

    if(S_ISDIR(mode)) {
        //this order to handle '..' properly
        //the link count can't drop to 0 because there is the link count of '.'
        dir1->removeEntry(sp1.second);
        dir2->addEntry(sp2.second, fpath1.get());
    } else {
        //this order to avoid the link count to drop to 0
        dir2->addEntry(sp2.second, fpath1.get()); //before the next line to
        dir1->removeEntry(sp1.second);
    }

    return 0;
}

u64 sysmkdir(u64 path, u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path)) return -EFAULT;

    // find the directory
    auto p = splitFileName((char*)path);
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), p.first.c_str());
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -EACCESS;
    auto dir = lifted_static_cast<HDD::Directory>(move(f));

    // check directory does not exists.
    auto testf = (*dir)[p.second];
    if(testf) return -EEXIST;

    // create the directory
    dir->addEntry(p.second, 0, 0, S_IRWXO | S_IRWXG | S_IRWXU | S_IFDIR);
    return 0;
}


u64 sysrmdir(u64 path, u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path)) return -EFAULT;

    // find the directory
    auto p = splitFileName((char*)path);
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), p.first.c_str());
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -EACCESS;
    auto dir = lifted_static_cast<HDD::Directory>(move(f));

    // check directory does exists.
    auto testf = (*dir)[p.second];
    if(!testf) return -EACCESS;
    //check if it is a directory
    if(testf->getType() != HDD::FileType::Directory) return -EACCESS;
    //check if it is empty
    if(!std::lifted_static_cast<HDD::Directory>(std::move(testf))->isEmpty()) return -EEXIST;

    // remove the directory
    dir->removeDirectory(p.second);
    return 0;
}


u64 syslink(u64 path1, u64 path2,u64,u64,u64,u64) {
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path1) || !pro->_usermem.in((void*)path2)) return -EFAULT;

    // find the directory
    auto p = splitFileName((char*)path2);
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), p.first.c_str());
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -EACCESS;
    auto dir = lifted_static_cast<HDD::Directory>(move(f));

    // check path2 does not exists.
    auto testf = (*dir)[p.second];
    if(testf) return -EEXIST;

    auto fpath1 = resolvePath(pro->_wd.get(), (char*)path1);

    if(!fpath1) return -EACCESS;

    if(fpath1->getType() != HDD::FileType::RegularFile) return -EACCESS;

    stat st;
    fpath1->getStats(&st);
    u32 dev1 = st.st_dev;
    dir->getStats(&st);
    u32 dev2 = st.st_dev;

    if(dev1 != dev2) return -EXDEV;

    dir->addEntry(p.second, fpath1.get());
    return 0;
}


u64 sysunlink(u64 path, u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path)) return -EFAULT;

    // find the directory
    auto p = splitFileName((char*)path);
    std::unique_ptr<HDD::File> f = resolvePath(pro->_wd.get(), p.first.c_str());
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::Directory) return -EACCESS;
    auto dir = lifted_static_cast<HDD::Directory>(move(f));

    // check file does exists.
    auto testf = (*dir)[p.second];
    if(!testf) return -EACCESS;
    //check if it is a regular file
    if(testf->getType() != HDD::FileType::RegularFile) return -EACCESS;

    // remove the file
    dir->removeEntry(p.second);
    return 0;
}




