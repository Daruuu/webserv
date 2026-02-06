#include "ErrorUtils.hpp"
#include "ResponseUtils.hpp"
#include "TemplateUtils.hpp"

#include <sstream>

static std::string statusMessage(int code)
{
    if (code == 400) return "Bad Request";
    if (code == 403) return "Forbidden";
    if (code == 404) return "Not Found";
    if (code == 405) return "Method Not Allowed";
    if (code == 413) return "Payload Too Large";
    if (code == 500) return "Internal Server Error";
    if (code == 501) return "Not Implemented";
    if (code == 502) return "Bad Gateway";
    if (code == 504) return "Gateway Timeout";
    return "Error";
}

void buildErrorResponse(HttpResponse& response,
                        const HttpRequest& request,
                        int statusCode,
                        bool shouldClose,
                        const ServerConfig* server)
{
    // TODO: si hay error_page configurada en server, usarla aqui.
    // TODO: leer archivo de error y usarlo como body.
    (void)server;
    std::string tpl;
    if (loadTemplateFromFile("./www/templates/error.html", tpl))
    {
        std::ostringstream codeStr;
        codeStr << statusCode;
        std::vector< std::pair<std::string, std::string> > vars;
        vars.push_back(std::make_pair("{{CODE}}", codeStr.str()));
        vars.push_back(std::make_pair("{{MESSAGE}}", statusMessage(statusCode)));
        std::string html = renderTemplate(tpl, vars);
        fillBaseResponse(response, request, statusCode, shouldClose, toBody(html));
        response.setHeader("Content-Type", "text/html");
        return;
    }
    fillBaseResponse(response, request, statusCode, shouldClose, toBody("Error\n"));
}
