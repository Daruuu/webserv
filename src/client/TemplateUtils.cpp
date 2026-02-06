#include "TemplateUtils.hpp"

#include <fstream>

static void replaceAll(std::string& text,
                       const std::string& from,
                       const std::string& to)
{
    if (from.empty())
        return;
    std::string::size_type pos = 0;
    while ((pos = text.find(from, pos)) != std::string::npos)
    {
        text.replace(pos, from.size(), to);
        pos += to.size();
    }
}

bool loadTemplateFromFile(const std::string& path, std::string& out)
{
    std::ifstream file(path.c_str(), std::ios::in);
    if (!file.is_open())
        return false;

    out.assign((std::istreambuf_iterator<char>(file)),
               std::istreambuf_iterator<char>());
    return true;
}

std::string renderTemplate(const std::string& text,
                           const std::vector< std::pair<std::string, std::string> >& vars)
{
    std::string result = text;
    for (size_t i = 0; i < vars.size(); ++i)
        replaceAll(result, vars[i].first, vars[i].second);
    return result;
}
