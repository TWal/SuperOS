#ifndef __SUPOS_UNISTD_H
#define __SUPOS_UNISTD_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


    struct timespec {
        uint32_t tv_sec;  ///< seconds
        uint32_t tv_nsec; ///< nanoseconds
    };

    /**
        @brief Represents a directory entry

        It is used by Directory::read
    */
    struct dirent {
        uint32_t d_ino;        ///< inode
        char d_name[256]; ///< name
    };

    /**
        @brief Represents the status of a file/directory

        It is used in File::getStatus
    */
    struct stat {
        uint32_t    st_dev;     ///< ID of device containing file
        uint32_t    st_ino;     ///< inode number
        uint16_t    st_mode;    ///< file type and mode
        uint16_t    st_nlink;   ///< number of hard links
        uint16_t    st_uid;     ///< user ID of owner
        uint16_t    st_gid;     ///< group ID of owner
        uint32_t    st_rde;     ///< device ID (if special file) //TODO: what is this?
        size_t st_size;         ///< total size, in bytes
        uint32_t    st_blksize; ///< blocksize for filesystem I/O
        uint32_t    st_blocks;  ///< number of 512B blocks allocated

        struct timespec st_atim;  ///< time of last access
        struct timespec st_mtim;  ///< time of last modification
        struct timespec st_ctim;  ///< time of last status change
        // Backward compatibility
#define st_atime st_atim.tv_sec
#define st_mtime st_mtim.tv_sec
#define st_ctime st_ctim.tv_sec
    };

    enum FileMode {
        S_IFMT = 0xF000,   ///< format mask
        S_IFSOCK = 0xA000, ///< socket
        S_IFLNK = 0xC000,  ///< symbolic link
        S_IFREG = 0x8000,  ///< regular file
        S_IFBLK = 0x6000,  ///< block device
        S_IFDIR = 0x4000,  ///< directory
        S_IFCHR = 0x2000,  ///< character device
        S_IFIFO = 0x1000,  ///< fifo

        S_ISUID = 0x0800,  ///< SUID
        S_ISGID = 0x0400,  ///< SGID
        S_ISVTX = 0x0200,  ///< sticky bit

        S_IRWXU = 0x01C0,  ///< user mask
        S_IRUSR = 0x0100,  ///< read
        S_IWUSR = 0x0080,  ///< write
        S_IXUSR = 0x0040,  ///< execute

        S_IRWXG = 0x0038,  ///< group mask
        S_IRGRP = 0x0020,  ///< read
        S_IWGRP = 0x0010,  ///< write
        S_IXGRP = 0x0008,  ///< execute

        S_IRWXO = 0x0007,  ///< other mask
        S_IROTH = 0x0004,  ///< read
        S_IWOTH = 0x0002,  ///< write
        S_IXOTH = 0x0001   ///< execute
    };

#define S_ISREG(m)  (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)  (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)  (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)  (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m) (((m) & S_IFMT) == S_IFIFO)
#define S_ISLNK(m)  (((m) & S_IFMT) == S_IFLNK)
#define S_ISSOCK(m) (((m) & S_IFMT) == S_IFSOCK)

    typedef long long int off_t;

    enum{O_RDONLY = 1, O_WRONLY = 2, O_RDWR = 3, O_CREAT = 4, O_TRUNC = 8, O_APPEND = 16};

    typedef unsigned short pid_t;

    size_t read(int fd, void* buf, size_t count);
    size_t write(int fd, const void* buf, size_t count);

    int open(const char* path, int flags);
    int close(int fd);

    enum{POLLIN, POLLERR, POLLEOF, POLLNVAL};
    typedef struct {
        int fd;
        int ret;
    } pollfd;

    int poll(pollfd* fds, int nfds);

    enum{SEEK_SET, SEEK_CUR, SEEK_END};
    off_t seek(int fd, off_t offset, int whence);

    int pipe(int* fd2);

    int dup(int oldfd);
    int dup2(int oldfd, int newfd);

    pid_t fork();
    pid_t clone(void(*func)(void),void* stackEnd);
    pid_t waitpid(pid_t p, int* status);
    pid_t wait(int* status);

    int exec(char* path, char** argv);

    void _exit(int status);
    void _texit(int status);

    int opend(const char* path);
    int readd(int fd, struct dirent* buf);

    int chdir(const char* path);
    int mkdir(const char* path);

    int rmdir(const char* path);
    int link(const char* path1, const char* path2);
    int unlink(const char* path);

    int rename(const char* oldpath, const char* newpath);

#ifdef __cplusplus
}
#endif

#endif
