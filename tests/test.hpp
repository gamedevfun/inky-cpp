#pragma once

#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

using InkyTestCase = std::pair<std::string, std::function<void()>>;

std::vector<InkyTestCase> &inkyTestRegistry();

struct InkyTestRegistrar {
    InkyTestRegistrar(std::string name, std::function<void()> fn);
};

#define INKY_TEST(name) \
    void name(); \
    namespace { InkyTestRegistrar name##_registrar(#name, name); } \
    void name()

#define INKY_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            throw std::runtime_error("Assertion failed: " #condition); \
        } \
    } while (false)

#define INKY_ASSERT_EQ(lhs, rhs) \
    do { \
        const auto lhsValue = (lhs); \
        const auto rhsValue = (rhs); \
        if (!(lhsValue == rhsValue)) { \
            throw std::runtime_error("Assertion failed: " #lhs " == " #rhs); \
        } \
    } while (false)
