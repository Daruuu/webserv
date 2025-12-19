#ifndef HTTP_PARSER_HPP
#define HTTP_PARSER_HPP

#include <string>

#include "HttpRequest.hpp"

// Parser incremental de HTTP.
// No sabe nada de sockets: solo consume texto y construye un HttpRequest.
class HttpParser {
public:
    enum State {
        PARSING_START_LINE,
        PARSING_HEADERS,
        PARSING_BODY,
        COMPLETE,
        ERROR
    };

private:
    State       _state;
    HttpRequest _request;
    std::string _buffer; // acumulador interno para líneas/body pendientes
};

#endif // HTTP_PARSER_HPP


