#ifndef TEMPLATE_UTILS_HPP
#define TEMPLATE_UTILS_HPP

#include <string>
#include <utility>
#include <vector>

bool loadTemplateFromFile(const std::string& path, std::string& out);

std::string renderTemplate(const std::string& text,
                           const std::vector< std::pair<std::string, std::string> >& vars);

#endif // TEMPLATE_UTILS_HPP
