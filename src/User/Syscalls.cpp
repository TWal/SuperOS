#include "Syscalls.h"
#include "Syscall.h"
#include "SyscallUtils.h"
#include "../Processes/Scheduler.h"
#include "../HDD/VFS.h"
#include "../Streams/BytesStream.h"
#include <errno.h>
#include "../Streams/PipeStream.h"

using namespace std;


void syscallFill(){
    handlers[SYSREAD] = sysread;
    handlers[SYSWRITE] = syswrite;
    handlers[SYSOPEN] = sysopen;
    handlers[SYSCLOSE] = sysclose;
    handlers[SYSBRK] = sysbrk;
    handlers[SYSDUP] = sysdup;
    handlers[SYSDUP2] = sysdup2;
    handlers[SYSCLONE] = sysclone;
    handlers[SYSFORK] = sysfork;
    handlers[SYSEXIT] = sysexit;
    handlers[SYSEXEC] = sysexec;
    handlers[SYSTEXIT] = systexit;
    handlers[SYSWAIT] = syswait;
}

u64 sread(Thread*t, uint fd, void* buf, u64 count){
    auto pro = t->getProcess();
    if(!pro->_usermem.in(buf)) return -EFAULT;
    if(pro->_fds.size() <= fd) return -EBADF;
    if(!pro->_fds[fd].hasStream()) return -EBADF;
    if(!pro->_fds[fd].check(Stream::READABLE)) return -EBADF;
    return pro->_fds[fd]->read((void*)buf,count); // UserMem must still be active
}

u64 sysread(u64 fd, u64 buf, u64 count, u64,u64,u64){
    Thread* t = schedul.enterSys();
    //fprintf(stderr,"sysread by %d on %lld to %p with size %lld\n",
    //       t->getTid(),fd,buf,count);
    auto tmp = sread(t,fd,(void*)buf,count);
    //fprintf(stderr,"sysread by %d on %lld to %p with size %lld returning %lld\n",
    //       t->getTid(),fd,buf,count,tmp);
    return tmp;
}

u64 swrite(Thread*t, uint fd, const void* buf, u64 count){
    auto pro = t->getProcess();
    if(!pro->_usermem.in(buf)) return -EFAULT;
    if(pro->_fds.size() <= fd) return -EBADF;
    if(!pro->_fds[fd].hasStream()) return -EBADF;
    if(!pro->_fds[fd].check(Stream::WRITABLE)) return -EBADF;
    return pro->_fds[fd]->write((void*)buf,count); // UserMem must still be active
}

u64 syswrite(u64 fd, u64 buf, u64 count, u64,u64,u64){
    Thread* t = schedul.enterSys();
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld\n",
    //        t->getTid(),fd,buf,count);
    auto tmp = swrite(t,fd,(const void*)buf,count);
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld returning %lld\n",
    //       t->getTid(),fd,buf,count,tmp);
    return tmp;
}

enum{O_RDONLY = 1, O_WRONLY = 2, O_RDWR = 3, O_CREAT = 4, O_TRUNC = 8, O_APPEND = 16};
u64 sysopen(u64 upath, u64 flags, u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)upath)) return -EFAULT;
    string s = reinterpret_cast<const char*>(upath);
    if(s.empty()) return -EACCESS;
    HDD::File* f;
    HDD::Directory* d;
    if(s[0] == '/'){
        assert(HDD::VFS::vfs);
        d = HDD::VFS::vfs->getRoot();
    }
    else{
        assert(pro->_wd);
        d = pro->_wd;
    }

    f = d->resolvePath(s);
    if(!f){
        if(flags & O_CREAT){
            auto p = splitFileName(s);
            f = d->resolvePath(p.first);
            if(f->getType() != HDD::FileType::Directory){
                return -EACCESS;
            }
            d = static_cast<HDD::Directory*>(f);
            d->addEntry(p.second,0,0, S_IROTH | S_IWOTH | S_IRGRP | S_IWGRP
                        | S_IRUSR | S_IWUSR | S_IFREG);
            f = (*d)[p.second];
            assert(f);
        }
        else return - EACCESS;
    }

    u32 newfd = pro->getFreeFD();
    if(pro->_fds.size() <= newfd) pro->_fds.resize(newfd+1);
    pro->_fds[newfd] = FileDescriptor(file2Stream(f));
    pro->_fds[newfd]._mask = 0;
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


u64 sysbrk(u64 addr,u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    debug(Syscalls,"sysbrk by %d with 0x%p\n",t->getTid(),addr);
    if((i64)addr < 0x200000 and addr != 0) return - EFAULT;
    auto tmp = t->getProcess()->_heap.brk((void*)addr);
    debug(Syscalls,"sysbrk by %d with 0x%p returning %p\n",t->getTid(),addr,tmp);
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
    pro->_fds[res[0]] = FileDescriptor(new PipeStreamOut(ps));
    pro->_fds[res[1]] = FileDescriptor(new PipeStreamIn(ps));
    return 0;
}

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

u64 sysclone(u64 rip,u64 rsp,u64,u64,u64,u64){
    return schedul.clone(rip,rsp);
}

u64 sysfork(u64,u64,u64,u64,u64,u64){
    return schedul.fork();
}

u64 sysexit(u64 rc,u64,u64,u64,u64,u64){
    schedul.exit(rc);
}

u64 sysexec(u64 path, u64 argv, u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    auto pro = t->getProcess();
    if(!pro->_usermem.in((void*)path)) return -EFAULT;
    info(Syscalls,"Exec in %d to %s", t->getTid(),(char*)path);
    string s = reinterpret_cast<const char*>(path);
    HDD::File* f;
    HDD::Directory* d;
    if(s[0] == '/'){
        assert(HDD::VFS::vfs);
        d = HDD::VFS::vfs->getRoot();
    }
    else{
        assert(pro->_wd);
        d = pro->_wd;
    }

    f = d->resolvePath(s);
    if(!f) return -EACCESS;
    if(f->getType() != HDD::FileType::RegularFile) return -EACCESS;
    // The file is OK, now computing argc;
    char** targv = (char**)argv;
    debug(Syscalls, "argv = %p",targv);
    //pro->_usermem.DumpTree();
    if(!pro->_usermem.in(targv)) return - EFAULT;
    int argc = 0;
    std::vector<std::string> argvSave;
    while(targv[argc]){
        if(!pro->_usermem.in(targv[argc])) return -EFAULT;
        debug(Syscalls,"argument %d is %s",argc,targv[argc]);
        argvSave.push_back(targv[argc]); // TODO correct security failure.
        ++argc;
    }
    // all is OK, ready to exec
    schedul.stopCurent();
    pro->clear();
    Thread* nt = pro->loadFromBytes(static_cast<HDD::RegularFile*>(f));
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
