#ifndef HTTP_REQUEST_HPP
#define HTTP_REQUEST_HPP

#include <string>
#include <map>

// Métodos HTTP básicos que vamos a soportar de momento.
enum HttpMethod {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST,
    HTTP_METHOD_DELETE,
    HTTP_METHOD_UNKNOWN
};

// Versión de HTTP (solo 1.0 y 1.1 para webserv).
enum HttpVersion {
    HTTP_VERSION_1_0,
    HTTP_VERSION_1_1,
    HTTP_VERSION_UNKNOWN
};

// Representa una petición HTTP ya parseada (modelo de datos, sin lógica de parseo).
class HttpRequest {
public:
    typedef std::map<std::string, std::string> HeaderMap;

private:
    HttpMethod  _method;
    HttpVersion _version;
    std::string _path;
    std::string _query;
    HeaderMap   _headers;
    std::string _body;
};

#endif // HTTP_REQUEST_HPP


