#pragma once
#include <iostream>
#include <string>

static int g_test_failures = 0;
static int g_test_passed = 0;

#define ASSERT_TRUE(cond, msg) \
    if (!(cond)) { \
        std::cout << "[FAIL] " << msg << std::endl; \
        g_test_failures++; \
    } else { \
        std::cout << "[OK]   " << msg << std::endl; \
        g_test_passed++; \
    }

#define ASSERT_EQ(a, b, msg) \
    ASSERT_TRUE((a) == (b), msg)

#define TEST_SUMMARY() \
    do { \
        std::cout << "\n=== Test Summary ===" << std::endl; \
        std::cout << "Passed: " << g_test_passed << std::endl; \
        std::cout << "Failed: " << g_test_failures << std::endl; \
        return g_test_failures == 0 ? 0 : 1; \
    } while(0)
