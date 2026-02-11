#include "../../lib/catch2/catch.hpp"
#include "../../src/config/ConfigParser.hpp"
#include "../../src/config/ConfigUtils.hpp"
#include "../../src/config/LocationConfig.hpp"
#include "../../src/config/ServerConfig.hpp"
#include <cstdlib>
#include <ctime>
#include <unistd.h>

// ============================================================================
// VALIDATION 1: IP/Host Format in listen directive
// ============================================================================

TEST_CASE("config::utils::isValidIPv4 - valid IPv4 addresses",
          "[config][validation][ip]") {
  SECTION("Standard loopback") {
    REQUIRE(config::utils::isValidIPv4("127.0.0.1") == true);
  }

  SECTION("All zeros") { REQUIRE(config::utils::isValidIPv4("0.0.0.0") == true); }

  SECTION("Private network") {
    REQUIRE(config::utils::isValidIPv4("192.168.1.1") == true);
  }

  SECTION("Maximum valid IP") {
    REQUIRE(config::utils::isValidIPv4("255.255.255.255") == true);
  }

  SECTION("Public IP example") {
    REQUIRE(config::utils::isValidIPv4("8.8.8.8") == true);
  }
}

TEST_CASE("config::utils::isValidIPv4 - invalid IPv4 addresses",
          "[config][validation][ip]") {
  SECTION("Octet out of range - 256") {
    REQUIRE(config::utils::isValidIPv4("256.1.1.1") == false);
  }

  SECTION("Octet out of range - 999") {
    REQUIRE(config::utils::isValidIPv4("192.168.1.999") == false);
  }

  SECTION("Negative octet") {
    REQUIRE(config::utils::isValidIPv4("192.168.-1.1") == false);
  }

  SECTION("Too few octets") {
    REQUIRE(config::utils::isValidIPv4("192.168.1") == false);
  }

  SECTION("Too many octets") {
    REQUIRE(config::utils::isValidIPv4("192.168.1.1.1") == false);
  }

  SECTION("Empty string") { REQUIRE(config::utils::isValidIPv4("") == false); }

  SECTION("Not a number") {
    REQUIRE(config::utils::isValidIPv4("abc.def.ghi.jkl") == false);
  }

  SECTION("Mixed valid and invalid") {
    REQUIRE(config::utils::isValidIPv4("192.168.1.abc") == false);
  }
}

TEST_CASE("config::utils::isValidHostname - valid hostnames",
          "[config][validation][hostname]") {
  SECTION("localhost") {
    REQUIRE(config::utils::isValidHostname("localhost") == true);
  }

  SECTION("Simple domain") {
    REQUIRE(config::utils::isValidHostname("example.com") == true);
  }

  SECTION("Subdomain") {
    REQUIRE(config::utils::isValidHostname("www.example.com") == true);
  }

  SECTION("With hyphens") {
    REQUIRE(config::utils::isValidHostname("my-server.local") == true);
  }

  SECTION("Multi-level subdomain") {
    REQUIRE(config::utils::isValidHostname("api.v1.example.com") == true);
  }

  SECTION("Numbers in hostname") {
    REQUIRE(config::utils::isValidHostname("server1.example.com") == true);
  }
}

TEST_CASE("config::utils::isValidHostname - invalid hostnames",
          "[config][validation][hostname]") {
  SECTION("Empty string") {
    REQUIRE(config::utils::isValidHostname("") == false);
  }

  SECTION("Starting with hyphen") {
    REQUIRE(config::utils::isValidHostname("-example.com") == false);
  }

  SECTION("Ending with hyphen") {
    REQUIRE(config::utils::isValidHostname("example-.com") == false);
  }

  SECTION("Double dot") {
    REQUIRE(config::utils::isValidHostname("example..com") == false);
  }

  SECTION("Starting with dot") {
    REQUIRE(config::utils::isValidHostname(".example.com") == false);
  }

  SECTION("Ending with dot") {
    REQUIRE(config::utils::isValidHostname("example.com.") == false);
  }

  SECTION("Special characters") {
    REQUIRE(config::utils::isValidHostname("example@domain.com") == false);
  }

  SECTION("Spaces") {
    REQUIRE(config::utils::isValidHostname("my server.com") == false);
  }
}

TEST_CASE("config::utils::isValidHost - combined IP and hostname validation",
          "[config][validation][host]") {
  SECTION("Valid IPv4") {
    REQUIRE(config::utils::isValidHost("127.0.0.1") == true);
    REQUIRE(config::utils::isValidHost("192.168.1.1") == true);
  }

  SECTION("Valid hostname") {
    REQUIRE(config::utils::isValidHost("localhost") == true);
    REQUIRE(config::utils::isValidHost("example.com") == true);
  }

  SECTION("Invalid IPv4") {
    REQUIRE(config::utils::isValidHost("256.1.1.1") == false);
  }

  SECTION("Invalid hostname") {
    REQUIRE(config::utils::isValidHost("-invalid.com") == false);
  }

  SECTION("Empty string") { REQUIRE(config::utils::isValidHost("") == false); }
}

// ============================================================================
// VALIDATION 2: Port Range (testing existing implementation)
// ============================================================================

TEST_CASE("ServerConfig::setPort - valid port range",
          "[config][validation][port]") {
  ServerConfig server;

  SECTION("Minimum port") {
    REQUIRE_NOTHROW(server.setPort(1));
    REQUIRE(server.getPort() == 1);
  }

  SECTION("Standard HTTP port") {
    REQUIRE_NOTHROW(server.setPort(80));
    REQUIRE(server.getPort() == 80);
  }

  SECTION("Common dev port") {
    REQUIRE_NOTHROW(server.setPort(8080));
    REQUIRE(server.getPort() == 8080);
  }

  SECTION("Maximum port") {
    REQUIRE_NOTHROW(server.setPort(65535));
    REQUIRE(server.getPort() == 65535);
  }
}

TEST_CASE("ServerConfig::setPort - invalid port range",
          "[config][validation][port]") {
  ServerConfig server;

  SECTION("Port zero") { REQUIRE_THROWS(server.setPort(0)); }

  SECTION("Negative port") { REQUIRE_THROWS(server.setPort(-1)); }

  SECTION("Port above maximum") { REQUIRE_THROWS(server.setPort(65536)); }

  SECTION("Large invalid port") { REQUIRE_THROWS(server.setPort(100000)); }
}

// ============================================================================
// VALIDATION 3: Location Path Format
// ============================================================================

TEST_CASE("config::utils::isValidLocationPath - valid paths",
          "[config][validation][location]") {
  SECTION("Root path") {
    REQUIRE(config::utils::isValidLocationPath("/") == true);
  }

  SECTION("Simple path") {
    REQUIRE(config::utils::isValidLocationPath("/api") == true);
  }

  SECTION("Nested path") {
    REQUIRE(config::utils::isValidLocationPath("/api/v1/users") == true);
  }

  SECTION("Path with file") {
    REQUIRE(config::utils::isValidLocationPath("/index.html") == true);
  }

  SECTION("Path with special characters") {
    REQUIRE(config::utils::isValidLocationPath("/uploads_files") == true);
  }
}

TEST_CASE("config::utils::isValidLocationPath - invalid paths",
          "[config][validation][location]") {
  SECTION("Empty path") {
    REQUIRE(config::utils::isValidLocationPath("") == false);
  }

  SECTION("No leading slash") {
    REQUIRE(config::utils::isValidLocationPath("api") == false);
  }

  SECTION("Leading whitespace") {
    REQUIRE(config::utils::isValidLocationPath(" /api") == false);
  }

  SECTION("Trailing whitespace") {
    REQUIRE(config::utils::isValidLocationPath("/api ") == false);
  }

  SECTION("Double slash") {
    REQUIRE(config::utils::isValidLocationPath("//api") == false);
  }

  SECTION("Trailing double slash") {
    REQUIRE(config::utils::isValidLocationPath("/api//") == false);
  }
}

// ============================================================================
// VALIDATION 5: HTTP Methods Whitelist
// ============================================================================

TEST_CASE("config::utils::isValidHttpMethod - valid methods",
          "[config][validation][methods]") {
  SECTION("GET method") {
    REQUIRE(config::utils::isValidHttpMethod("GET") == true);
  }

  SECTION("POST method") {
    REQUIRE(config::utils::isValidHttpMethod("POST") == true);
  }

  SECTION("DELETE method") {
    REQUIRE(config::utils::isValidHttpMethod("DELETE") == true);
  }
}

TEST_CASE("config::utils::isValidHttpMethod - invalid methods",
          "[config][validation][methods]") {
  SECTION("PUT method not allowed") {
    REQUIRE(config::utils::isValidHttpMethod("PUT") == false);
  }

  SECTION("PATCH method not allowed") {
    REQUIRE(config::utils::isValidHttpMethod("PATCH") == false);
  }

  SECTION("Lowercase get") {
    REQUIRE(config::utils::isValidHttpMethod("get") == false);
  }

  SECTION("Mixed case") {
    REQUIRE(config::utils::isValidHttpMethod("Get") == false);
  }

  SECTION("Typo in DELETE") {
    REQUIRE(config::utils::isValidHttpMethod("DELETEE") == false);
  }

  SECTION("Spaces in method") {
    REQUIRE(config::utils::isValidHttpMethod("G E T") == false);
  }

  SECTION("Empty string") {
    REQUIRE(config::utils::isValidHttpMethod("") == false);
  }

  SECTION("HEAD method not in whitelist") {
    REQUIRE(config::utils::isValidHttpMethod("HEAD") == false);
  }
}

// ============================================================================
// VALIDATION 6: Error Code Range (testing existing implementation)
// ============================================================================

TEST_CASE("ServerConfig::addErrorPage - valid error codes",
          "[config][validation][errorcode]") {
  ServerConfig server;

  SECTION("Informational code - 100") {
    REQUIRE_NOTHROW(server.addErrorPage(100, "/error.html"));
  }

  SECTION("Not Found - 404") {
    REQUIRE_NOTHROW(server.addErrorPage(404, "/404.html"));
  }

  SECTION("Internal Server Error - 500") {
    REQUIRE_NOTHROW(server.addErrorPage(500, "/500.html"));
  }

  SECTION("Maximum valid code - 599") {
    REQUIRE_NOTHROW(server.addErrorPage(599, "/error.html"));
  }
}

TEST_CASE("ServerConfig::addErrorPage - invalid error codes",
          "[config][validation][errorcode]") {
  ServerConfig server;

  SECTION("Below minimum - 99") {
    REQUIRE_THROWS(server.addErrorPage(99, "/error.html"));
  }

  SECTION("Zero") { REQUIRE_THROWS(server.addErrorPage(0, "/error.html")); }

  SECTION("Above maximum - 600") {
    REQUIRE_THROWS(server.addErrorPage(600, "/error.html"));
  }

  SECTION("Very large code") {
    REQUIRE_THROWS(server.addErrorPage(1000, "/error.html"));
  }
}

// ============================================================================
// VALIDATION 7: Max Body Size Limits
// ============================================================================

TEST_CASE("config::utils::parseSize - valid values",
          "[config][validation][bodysize]") {
  SECTION("Zero bytes") { REQUIRE(config::utils::parseSize("0") == 0); }

  SECTION("Plain bytes") { REQUIRE(config::utils::parseSize("1024") == 1024); }

  SECTION("Kilobytes") {
    REQUIRE(config::utils::parseSize("1K") == 1024);
    REQUIRE(config::utils::parseSize("10K") == 10240);
  }

  SECTION("Megabytes") {
    REQUIRE(config::utils::parseSize("1M") == 1048576);
    REQUIRE(config::utils::parseSize("10M") == 10485760);
  }

  SECTION("Gigabytes") { REQUIRE(config::utils::parseSize("1G") == 1073741824); }

  SECTION("Maximum safe value (2GB - 1)") {
    REQUIRE_NOTHROW(config::utils::parseSize("2147483647"));
  }
}

TEST_CASE("config::utils::parseSize - invalid values",
          "[config][validation][bodysize]") {
  SECTION("Negative value") { REQUIRE_THROWS(config::utils::parseSize("-1")); }

  SECTION("Negative with suffix") {
    REQUIRE_THROWS(config::utils::parseSize("-100K"));
  }

  SECTION("Exceeds 2GB limit") { REQUIRE_THROWS(config::utils::parseSize("3G")); }

  SECTION("Overflow with large multiplier") {
    REQUIRE_THROWS(config::utils::parseSize("99999G"));
  }

  SECTION("Value above INT_MAX") {
    REQUIRE_THROWS(config::utils::parseSize("2147483648"));
  }
}

// ============================================================================
// VALIDATION 8: Path Existence Checks
// ============================================================================

TEST_CASE("config::utils::checkRootPath - path validation",
          "[config][validation][pathcheck]") {
  SECTION("Existing directory returns empty string") {
    std::string result = config::utils::checkRootPath("/tmp");
    REQUIRE(result.empty());
  }

  SECTION("Non-existing directory returns warning") {
    std::string result =
        config::utils::checkRootPath("/this/path/does/not/exist/webserv/test");
    REQUIRE_FALSE(result.empty());
    REQUIRE(result.find("does not exist") != std::string::npos);
  }

  SECTION("File instead of directory returns warning") {
    std::string result = config::utils::checkRootPath("/etc/passwd");
    REQUIRE_FALSE(result.empty());
  }
}

TEST_CASE("config::utils::ensureUploadStorePath - directory creation",
          "[config][validation][pathcheck]") {
  SECTION("Existing directory succeeds") {
    REQUIRE_NOTHROW(config::utils::ensureUploadStorePath("/tmp"));
  }

  SECTION("Creates non-existing directory in /tmp") {
    std::string test_path =
        "/tmp/webserv_test_upload_" + std::to_string(time(NULL));
    REQUIRE_NOTHROW(config::utils::ensureUploadStorePath(test_path));
    // Cleanup
    rmdir(test_path.c_str());
  }

  SECTION("Fails for invalid parent directory") {
    REQUIRE_THROWS(config::utils::ensureUploadStorePath(
        "/invalid_root_path_webserv/uploads"));
  }
}
