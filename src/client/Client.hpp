#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <netinet/in.h>
#include <ctime>

#include "../http/HttpParser.hpp"
#include "../http/HttpRequest.hpp"
#include "../http/HttpResponse.hpp"
#include "RequestProcessor.hpp"

// Representa una conexión TCP con un cliente.
// Se encarga de:
// - Acumular bytes que vienen del socket.
// - Pasarlos al HttpParser.
// - Cuando hay HttpRequest completa, invocar a RequestProcessor.
// - Acumular la respuesta serializada y enviarla por el socket.





//EL cliente no puede simplemente leer y escribir . Necesita saber que esta
//haciendo en cada momento para que el bucle rpincipal sepa escuchar eventos
//POLLIN POLLOUT 
enum ClientState { 
    STATE_IDLE,
    STATE_READING_HEADER,
    STATE_READING_BODY,
    STATE_WRITING_RESPONSE,
    STATE_CLOSED,
};


class Client {
private:
    int                 _fd;
    std::string         _inBuffer;   // datos recibidos aún sin procesar
    std::string         _outBuffer;  // respuesta lista para enviar (o parcialmente enviada)
    HttpParser          _parser;
    HttpResponse        _response;
    RequestProcessor    _processor;
    ClientState         _state;
    struct sockaddr_in  _addr;
    time_t              _lastActivity; // para gestionar timeouts

    // Invocado cuando el parser marca una HttpRequest como completa.
    void handleCompleteRequest();

public:
    Client(int fd, struct sockaddr_in addr);
    ~Client();
    

    //GETTERS
    int getFd() const;
    ClientState getState() const;
    bool needsWrite() const;
    time_t getLastActivity() const;
    

    //HANDLES
    //el motor de entrada este metodo se llamara cuando epoll te avise cuando
    //hay datos con EPOLLIN
    void handleRead(); 
    void handleWrite();

    //BUILD RESPONSE
    void buildResponse();

};

#endif // CLIENT_HPP



