#include <string.h>
#include "FAT.h"
#include "FrameBuffer.h"

using namespace std;

namespace fat {

    //-------------------------File---------------------------------------------

    File::File(FS* fs, uint cluster,ulint size,Directory* parent)
        :_parent(parent), _fs(fs),_cluster(cluster),_size(size){
        _clusterSize = _fs->_fmbr.SectorPerCluster;
    }
    ulint File::getSize(){
        if(!_size) _size = 512* _clusterSize
                       * _fs ->nbRemainingCluster(_cluster);
        return _size;
    }

    void File::writeaddr (ulint addr,const void * data, uint size){
        assert (size <= 512);// will take care of larger write later
        uint clusterOffset = addr / (512*_clusterSize);
        uint offset = addr - clusterOffset *512 * _clusterSize;
        uint LBA = _fs->clusterToLBA(_cluster,clusterOffset);
        _fs->_part->writeaddr(LBA*512 + offset,data,size);
    }
    void File::readaddr (ulint addr, void * data, uint size){
        assert (size <= 512);// will take care of larger read later
        uint clusterOffset = addr / (512*_clusterSize);
        uint offset = addr - clusterOffset *512 * _clusterSize;
        uint LBA = _fs->clusterToLBA(_cluster,clusterOffset);
        _fs->_part->readaddr(LBA*512 + offset,data,size);
    }
    void File::writelba (ulint LBA , const void* data, uint nbsector){
        assert (nbsector == 1);// will take care of larger write later
        uint clusterOffset = LBA / _clusterSize;
        uint offset = LBA - clusterOffset * _clusterSize;
        uint LBAo = _fs->clusterToLBA(_cluster,clusterOffset);
        _fs->_part->writelba(LBAo+ offset,data,nbsector);
    }
    void File::readlba (ulint LBA, void * data, uint nbsector){
        assert (nbsector == 1);// will take care of larger read later
        uint clusterOffset = LBA / _clusterSize;
        uint offset = LBA - clusterOffset * _clusterSize;
        uint LBAo = _fs->clusterToLBA(_cluster,clusterOffset);
        //printf("file reading at %u \n",LBAo + offset);
        _fs->_part->readlba(LBAo+ offset,data,nbsector);
    }

    //void File::setName(const std::string& name){}

    ::Directory* File::getParent(){
        return _parent;
    }


    /*//-------------------------------DATAFile-----------------------------------

    DATAFile::DATAFile(FS* fs, uint cluster, ulint size,Directory* parent)
        :File(fs,cluster,size,parent){}
    */

    //-------------------------------Directory----------------------------------

    std::string LongFileName::getName(){
        char buffer[14]= {};
        for(int i = 0; i < 5 ; ++i){
            buffer[i] = char(first5[i]);
        }
        for(int i = 0 ; i < 6 ; ++i){
            buffer[i+5] = char(second6[i]);
        }
        for(int i = 0 ; i < 2 ; ++i){
            buffer[i+11] = char(third2[i]);
        }
        return buffer;
    }
    std::string DirectoryEntry::getName(){
        string ext(shortName+8,3);
        string beg(shortName,8);
        while(!ext.empty() && ext.back() == ' ') ext.pop_back();
        while(!beg.empty() && beg.back() == ' ') beg.pop_back();
        if(!ext.empty()) {
            beg += ".";
            beg += ext;
        }
        return beg;
    }

    std::string Directory::fusion(const std::map<uchar,LongFileName>& longName){
        std::string res;
        for(auto p : longName){
            res += p.second.getName();
        }
        return res;
    }

    Directory::Directory(FS* fs, uint cluster, ulint size,Directory* parent)
        :fat::File(fs,cluster,size,parent),_loaded(false){}

    void Directory::load(){
        if(_loaded) return;
        _loaded = true;
        static LongFileName buffer[16];
        std::map<uchar,LongFileName> longName;
        for(size_t i = 0 ; i < getLBASize() ; ++i){
            readlba(i,buffer,1);
            for(int j = 0 ; j < 16 ; ++j){
                /*printf("treating area :");
                uchar * buf = reinterpret_cast<uchar*> (&buffer[j]);
                for(int k = 0 ;k < 32 ; ++k){
                    printf("%2x",buf[k]);
                    }printf("\n");*/

                if (buffer[j].order == 0xE5) continue; //deleted or invalid entry.
                if (buffer[j].order == 0x00) return; //end of directory.

                if (buffer[j].attribute == 0x0F){
                    //printf("found long name : %s\n",buffer[j].getName().c_str());
                    //WAIT(100000000);
                    longName [buffer[j].order] = buffer[j];
                    //printf("Registeredlong name");
                    //WAIT(100000000);
                }

                else {
                    DirectoryEntry* entry = reinterpret_cast<DirectoryEntry*>(&buffer[j]);
                    if(entry->directory){
                        if (entry->getCluster() == 0) {
                            entry->setCluster(_fs->_fmbr.RootCluster);
                        }
                        string name = fusion(longName);
                        if(name == "")name = entry->getName();
                        //printf("Found directory %s\n",name.c_str());
                        //WAIT(100000000);
                        _content[name] =
                            new Directory(_fs,entry->getCluster(),entry->size,this);
                        //printf("Registered directory %s",name.c_str());
                        //WAIT(300000000);
                        longName.clear();
                        //printf("Cleared map");
                        //WAIT(300000000);

                    }
                    else if(entry->volume_id){
                        longName.clear();
                    }
                    else { //regular file
                        string name = fusion(longName);
                        if(name == "")name = entry->getName();
                        //printf("Found file %s\n",name.c_str());
                        //WAIT(100000000);
                        _content[name] =
                            new File(_fs,entry->getCluster(),entry->size,this);
                            longName.clear();
                    }
                }
            }
        }
    }


    std::vector<std::string> Directory::getFilesName (){
        load();
        std::vector<std::string> res;
        for(auto p : _content){
            res.push_back(p.first);
        }
        return res;
    }
    File * Directory::operator[](const std::string& name){
        load();
        auto it = _content.find(name);
        if(it == _content.end()) return nullptr;
        else return it->second;
    }
    //-----------------------------FileSystem-----------------------------------

    FS::FS (Partition* part) : FileSystem(part){
        //assert(_fmbr.BytesPerSector == 512); // TODO generic sector size. 
        //uint clusterOffset = LBA / _clusterSize;
        _part->readaddr(0,&_fmbr,sizeof(_fmbr));
        char name [12] = {};
        memcpy(name,_fmbr.Label,11);
        printf("Opening FAT32 filesystem : %s; with Size of %u with reserved sectors %d\n"
                  ,name,_fmbr.FATSize,_fmbr.nbofReservedSectors);
        printf("FAT at sector %d\n",_fmbr.nbofReservedSectors);
        printf("Root Directory at sector %u\n",clusterToLBA(_fmbr.RootCluster,0));

    }

    uint FS::getFATEntry(uint cluster){
        uint res;
        _part->readaddr(_fmbr.nbofReservedSectors *512 + cluster *4,&res,4);
        return res;
    }
    uint FS::clusterToLBA(uint cluster,uint offset){
        cluster &= 0x0FFFFFFF;
        assert (cluster != 0x0FFFFFF7);
        if (cluster > 0x0FFFFFF7) return -1;
        if(offset == 0){
            return _fmbr.nbofReservedSectors
                + _fmbr.nbofFAT*_fmbr.FATSize-2
                + cluster*_fmbr.SectorPerCluster;
            //-2 is because the first 2 entries of the FAT are unused
        }
        else {
            return clusterToLBA(getFATEntry(cluster),offset-1);
        }
    }
    uint FS::nbRemainingCluster(uint cluster){
        int res =0;
        cluster &= 0x0FFFFFFF;

        while(cluster < 0x0FFFFFF7){
            ++res;
            cluster = getFATEntry(cluster);
        }
        assert (cluster != 0x0FFFFFF7);
        return res;
    }
    ::Directory* FS::getRoot(){
        return new Directory (this,_fmbr.RootCluster,0,nullptr);
    }
    Directory* FS::getRootFat(){
        return new Directory (this,_fmbr.RootCluster,0,nullptr);
    }

};
