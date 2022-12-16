#include <iostream>
#include <ivfs.h>

void test3_read_func(TestTask::IVFS& ivfs, TestTask::File* f, char* buff_read){
    ivfs.Read(f, buff_read, 100);
}

void test3_write_func(TestTask::IVFS& ivfs, TestTask::File* f){
    ivfs.Write(f, "I was waiting for mutex...\n", 100);
}

int main()
{
    TestTask::IVFS ivfs = TestTask::IVFS();


    TestTask::File* savefile = ivfs.Create("savefile.txt");
    ivfs.Write(savefile, "health: 10\n", 11);
    TestTask::File* helloworld = ivfs.Create("helloworld.txt");
    ivfs.Write(helloworld, "Hello World!\n", 25);
    ivfs.Write(savefile, "ammo: 1\n", 8);
    ivfs.Close(savefile);
    ivfs.Close(helloworld);

    /* Test 1 */
    std::cout << "~~~~~TESTS~~~~~~" << std::endl;
    TestTask::File* opened_savefile = ivfs.Open("savefile.txt");
    char buff_test1[128];
    size_t read_bytes_test1 = ivfs.Read(opened_savefile, buff_test1, 30);
    std::cout << "Test 1:" << std::endl << "read from savefile.txt " << std::endl <<
                 buff_test1 << std::endl << "bytes: " << read_bytes_test1 << std::endl;
    std::cout << "-----" << std::endl;
    TestTask::File* opened_helloworld = ivfs.Open("helloworld.txt");
    char buff_test2[128];
    size_t read_bytes_test2 = ivfs.Read(opened_helloworld, buff_test2, 30);
    std::cout << "read from helloworld.txt " << std::endl <<
                 buff_test2 << std::endl << "bytes: " << read_bytes_test2 << std::endl;
    ivfs.Close(opened_helloworld);
    ivfs.Close(opened_savefile);

    /* Test 2 */
    TestTask::File* overwrite = ivfs.Create("savefile.txt");
    ivfs.Close(overwrite);
    TestTask::File* inpalce_of_overwritten = ivfs.Create("new_savefile.txt");
    size_t written_bytes_test3 = ivfs.Write(inpalce_of_overwritten,
                                            "Here is new file.\nIt's bigger.\tMuch bigger.", 100);
    ivfs.Close(inpalce_of_overwritten);
    TestTask::File* read_overwrite = ivfs.Open("new_savefile.txt");
    char buff_test3[128];
    size_t read_bytes_test3 = ivfs.Read(read_overwrite, buff_test3, 100);
    std::cout << "Test 2: write and read to new_savefile.txt inplace of savefile.txt: " <<
                 std::endl << buff_test3 << std::endl << "bytes written: " << written_bytes_test3 <<
                 std::endl << "bytes read: " << read_bytes_test3 << std::endl;
    ivfs.Close(read_overwrite);

    /* Test 3 */
    TestTask::File* test3_open_helloworld = ivfs.Open("helloworld.txt");
    char buff_read[128];
    TestTask::File* test3_create_new_file = ivfs.Create("mutex.txt");
    char buff_read_write[128];
    std::cout << "Test 3: parallel access to IVFS" << std::endl;
    std::thread t1(test3_read_func, std::ref(ivfs), test3_open_helloworld, buff_read);
    std::thread t2(test3_write_func, std::ref(ivfs), test3_create_new_file);
    t1.join();
    t2.join();
    std::cout << "Read from thread 1: " << std::endl << buff_read << std::endl;
    ivfs.Close(test3_create_new_file);
    TestTask::File* reopen = ivfs.Open("mutex.txt");
    ivfs.Read(reopen, buff_read_write, 100);
    std::cout << "Wrote in thread 2: " << std::endl << buff_read_write << std::endl;
    ivfs.Close(reopen);
    ivfs.CloseIVFS();


    return 0;
}
