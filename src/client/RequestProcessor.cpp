#include "RequestProcessor.hpp"

static std::string httpVersionToString(HttpVersion version)
{
    if (version == HTTP_VERSION_1_0)
        return "HTTP/1.0";
    if (version == HTTP_VERSION_1_1)
        return "HTTP/1.1";
    return "HTTP/1.1";
}

HttpResponse RequestProcessor::process(const HttpRequest& request, bool parseError)
{
    HttpResponse response;
    int statusCode = HTTP_STATUS_OK;
    std::string body = "OK\n";
    bool shouldClose = request.shouldCloseConnection();

    if (parseError || request.getMethod() == HTTP_METHOD_UNKNOWN)
    {
        statusCode = HTTP_STATUS_BAD_REQUEST;
        body = "Bad Request\n";
        shouldClose = true;
    }

    response.setStatusCode(statusCode);
    response.setVersion(httpVersionToString(request.getVersion()));
    response.setHeader("Connection", shouldClose ? "close" : "keep-alive");
    response.setHeader("Content-Type", "text/plain");
    response.setBody(body);

    return response;
}
