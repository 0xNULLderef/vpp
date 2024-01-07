#ifndef MEMORY_SCANNER_HPP
#define MEMORY_SCANNER_HPP

#include "location.hpp"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string>

namespace Memory {
    class Target {
    public:
        virtual std::span<std::byte> ByteSpan() const = 0;
    };

    class ScannerImplementation {
    public:
        virtual Memory::Location Scan(const std::span<std::byte> target, std::string pattern, std::intptr_t offset) = 0;
    };

    class Scanner {
    public:
        static Memory::Location Scan(const std::span<std::byte> target, std::string pattern, std::intptr_t offset = 0);

    private:
        static std::unique_ptr<ScannerImplementation>& Implementation();
    };
};

#endif // MEMORY_SCANNER_HPP
