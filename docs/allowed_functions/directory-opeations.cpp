#include <iostream>
#include <cerrno>
#include <unistd.h>
#include <dirent.h>

int test_directory(int ac, char **av)
{
    if (ac != 2)
        std::cerr << "invalid number of arguments" << std::endl;
    
    std::string path = av[1];
    if (path.back() != '/')
        path += '/';
    
    DIR *dir = opendir(path.c_str());
    if (dir == NULL)
        std::cerr << "opendir failed: " << std::endl;
    else
    {
        std::cout << "opendir successfull" << std::endl;
        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL)
            std::cout << entry->d_name << std::endl;
        closedir(dir);
    }
    return 0;
}