#include "ConfigParser.hpp"

#include <cstdlib>

#include "LocationConfig.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include "ConfigException.hpp"
#include "ConfigUtils.hpp"
#include "../common/namespaces.hpp"

ConfigParser::ConfigParser() : servers_count_(0)
{
}

ConfigParser::ConfigParser(const std::string& configFile) :
	config_file_path_(configFile), servers_count_(0U)
{
}

ConfigParser::~ConfigParser()
{
}

//	============= PRIVATE CONSTRUCTORS ===============

ConfigParser::ConfigParser(const ConfigParser& other) :
	config_file_path_(other.config_file_path_),
	servers_count_(other.servers_count_),
	raw_server_blocks_(other.raw_server_blocks_)
{
	// servers_ = other.servers_;
}

ConfigParser& ConfigParser::operator=(const ConfigParser& other)
{
	if (this != &other)
	{
		ConfigParser tmp(other);
		std::swap(config_file_path_, tmp.config_file_path_);
		std::swap(servers_count_, tmp.servers_count_);
		// std::swap(servers_, tmp.servers_);
	}
	return *this;
}

//	Getters and Setters

std::string& ConfigParser::getConfigFilePath()
{
	return config_file_path_;
}

size_t ConfigParser::getServerCount() const
{
	return servers_count_;
}

const std::vector<ServerConfig>& ConfigParser::getServers() const
{
	return servers_;
}

void ConfigParser::exportToLogFile(std::string fileContent, std::string pathToExport)
{
	std::ofstream log((pathToExport.data()));
	log << fileContent;
	log.close();
}

/**
 * main function of parsing
 *
 */
void ConfigParser::parse()
{
	if (!ValidateFileExtension())
	{
		throw ConfigException(
			config::errors::invalid_extension + config_file_path_);
	}
	else
	{
		std::cout << "VALID FILE EXTENSION: ✅\n";
	}
	if (!ValidateFilePermissions())
	{
		throw ConfigException(
			config::errors::cannot_open_file + config_file_path_);
	}
	else
	{
		std::cout << "VALID FILE PERMISSIONS: ✅\n";
	}

	clean_file_str_ = CleanFileConfig();
	exportToLogFile(clean_file_str_, config::paths::log_file_config);

	// std::cout << "CLEANFILESTR in PARSE:\n" << clean_file_str_.c_str();

	//TODO: need to fix error order of brackets: '} {' should be error but now is not a error.
	if (!ValidateCurlyBrackets())
	{
		throw ConfigException(
			"Invalid number of curly brackets " + config_file_path_);
	}
	else
	{
		std::cout << "VALID CURLY BRACKETS PAIRS: ✅\n";
	}
	
	MachineStatesOfConfigFile();
	parserServerBlocks();
}

/**
 * manage if config_file_path_:
 * has valid size of length
 * has extension '.conf'
 * @return true or false
 */
bool ConfigParser::ValidateFileExtension() const
{
	if (config_file_path_.size() < 5 || config_file_path_.substr(
		config_file_path_.size() - 5) != config::paths::extension_file)
	{
		return false;
	}
	return true;
}

bool ConfigParser::ValidateFilePermissions() const
{
	std::ifstream ifs(config_file_path_.c_str());
	if (!ifs.is_open())
		return false;
	ifs.close();
	return true;
}

/**
 * RemoveComments(): if line start with '#' skip line
 * TrimLine(): if line find "\t\n\r" remove character
 * NormalizeSpaces(): replace 'X' spaces for one space ' '
 * iterate through each line of file.
 * @return
 */
std::string ConfigParser::CleanFileConfig() const
{
	std::ifstream ifs(config_file_path_.c_str());
	if (!ifs.is_open())
	{
		throw ConfigException(
			config::errors::cannot_open_file + config_file_path_ +
			" in CleanFileConfig()");
	}

	std::ostringstream logBuffer;
	std::string line;
	// size_t lineNumber = 0;

	while (std::getline(ifs, line))
	{
		// ++lineNumber;
		config::utils::removeComments(line);
		line = config::utils::trimLine(line);
		line = config::utils::normalizeSpaces(line);
		if (line.empty())
			continue;
		// logBuffer << "|" << lineNumber << "|" << line << "\n";
		logBuffer << line << "\n";
	}
	ifs.close();
	return logBuffer.str();
}

/**
* Usar un contador dinámico (incrementa con {, decrementa con }) que nunca baje de 0
* Detectar { y }solo cuando son estructurales (inicio/fin de bloque, no dentro de valores)
* Manejar casos como:
* server { (abre)
* } (cierra)
* location /path { (abre)
* Cierres prematuros

 * @return true solo si:
 * Contador nunca negativo durante la lectura
 * Contador == 0 al final
 * Mejorar mensajes de error (línea + descripción)
 * Opcional: devolver también el nivel máximo de anidamiento o lista de errores
 */
bool ConfigParser::ValidateCurlyBrackets() const
{
	int countBrackets = 0;

	for (size_t Index = 0; Index < clean_file_str_.size(); ++Index)
	{
		if (clean_file_str_.at(Index) == '{')
		{
			++countBrackets;
		}
		else if (clean_file_str_.at(Index) == '}')
		{
			--countBrackets;
			if (countBrackets < 0)
			{
				return false;
			}
		}
	}
	return countBrackets == 0;
}

/**
 * la idea es que dependiendo de que estado se encuentre se actualize el enum,
 * asi saber cuando esta en un bloque de server o location o fuera de bloque
 *
 * extract all blocks 'server'
 * the function extractServerBlock() search all the occurrences to fill the vector
 * raw_server_block_
 */
void ConfigParser::MachineStatesOfConfigFile()
{
	if (clean_file_str_.empty())
		return;

	extractServerBlock(clean_file_str_, "server");
}

/**
 * Extracts all server { } blocks from the config content.
 * Handles nested braces within location blocks.
 * @param content The entire config file content
 * @param typeOfExtraction
 */
void ConfigParser::extractServerBlock(const std::string& content,
									const std::string& typeOfExtraction)
{
	size_t currentPos = 0;
	while ((currentPos = content.find(typeOfExtraction, currentPos)) !=
		std::string::npos)
	{
		size_t braceStart = content.find('{', currentPos);
		if (braceStart == std::string::npos)
			break;

		// Find matching closing brace
		int braceCount = 1;
		size_t braceEnd = braceStart + 1;

		while (braceEnd < content.size() && braceCount > 0)
		{
			if (content[braceEnd] == '{')
			{
				braceCount++;
			}
			else if (content[braceEnd] == '}')
			{
				braceCount--;
			}
			braceEnd++;
		}

		// Extract the complete server block
		std::string getBlock = content.
			substr(currentPos, braceEnd - currentPos);


		raw_server_blocks_.push_back(getBlock);
		currentPos = braceEnd;
	}

	servers_count_ = raw_server_blocks_.size();
}

void ConfigParser::parserServerBlocks()
{
	for (size_t i = 0; i < raw_server_blocks_.size(); ++i)
	{
		ServerConfig server = parseServerBlock(raw_server_blocks_[i]);
		servers_.push_back(server);
		// std::cout << "Parsing Block " << i + 1 << " [OK]\n";
	}
}

ServerConfig ConfigParser::parseServerBlock(const std::string& blockContent)
{
	ServerConfig server;
	std::stringstream ss(blockContent);
	std::string line;

	while (getline(ss, line))
	{
		// 1. basic clean
		line = config::utils::trimLine(line);
		if (line.empty() || line[0] == '#')
			continue;

		// 2. Tokenization
		std::vector<std::string> tokens;
		std::string directive = tokens[0];

		// 3. Dispatcher (Decidir qué hacer)
		if (directive == config::section::listen)
		{
			server.setPort(atoi(tokens[1].c_str()));
		}
		else if (directive == config::section::host)
		{
			//	remove ';'
			server.setHost(config::utils::removeSemicolon(tokens[1]));
		}
		else if (directive == config::section::error_page)
		{
			// Lógica especial para múltiples códigos de error
			// error_page 404 500 /error.html;
			// error_page 404 /404.html;
			// error_page 500 502 503 504 /50x.html;
			if (tokens.size() >= 3)
			{
				std::string path = config::utils::removeSemicolon(tokens.back());
				for (size_t i = 1; i < tokens.size() - 1; ++i)
				{
					server.addErrorPage(std::atoi(tokens[i].c_str()), path);
				}
			}
		}
		else if (directive == config::section::location)
		{
			std::string locationPath = tokens[1];
			LocationConfig loc;
			loc.setPath(locationPath);

			while (std::getline(ss, line))
			{
				config::utils::removeComments(line);
				line = config::utils::trimLine(line);
				if (!line.empty())
				{
					std::vector<std::string> locTokens = config::utils::split(
						line, ' ');
					if (locTokens.empty())
					{
						continue;
					}
					if (locTokens[0] == "}")
					{
						break; // End of location block
					}
					if (locTokens[0] == config::section::root)
					{
						loc.setRoot(
							config::utils::removeSemicolon(locTokens[1]));
					}
					else if (locTokens[0] == config::section::index)
					{
						for (size_t i = 1; i < locTokens.size(); ++i)
							loc.addIndex(
								config::utils::removeSemicolon(locTokens[i]));
					}
					else if (locTokens[0] == config::section::autoindex)
					{
						std::string val = config::utils::removeSemicolon(
							locTokens[1]);
						loc.setAutoIndex(val == config::section::autoindexOn);
					}
					else if (locTokens[0] == "methods" || locTokens[0] ==
						"allow_methods")
					{
						for (size_t i = 1; i < locTokens.size(); ++i)
							loc.addMethod(
								config::utils::removeSemicolon(locTokens[i]));
					}
					else if (locTokens[0] == config::section::returnStr) // redirection
					{
						// simple support: return 301 /url;
						if (locTokens.size() >= 3)
							loc.setRedirect(
								config::utils::removeSemicolon(locTokens[2]));
					}
					else if (locTokens[0] == config::section::uploadStore)
					{
						loc.setUploadStore(
							config::utils::removeSemicolon(locTokens[1]));
					}
				}
			}
			server.addLocation(loc);
		}
	}
	return server;
}
