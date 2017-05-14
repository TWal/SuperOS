#include "Syscalls.h"
#include "Syscall.h"
#include "../Processes/Scheduler.h"
#include <errno.h>

void syscallFill(){
    handlers[SYSREAD] = sysread;
    handlers[SYSWRITE] = syswrite;
    handlers[SYSBRK] = sysbrk;
    handlers[SYSDUP] = sysdup;
    handlers[SYSDUP2] = sysdup2;
    handlers[SYSCLONE] = sysclone;
    handlers[SYSFORK] = sysfork;
    handlers[SYSEXIT] = sysexit;
    handlers[SYSTEXIT] = systexit;
    handlers[SYSWAIT] = syswait;
}

u64 sread(Thread*t,uint fd,void* buf,u64 count){
    auto pro = t->getProcess();
    if(pro->_fds.size() <= fd) return -EBADF;
    if(pro->_fds[fd].empty()) return -EBADF;
    if(!pro->_fds[fd]->check(Stream::READABLE)) return -EBADF;
    return pro->_fds[fd]->read((void*)buf,count); // UserMem must still be active
}

u64 sysread(u64 fd,u64 buf,u64 count,u64,u64,u64){
    Thread* t = schedul.enterSys();
    //fprintf(stderr,"sysread by %d on %lld to %p with size %lld\n",
    //       t->getTid(),fd,buf,count);
    auto tmp = sread(t,fd,(void*)buf,count);
    //fprintf(stderr,"sysread by %d on %lld to %p with size %lld returning %lld\n",
    //       t->getTid(),fd,buf,count,tmp);
    return tmp;
}

u64 swrite(Thread*t,uint fd,const void* buf,u64 count){
    auto pro = t->getProcess();
    if(pro->_fds.size() <= fd) return -EBADF;
    if(pro->_fds[fd].empty()) return -EBADF;
    if(!pro->_fds[fd]->check(Stream::WRITABLE)) return -EBADF;
    return pro->_fds[fd]->write((void*)buf,count); // UserMem must still be active
}

u64 syswrite(u64 fd,u64 buf,u64 count,u64,u64,u64){
    Thread* t = schedul.enterSys();
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld\n",
    //        t->getTid(),fd,buf,count);
    auto tmp = swrite(t,fd,(const void*)buf,count);
    //fprintf(stderr,"syswrite by %d on %lld to %p with size %lld returning %lld\n",
    //       t->getTid(),fd,buf,count,tmp);
    return tmp;
}

u64 sysbrk(u64 addr,u64,u64,u64,u64,u64){
    Thread* t = schedul.enterSys();
    fprintf(stderr,"sysbrk by %d with 0x%p\n",t->getTid(),addr);
    assert((i64)addr >= 0);
    auto tmp = t->getProcess()->_heap.brk((void*)addr);
    fprintf(stderr,"sysbrk by %d with 0x%p returning %p\n",t->getTid(),addr,tmp);
    return tmp;
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
