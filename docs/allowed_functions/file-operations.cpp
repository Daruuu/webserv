#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

//  test using open, read, close

int test_file(int ac, char **av)
{
    if (ac != 2)
        std::cerr << "invalid number of arguments" << std::endl;

    std::string file_name = av[1];
    int fd = open(file_name.c_str(), O_RDONLY);

    if (fd == -1)
        std::cerr << "open failed: " << "[" << file_name << "]" << std::endl;

    std::cout << "open file successfull" << std::endl;
    char buffer[1024];
    int bytes_read = read(fd, buffer, sizeof(buffer));
    if (bytes_read == -1)
        std::cerr << "read file failed: " << std::endl;
    else
    {
        std::cout << "read file successfull" << std::endl;
        std::cout << buffer << std::endl;
        std::cout << "bytes read: " << bytes_read << std::endl;
        std::cout << "file size: " << bytes_read << std::endl;

    }
    close(fd);
    return 0;
}

int main(int argc, char **argv)
{
    test_file(argc, argv);

    return 0;
}
