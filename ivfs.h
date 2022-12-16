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


//helper functions for file parsing and writing
std::pair<std::string, FileLocation> parse_idx_line(const std::string& line);

void parse_idx(const std::string& file, std::map<std::string, FileLocation>& index);

void parse_free_chunks(const std::string& file, std::vector<size_t>& chunks);

void write_index(std::ofstream& file, std::map<std::string, FileLocation> index);

void write_free_chunks(std::ofstream& file, std::vector<size_t> free_chunks);

struct IVFS{
    std::map<const char*, std::pair<mode, size_t>> opened; //map of opened files
    std::map<std::string, FileLocation> index = {}; //map of file locations in VFS
    std::vector<size_t> free_chunks = {}; //free chunks in file
    size_t last_chunk = 0; //index of last free chunk -- overwritten we all chunks before it are full

    IVFS();

    File* Open(const char* name);

    File* Create(const char* name);

    size_t Read(File *f, char *buff, size_t len);

    size_t Write(File *f, char *buff, size_t len);

    void Close(File *f);

    void CloseIVFS();
};
}
