#ifndef HTTP_RESPONSE_HPP
#define HTTP_RESPONSE_HPP

#include <string>
#include <map>

#include "HttpRequest.hpp" // para reutilizar HttpVersion

// Códigos de estado mínimos para empezar.
enum HttpStatusCode {
    HTTP_STATUS_OK = 200,
    HTTP_STATUS_BAD_REQUEST = 400,
    HTTP_STATUS_NOT_FOUND = 404,
    HTTP_STATUS_METHOD_NOT_ALLOWED = 405,
    HTTP_STATUS_INTERNAL_SERVER_ERROR = 500
};

// Representa una respuesta HTTP que se enviará al cliente.
class HttpResponse {
public:
    typedef std::map<std::string, std::string> HeaderMap;

private:
    HttpStatusCode _status;
    HttpVersion    _version;
    HeaderMap      _headers;
    std::string    _body;
};

#endif // HTTP_RESPONSE_HPP


