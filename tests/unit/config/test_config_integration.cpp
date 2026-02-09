#include <fstream>

#include "../../lib/catch2/catch.hpp"
#include "../../src/config/ConfigException.hpp"
#include "../../src/config/ConfigParser.hpp"

// ============================================================================
// INTEGRATION TESTS: Full configuration file parsing with validations
// ============================================================================

TEST_CASE("Integration: Valid IP formats in listen directive",
          "[config][integration][ip]") {
  SECTION("IPv4 loopback") {
    std::ofstream file("test_valid_ipv4.conf");
    file << "server {\n"
         << "    listen 127.0.0.1:8080;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_ipv4.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_ipv4.conf");
  }

  SECTION("Hostname localhost") {
    std::ofstream file("test_valid_localhost.conf");
    file << "server {\n"
         << "    listen localhost:8080;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_localhost.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_localhost.conf");
  }

  SECTION("Domain name") {
    std::ofstream file("test_valid_domain.conf");
    file << "server {\n"
         << "    listen example.com:8080;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_domain.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_domain.conf");
  }
}

TEST_CASE("Integration: Invalid IP formats in listen directive",
          "[config][integration][ip]") {
  SECTION("IPv4 with octet > 255") {
    std::ofstream file("test_invalid_ip.conf");
    file << "server {\n"
         << "    listen 256.1.1.1:8080;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_ip.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_ip.conf");
  }

  SECTION("Invalid hostname starting with hyphen") {
    std::ofstream file("test_invalid_hostname.conf");
    file << "server {\n"
         << "    listen -invalid.com:8080;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_hostname.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_hostname.conf");
  }
}

TEST_CASE("Integration: Valid location paths",
          "[config][integration][location]") {
  SECTION("Root location") {
    std::ofstream file("test_valid_location.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location / {\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_location.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_location.conf");
  }

  SECTION("API location") {
    std::ofstream file("test_valid_api_location.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location /api/v1 {\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_api_location.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_api_location.conf");
  }
}

TEST_CASE("Integration: Invalid location paths",
          "[config][integration][location]") {
  SECTION("Location without leading slash") {
    std::ofstream file("test_invalid_location.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location api {\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_location.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_location.conf");
  }

  SECTION("Empty location path") {
    std::ofstream file("test_empty_location.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location  {\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_empty_location.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_empty_location.conf");
  }
}

TEST_CASE("Integration: Valid HTTP methods", "[config][integration][methods]") {
  SECTION("Single GET method") {
    std::ofstream file("test_valid_methods_get.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location / {\n"
         << "        limit_except GET;\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_methods_get.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_methods_get.conf");
  }

  SECTION("Multiple valid methods") {
    std::ofstream file("test_valid_methods_multi.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location / {\n"
         << "        limit_except GET POST DELETE;\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_methods_multi.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_methods_multi.conf");
  }
}

TEST_CASE("Integration: Invalid HTTP methods",
          "[config][integration][methods]") {
  SECTION("PUT method not allowed") {
    std::ofstream file("test_invalid_method_put.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location / {\n"
         << "        limit_except PUT;\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_method_put.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_method_put.conf");
  }

  SECTION("Lowercase method") {
    std::ofstream file("test_invalid_method_lowercase.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    location / {\n"
         << "        limit_except get;\n"
         << "        root /var/www;\n"
         << "    }\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_method_lowercase.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_method_lowercase.conf");
  }
}

TEST_CASE("Integration: Valid body size limits",
          "[config][integration][bodysize]") {
  SECTION("Size in bytes") {
    std::ofstream file("test_valid_bodysize_bytes.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    client_max_body_size 1048576;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_bodysize_bytes.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_bodysize_bytes.conf");
  }

  SECTION("Size with K suffix") {
    std::ofstream file("test_valid_bodysize_k.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    client_max_body_size 10K;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_bodysize_k.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_bodysize_k.conf");
  }

  SECTION("Size with M suffix") {
    std::ofstream file("test_valid_bodysize_m.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    client_max_body_size 100M;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_valid_bodysize_m.conf");
    REQUIRE_NOTHROW(parser.parse());
    std::remove("test_valid_bodysize_m.conf");
  }
}

TEST_CASE("Integration: Invalid body size limits",
          "[config][integration][bodysize]") {
  SECTION("Negative size") {
    std::ofstream file("test_invalid_bodysize_neg.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    client_max_body_size -1;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_bodysize_neg.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_bodysize_neg.conf");
  }

  SECTION("Size exceeds 2GB limit") {
    std::ofstream file("test_invalid_bodysize_large.conf");
    file << "server {\n"
         << "    listen 8080;\n"
         << "    client_max_body_size 3G;\n"
         << "    root /var/www;\n"
         << "}\n";
    file.close();

    ConfigParser parser("test_invalid_bodysize_large.conf");
    REQUIRE_THROWS_AS(parser.parse(), ConfigException);
    std::remove("test_invalid_bodysize_large.conf");
  }
}
