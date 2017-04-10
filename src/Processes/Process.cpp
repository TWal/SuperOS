#include"Process.h"
#include"../Memory/Paging.h"
#include"../Memory/PhysicalMemoryAllocator.h"
#include"../User/Elf64.h"

void* const loadedProcess = (void*)-0x100000000ll; // -4G
const uptr stackStart =  0x8000000000ull; // 512 G


Process::Process(u32 pid,std::vector<FileDescriptor*> fds) :
    _pid(pid),_terminated(false),_returnCode(0),
    _fds(fds),_userPDP(paging.newUserPDP()){
}





Thread* Process::loadFromBytes(Bytes* file){
    std::vector<void*> allocPages;
    paging.switchUser(_userPDP);

    // smart reader to allow reading beyond end
    auto readaddr = [file](uptr addr, void* buffer,size_t size){
        file->readaddr(addr,buffer,max((size_t)0,min(size,file->getSize() - addr)));
    };

    //loading first page of file
    void* firstblock = physmemalloc.alloc();
    allocPages.push_back(firstblock);
    paging.createMapping((uptr)firstblock,(uptr)loadedProcess);
    readaddr(0,loadedProcess,0x1000);

    // Parsing executable
    Elf64::Elf64 elf((char*)loadedProcess,file->getSize());
    size_t toLoad = elf.toLoadSize();
    toLoad += 0x1000 -1;
    toLoad /= 0x1000;

    //loading enough to have all information in kernel Space
    for(size_t i = 1 ; i < toLoad ; ++i){
        void* block = physmemalloc.alloc();
        allocPages.push_back(block);
        paging.createMapping((uptr)block,(uptr)loadedProcess + i * 0x1000);
        readaddr(i*0x1000,(char*)loadedProcess+i*0x1000,0x1000);
    }

    //Creating mapping for each program header
    for(int i = 0; i < elf.phnum ; ++ i){
        auto ph = elf.getProgramHeader(i);
        //printf("%d, t : %d, off : %p, virt : %llx, size :%d %d\n",i,ph.type,ph.getData(),ph.vaddr,ph.filesz,ph.memsz);

        if(ph.type == Elf64::PT_LOAD){
            printf("loading\n");
            // if this program header is to be loaded
            assert(!(ph.vaddr & (0x1000 -1))); // assert ph.vaddr aligned on 4K

            size_t offset = ph.offset;
            if(offset % 0x1000 == 0){
                // the mapping is 4K aligned : better case
                size_t po = offset / 0x1000;
                size_t nbPages = (ph.filesz + 0x1000-1) / 0x1000;
                //mapping page by page
                for(size_t i = po ; i < nbPages ; ++i){
                    void* phyblock;
                    if(i < toLoad){
                        phyblock = allocPages[i];
                        allocPages[i] = nullptr;
                    }
                    else phyblock = physmemalloc.alloc();
                    assert(phyblock);
                    paging.createMapping((uptr)phyblock,ph.vaddr + i * 0x1000);
                    if( i >= toLoad){
                        readaddr(offset + i * 0x1000,(char*)ph.vaddr + i * 0x1000,0x1000);
                    }
                }
            }
            else bsod("Loading unaligned elf64 is not supported for now");
        }
    }
    //printf("loading stack");
    //WAIT(1000000000000);

    // allocating stack
    for(size_t i = 1 ; i <= FIXED_SIZE_STACK ; ++i){
        void * block = physmemalloc.alloc();
        paging.createMapping((uptr)block,stackStart - i * 0x1000);
    }
    //TODO preparing Heap
    //printf("Creating thread\n");
    //the file is now ready to be executed. Creating main thread
    return new Thread(elf.entry,this);
}






void Process::addThread(Thread* thread){
    _threads.push_back(thread);
}

Process::~Process(){
    assert(_terminated); // check the process is effectively a zombie
}

void Process::terminate(u64 returnCode){
    _returnCode = returnCode;
    _terminated = true;
    paging.freeUserPDP(_userPDP);
    for(auto t : _threads){
        delete t;
    }
    /*for(auto fd : _fds){
        fd->drop();
        }*/
    // this process is now a zombie
}
void Process::prepare(){
    paging.switchUser(_userPDP);
}


Thread::Thread(u64 rip,Process* parent) : _process(parent),wr(nullptr){
    parent->addThread(this);
    context.rip = rip;
    context.rflags = 2 | (1 << 9);
    context.rsp = stackStart;
}

[[noreturn]] void Thread::run(){
    printf("Starting thread\n");
    _process->prepare();
    context.launch();
}
