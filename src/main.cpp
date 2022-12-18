#include <iostream>
#include <ivfs.h>

void test3_read_func(TestTask::IVFS& ivfs, TestTask::File* f, char* buff_read){
    size_t err = ivfs.Read(f, buff_read, 100);
    std::cerr << err << std::endl; //needed in order for windows to call function in release
}

void test3_write_func(TestTask::IVFS& ivfs, TestTask::File* f){
    size_t err = ivfs.Write(f, "I was waiting for mutex...\n", 100);
    std::cerr << err << std::endl; //needed in order for windows to call function in release
}

int main()
{
    std::ios_base::sync_with_stdio(false);
    TestTask::IVFS ivfs = TestTask::IVFS();

    { /* Test 1 */
        TestTask::File* savefile = ivfs.Create("savefile.txt");
        size_t written_health = ivfs.Write(savefile, "health: 10\n", 11);
        TestTask::File* helloworld = ivfs.Create("helloworld.txt");
        size_t written_helloword = ivfs.Write(helloworld, "Hello World!\n", 25);
        size_t written_ammo = ivfs.Write(savefile, "ammo: 1\n", 8);
        ivfs.Close(savefile);
        ivfs.Close(helloworld);


        std::cout << "~~~~~TESTS~~~~~~" << std::endl;
        TestTask::File* opened_savefile = ivfs.Open("savefile.txt");
        char buff[128] = {0};
        size_t read_bytes = ivfs.Read(opened_savefile, buff, 30);
        std::cout << "Test 1:" << std::endl << "read from savefile.txt " << std::endl <<
                     buff << std::endl << "read bytes: " << read_bytes<< std::endl <<
                     "written bytes: " << written_ammo + written_health << std::endl;
        TestTask::File* opened_helloworld = ivfs.Open("helloworld.txt");
        char buff_2[128] = {0};
        size_t read_bytes_2 = ivfs.Read(opened_helloworld, buff_2, 30);
        std::cout << "read from helloworld.txt " << std::endl <<
                     buff_2 << std::endl << "read bytes: " << read_bytes_2 << std::endl <<
                     "written bytes: " << written_helloword << std::endl;
        ivfs.Close(opened_helloworld);
        ivfs.Close(opened_savefile);
        std::cout << "----" << std::endl;
    }

    { /* Test 2 */
        TestTask::File* overwrite = ivfs.Create("savefile.txt");
        ivfs.Close(overwrite);
        TestTask::File* inpalce_of_overwritten = ivfs.Create("new_savefile.txt");
        size_t written_bytes= ivfs.Write(inpalce_of_overwritten,
                                         "Here is new file.\nIt's bigger.\tMuch bigger.",
                                         100);
        ivfs.Close(inpalce_of_overwritten);
        TestTask::File* read_overwrite = ivfs.Open("new_savefile.txt");
        char buff[128] = {0};
        size_t read_bytes = ivfs.Read(read_overwrite, buff, 100);
        std::cout << "Test 2: write and read to new_savefile.txt inplace of savefile.txt: " <<
                     std::endl << buff << std::endl << "bytes written: " << written_bytes <<
                     std::endl << "bytes read: " << read_bytes << std::endl;
        ivfs.Close(read_overwrite);
        std::cout << "----" << std::endl;
    }

    { /* Test 3 */
        TestTask::File* open_helloworld = ivfs.Open("helloworld.txt");
        char buff_read[128] = {0};
        TestTask::File* create_new_file = ivfs.Create("mutex.txt");
        char buff_read_write[128] = {0};
        std::cout << "Test 3: parallel access to IVFS" << std::endl;
        std::thread t1(test3_read_func, std::ref(ivfs), open_helloworld, buff_read);
        std::thread t2(test3_write_func, std::ref(ivfs), create_new_file);
        t1.join();
        t2.join();
        std::cout << "Read from thread 1: " << std::endl << buff_read << std::endl;
        ivfs.Close(create_new_file);
        TestTask::File* reopen = ivfs.Open("mutex.txt");
        size_t test3_read = ivfs.Read(reopen, buff_read_write, 100);
        std::cout << "Wrote in thread 2: " << std::endl << buff_read_write << std::endl
                  << "read bytes: " << test3_read << std::endl;
        ivfs.Close(reopen);
        ivfs.Close(open_helloworld);
    }
    ivfs.CloseIVFS();
    return 0;
}
