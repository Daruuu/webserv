#include "config/ConfigException.hpp"
#include "network/ServerManager.hpp"

#include <exception>
#include <iostream>
#include <signal.h>

int main(int argc, char *argv[]) {
	((void)argc, (void)argv);

	std::cout << "Start webserver in c++98" << std::endl;

	// Disable SIGPIPE globally to prevent crashes
	// this will be for the CGI part, but already having it here doesn't hurt
	signal(SIGPIPE, SIG_IGN);

	try {
		//ConfigParser parser(argv[1]);
		//ServerConfig config = parser.parse();

		//Webserver server(config);
		//server.run();
		
		int port = 8080;

		std::cout << "Listening on port: " << port << std::endl;

		ServerManager server(port);
		server.run();


	} catch (const ConfigException& e) {

		std::cerr << "Configuration Error: " << e.what() << std::endl;
		return 1;

	} catch (std::exception& e) {

		std::cerr << "Fatal Runtime Error: " << e.what() << std::endl;
		return 1;
	}
	return 0;

}
