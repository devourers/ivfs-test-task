#include <iostream>
#include <map>
#include <stdio.h>
#include <fstream>
#include <cstring>
#include <thread>
#include <string>
#include <sstream>
#include <mutex>
#include <vector>

namespace TestTask{

enum mode{
    readonly,
    writeonly
};


struct File{
    const char* loc;
    mode mode_;
    size_t file_pos = 0;
    size_t file_size = 0;
    ~File() = default;
};


struct FileLocation{
    std::vector<size_t> indexes;
    size_t size;
};


struct IVFS{

    //map of opened files
    std::map<const char*, std::pair<mode, size_t>> opened;
    //map of file locations in VFS
    std::map<std::string, FileLocation> index = {};
    //free chunks in file
    std::vector<size_t> free_chunks = {};
    //index of last free chunk -- overwritten we all chunks before it are full
    size_t last_chunk = 0;

    IVFS();

    File* Open(
            const char* name
            );

    File* Create(
            const char* name
            );

    size_t Read(
            File *f,
            char *buff,
            size_t len
            );

    size_t Write(
            File *f,
            char *buff,
            size_t len
            );

    void Close(
            File *f
            );

    void CloseIVFS();
};

} //namespace TestTask
