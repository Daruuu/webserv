#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>

//  test using open, read, close

int test_file(int ac, char **av)
{
    if (ac != 2)
        std::cerr << "invalid number of arguments" << std::endl;
    
    int fd = open(av[1], O_RDONLY);
    if (fd == -1)
        std::cerr << "open failed: " << std::endl;
    else
    {
        std::cout << "open file successfull" << std::endl;
        char buffer[1024];
        int bytes_read = read(fd, buffer, sizeof(buffer));
        if (bytes_read == -1)
            std::cerr << "read file failed: " << std::endl;
        else
            std::cout << "read file successfull" << std::endl;
    }
    close(fd);

    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 2)

    return 0;
}