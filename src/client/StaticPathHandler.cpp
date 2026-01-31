#include "StaticPathHandler.hpp"

#include "ErrorUtils.hpp"

#include <fstream>
#include <sys/stat.h>
#include <unistd.h>

static bool readFileToBody(const std::string& path, std::vector<char>& out)
{
    std::ifstream file(path.c_str(), std::ios::in | std::ios::binary);
    if (!file.is_open())
        return false;

    out.clear();
    char c;
    while (file.get(c))
        out.push_back(c);
    return true;
}

bool handleStaticPath(const HttpRequest& request,
                      const ServerConfig* server,
                      const LocationConfig* location,
                      const std::string& path,
                      std::vector<char>& body,
                      HttpResponse& response)
{
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
    {
        buildErrorResponse(response, request, 404, false, server);
        return true;
    }

    if (S_ISDIR(st.st_mode))
    {
        std::vector<std::string> indexes;
        if (location)
            indexes = location->getIndexes();
        if (indexes.empty() && server && !server->getIndex().empty())
            indexes.push_back(server->getIndex());
        if (indexes.empty())
            indexes.push_back("index.html");

        bool foundIndex = false;
        std::string indexPath;
        for (size_t i = 0; i < indexes.size(); ++i)
        {
            indexPath = path;
            if (!indexPath.empty() && indexPath[indexPath.size() - 1] != '/')
                indexPath += "/";
            indexPath += indexes[i];

            struct stat idx;
            if (stat(indexPath.c_str(), &idx) == 0 && S_ISREG(idx.st_mode))
            {
                foundIndex = true;
                break;
            }
        }

        if (foundIndex)
        {
            if (!readFileToBody(indexPath, body))
            {
                buildErrorResponse(response, request, 403, false, server);
                return true;
            }
            return false;
        }

        if (location && location->getAutoIndex())
        {
            // TODO: generar listado de directorio (autoindex).
            buildErrorResponse(response, request, 501, false, server);
            return true;
        }

        buildErrorResponse(response, request, 403, false, server);
        return true;
    }

    if (!S_ISREG(st.st_mode))
    {
        buildErrorResponse(response, request, 403, false, server);
        return true;
    }

    // Archivos regulares: diferenciar metodo
    if (request.getMethod() == HTTP_METHOD_POST)
    {
        buildErrorResponse(response, request, 405, false, server);
        return true;
    }
    if (request.getMethod() == HTTP_METHOD_DELETE)
    {
        if (unlink(path.c_str()) == 0)
        {
            body.clear();
            return false;
        }
        buildErrorResponse(response, request, 500, true, server);
        return true;
    }

    if (!readFileToBody(path, body))
    {
        buildErrorResponse(response, request, 403, false, server);
        return true;
    }

    return false;
}
