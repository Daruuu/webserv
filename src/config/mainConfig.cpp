#include <iostream>
#include <fcntl.h>
#include <string>


int main(int argc, char const *argv[])
{
    if (argc == 1)
    {
        std::cout << "Use default config file store in [config/default.conf] "<< std::endl;
    }
    else if (argc == 2)
    {
		std::string fileName = argv[1];

		std::cout << "filename: [" << fileName << "]" << std::endl;

        std::string extensionFile = ".conf";

        size_t extLen = extensionFile.size();
        if (fileName.size() >= extLen &&
            fileName.compare(fileName.size() - extLen, extLen, extensionFile) == 0)
        {

		    std::cout << "valid extension file config." << std::endl;
        }

    }
    
    return 0;
}
