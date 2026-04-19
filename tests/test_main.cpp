#include "test.hpp"

#include <exception>
#include <iostream>

std::vector<InkyTestCase> &inkyTestRegistry() {
    static std::vector<InkyTestCase> tests;
    return tests;
}

InkyTestRegistrar::InkyTestRegistrar(std::string name, std::function<void()> fn) {
    inkyTestRegistry().emplace_back(std::move(name), std::move(fn));
}

int main() {
    int failures = 0;

    for (const auto &[name, fn] : inkyTestRegistry()) {
        try {
            fn();
            std::cout << "[PASS] " << name << '\n';
        } catch (const std::exception &ex) {
            ++failures;
            std::cerr << "[FAIL] " << name << ": " << ex.what() << '\n';
        }
    }

    if (failures != 0) {
        std::cerr << failures << " test(s) failed." << std::endl;
        return 1;
    }

    std::cout << inkyTestRegistry().size() << " test(s) passed." << std::endl;
    return 0;
}
