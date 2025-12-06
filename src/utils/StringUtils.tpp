#ifndef STRING_UTILS_TPP
#define STRING_UTILS_TPP

#include <sstream>
#include <string>

namespace string_utils {
	template <typename T>
	std::string toString(const T& value) {
		std::ostringstream oss;
		oss << value;
		return oss.str();
	}
}

#endif
