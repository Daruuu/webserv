#include <exception>
#include <iostream>
#include <csignal>
#include "config/ConfigParser.hpp"

int main(int argc, char* argv[])
{
	((void)argc, (void)argv);

	// Disable SIGPIPE globally to prevent crashes
	// this will be for the CGI part, but already having it here doesn't hurt
	signal(SIGPIPE, SIG_IGN);

	std::cout << "Esto se pone interesante. :))" << std::endl;

	try
	{
		// load configuration into webserver
		// execute webserver
	}
	catch (std::exception& e)
	{
		// ooh fuck! something wrong happend
		// you can do it better next time :)
	}
	return 0;
}
