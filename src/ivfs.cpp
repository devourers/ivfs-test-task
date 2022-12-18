#include <ivfs.h>


namespace TestTask{

const size_t CHUNK_SIZE = 8; //size of chunk in file
std::mutex write_mutex; //mutex for parallel access to IVFS

//helper functions
std::pair<std::string, FileLocation> parse_idx_line(const std::string& line){
    size_t end_ = line.length() - 1;
    size_t start = 0;
    size_t end = line.find('%');
    std::string name;
    std::vector<size_t> indexes;
    size_t file_size;
    name = line.substr(start, end);
    start = end+1;
    end = line.find('%', start);
    std::string indexs = line.substr(start, end-start);
    //parse indexes to vector
    if (indexs.size() > 0){
        size_t start_v = 0;
        size_t end_v = indexs.find(' ');
        while (end_v != std::string::npos){
            indexes.push_back(static_cast<size_t>(
                                  std::stoi(indexs.substr(start_v, end_v - start_v))));
            start_v = end_v + 1;
            end_v = indexs.find(' ', start_v);
        }
        indexes.push_back(static_cast<size_t>(
                              std::stoi(indexs.substr(start_v, end_v - start_v))));
    }
    std::string length = line.substr(end+1, end_);
    file_size = static_cast<size_t>(std::stoi(length));
    FileLocation file_loc;
    file_loc.size = file_size;
    file_loc.indexes = indexes;
    return {name, file_loc};
}

void parse_idx(const std::string& file, std::map<std::string, FileLocation>& index){
    size_t start_line = 0;
    size_t end_line = file.find('\n');
    std::string cur_line;
    if (file.length() == 0 ){
        return;
    }
    while (end_line != std::string::npos){
        cur_line = file.substr(start_line, end_line - start_line);
        start_line = end_line + 1;
        std::pair<std::string, FileLocation> res = parse_idx_line(cur_line);
        index.insert({res.first, res.second});
        end_line = file.find('\n', start_line);
    }
    cur_line = file.substr(start_line, end_line - start_line);
    std::pair<std::string, FileLocation> res = parse_idx_line(cur_line);
    index.insert({res.first, res.second});
}

void parse_free_chunks(const std::string& file, std::vector<size_t>& chunks){
    size_t start_v = 0;
    size_t end_v = file.find(' ');
    if (file.size() == 0){
        return;
    }
    while (end_v != std::string::npos){
        chunks.push_back(static_cast<size_t>(
                              std::stoi(file.substr(start_v, end_v - start_v))));
        start_v = end_v + 1;
        end_v = file.find(' ', start_v);
    }
    chunks.push_back(static_cast<size_t>(
                          std::stoi(file.substr(start_v, end_v - start_v))));
}

void write_index(std::ofstream& file, std::map<std::string, FileLocation> index){
    size_t count = 0;
    for (std::pair<std::string, FileLocation> cur_line : index){
        file << cur_line.first;
        file << '%';
        if (cur_line.second.indexes.size() > 0){
            for (size_t i = 0; i < cur_line.second.indexes.size() - 1; i++){
                file << cur_line.second.indexes[i];
                file << " ";
            }
            file << cur_line.second.indexes[cur_line.second.indexes.size() - 1];
        }

        file << '%';
        file << cur_line.second.size;
        if (count != index.size() - 1){
            file << "\n";
        }
        count++;
    }
    file.close();
}

void write_free_chunks(std::ofstream& file, std::vector<size_t> free_chunks){
    if (free_chunks.size() > 0){
        for (size_t i = 0; i < free_chunks.size() - 1; i++){
            file << free_chunks[i];
            file << " ";
        }
        file << free_chunks[free_chunks.size() - 1];
    }
}

void put_chars(std::ofstream& fs, size_t& chunk_ind,
               size_t& written, size_t& len, char* buff, size_t curr_chunk){
    while (chunk_ind < 8 && written < len && written < strlen(buff)){
        size_t actual_ind = CHUNK_SIZE * curr_chunk + chunk_ind;
        fs.seekp(actual_ind);
        chunk_ind++;
        fs.put(buff[written]);
        written++;
    }
}

void padd_files(std::ofstream& fs, size_t& chunk_ind){
    while (chunk_ind < 8){
        fs.seekp(0, std::ios::cur);
        fs.put(' ');
        chunk_ind++;
    }
}

//IVFS constructor
IVFS::IVFS(){
    //IVFS -- data in contained in a single file via chunks
    /*
     * [awdadwadw...............][aawdaeqewq...........][qweqewqwe............].....
     * each chunks is 8 bytes, and idx file contatins indexes of chunks and file size
     * creating new file takes first free chunk
     * (or last_chunk+1 if no free chunks avaliable) and writes file in them.
    */
    //index file -- contains all the data in format

    // filename%chunk indexes%file size (in bytes)
    std::ifstream index_str;
    index_str.open("index.idx");

    //last_chunk.chnks -- file with single number --
    //last chunk index (if no free chunks -- write in (last_chunk +1) )
    std::ifstream lastchunk_str;
    lastchunk_str.open("last_chunk.chnks");

    //free.chnks -- file with free chunks
    std::ifstream freechunks_str;
    freechunks_str.open("free.chnks");

    //parse index.idx
    if (index_str.is_open()){
        std::stringstream idxbuff;
        idxbuff << index_str.rdbuf();
        std::string idx = idxbuff.str();
        parse_idx(idx, index);
    }
    //parse lastchunk.chnks
    if (lastchunk_str.is_open()){
        std::stringstream lstchnkbuff;
        lstchnkbuff << lastchunk_str.rdbuf();
        if (lstchnkbuff.str().length() == 0){
            last_chunk = 0;
            return;
        }
        last_chunk = static_cast<size_t>(std::stoi(lstchnkbuff.str()));
    }
    //parse free.chnks
    if (freechunks_str.is_open()){
        std::stringstream frchnksbuff;
        frchnksbuff << freechunks_str.rdbuf();
        std::string freechunks = frchnksbuff.str();
        parse_free_chunks(freechunks, free_chunks);
    }
}

//Opens file for reading if it exists, return nullptr otherwise.
File* IVFS::Open(const char* name){
    std::lock_guard<std::mutex> lock(write_mutex);
    if (index.count(name) == 0){
        return nullptr;
    }
    else if (opened.count(name) != 0 && opened[name].first == mode::writeonly){
        return nullptr;
    }
    File* res = new File();
    res->loc = name;
    res->mode_ = mode::readonly;
    res->file_pos = 0;
    res->file_size = index[std::string(name)].size;
    if (opened.count(name) != 0){
        opened[name].second++;
    }
    else{
        opened.insert({name, {mode::readonly, 1}});
    }
    return res;
}

//Creates file if none exists, and overwrites file if it existed.
//If file is opened as readonly -- return nullptr
File* IVFS::Create(const char* name){
    std::lock_guard<std::mutex> lock(write_mutex);
    if (opened.count(name) != 0){
        if (opened[name].second == mode::readonly){
            return nullptr;
        }
        else{
            //rewrite file -- move all its chunks to free, size equal to 0
            std::ofstream fs;
            for (size_t chunk_id : index[std::string(name)].indexes){
                free_chunks.push_back(chunk_id);
            }
            index[name].indexes = {};
            index[name].size = 0;
            File* f = new File();
            f->file_pos = 0;
            f->loc = name;
            f->mode_ = mode::writeonly;
            opened[name].second += 1;
            return f;
        }
    }
    else{
        if (index.count(std::string(name)) != 0){
            //rewrite file -- move all its chunks to free, size equal to 0
            for (size_t chunk_id : index[std::string(name)].indexes){
                free_chunks.push_back(chunk_id);
            }
            index[name].indexes = {};
            index[name].size = 0;
            File* f = new File();
            f->file_pos = 0;
            f->loc = name;
            f->mode_ = mode::writeonly;
            opened.insert({name, {mode::writeonly, 1}});
            return f;
        }
        else{
            FileLocation file_loc;
            file_loc.indexes = {};
            file_loc.size = 0;
            index.insert({std::string(name), file_loc});
            File* f = new File();
            f->file_pos = 0;
            f->loc = name;
            f->mode_ = mode::writeonly;
            opened.insert({name, {mode::writeonly, 1}});
            return f;
        }
    }
}

//Read function -- read len bytes to buff.
//Reading is begining from internal file pointer file_pos, which saved between reads.
size_t IVFS::Read(File *f, char *buff, size_t len){
    if (f->mode_ == mode::writeonly){
        return 0;
    }
    std::lock_guard<std::mutex> lock(write_mutex);
    std::vector<size_t> locs = index[std::string(f->loc)].indexes;
    size_t sz = index[std::string(f->loc)].size;
    std::ifstream fs;
    size_t ind = f->file_pos;
    fs.open("files.cvfs", std::ios::binary);
    char ch;
    while (ind < len && ind < sz ){
        size_t chunk_ind = ind / CHUNK_SIZE;
        size_t actual_ind = locs[chunk_ind]*CHUNK_SIZE  + (ind % CHUNK_SIZE);
        fs.seekg(actual_ind, std::ios::beg);
        fs.get(ch);
        buff[ind] = ch;
        ind++;
    }
    fs.close();
    f->file_pos = ind;
    return ind;
}

size_t IVFS::Write(File *f, char *buff, size_t len){
    std::ofstream fs;
    std::lock_guard<std::mutex> lock(write_mutex);
    if (f->mode_ == mode::readonly){
        return 0;
    }
    fs.open("files.cvfs", std::ios::out | std::ios::in | std::ios::binary);
    if (!fs.is_open()){
        fs.open("files.cvfs");
    }
    size_t buff_len = strlen(buff);
    size_t written = 0;
    while (written < len && written < buff_len){
        if (f->file_size % CHUNK_SIZE != 0){
            size_t curr_chunk = index[std::string(f->loc)].
                    indexes[index[std::string(f->loc)].indexes.size() - 1];
            size_t chunk_ind = f->file_size % CHUNK_SIZE;
            put_chars(fs, chunk_ind, written, len, buff, curr_chunk);
        }
        if (free_chunks.size() > 0){
            size_t curr_chunk = free_chunks[0];
            size_t chunk_ind = 0;
            put_chars(fs, chunk_ind, written, len, buff, curr_chunk);
            if (chunk_ind < 8){
                padd_files(fs, chunk_ind);
            }
            index[std::string(f->loc)].indexes.push_back(curr_chunk);
            free_chunks.erase(free_chunks.begin());

        }
        else{
            size_t curr_chunk = last_chunk;
            size_t chunk_ind = 0;
            put_chars(fs, chunk_ind, written, len, buff, curr_chunk);
            if (chunk_ind < 8){
                padd_files(fs, chunk_ind);
            }
            index[std::string(f->loc)].indexes.push_back(curr_chunk);
            last_chunk++;
        }
    }
    index[std::string(f->loc)].size += written;
    f->file_size += written;
    fs.close();
    return written;
}

void IVFS::Close(File *f){
    opened[f->loc].second -=1;
    if (opened[f->loc].second == 0){
        opened.erase(f->loc);
    }
    delete(f);
}

void IVFS::CloseIVFS(){

    //Close IVFS and save index, last chunk, free chunks
    std::ofstream index_str;
    index_str.open("index.idx");

    //last_chunk.chnks -- file with single number -- last chunk index
    //if no free chunks -- write in (last_chunk +1) )
    std::ofstream lastchunk_str;
    lastchunk_str.open("last_chunk.chnks");

    //free.chnks -- file with free chunks
    std::ofstream freechunks_str;
    freechunks_str.open("free.chnks");

    //write index
    write_index(index_str, index);

    //write free chunks
    write_free_chunks(freechunks_str, free_chunks);

    //write last_chunk
    lastchunk_str << last_chunk;
}

} //namespace TestTask
