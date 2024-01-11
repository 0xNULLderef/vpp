#include "scanner.hpp"

#include <immintrin.h>
#include <sstream>
#include <vector>
#include "logger.hpp"

namespace Memory {
    static constexpr auto MASK_FULL = std::byte { 0xFF };
    static constexpr auto MASK_EMPTY = std::byte { 0x00 };

    class ScanData {
    public:
        ScanData(std::string pattern) : bytes({}), mask({}), locationIndexFirst(0), locationIndexLast(0) {
            std::istringstream stream(pattern);
            std::string characterGroup;

            while(stream >> characterGroup) {
                if(characterGroup == "?" || characterGroup == "??") {
                    this->bytes.push_back(std::byte { 0 });
                    this->mask.push_back(MASK_EMPTY);
                } else {
                    this->bytes.push_back(std::byte { static_cast<std::uint8_t>(std::stoul(characterGroup, nullptr, 16)) });
                    this->mask.push_back(MASK_FULL);
                }
            }

            for(size_t first = 0; first < this->mask.size() - 1; first++) {
                if(this->mask[first] == MASK_FULL) {
                    this->locationIndexFirst = first;
                    break;
                }
            }

            for(size_t last = this->mask.size() - 1; last > 0; last--) {
                if(this->mask[last] == MASK_FULL) {
                    this->locationIndexLast = last;
                    break;
                }
            }

            this->length = this->bytes.size();
        }
        
        size_t length;
        std::vector<std::byte> bytes;
        std::vector<std::byte> mask;
        size_t locationIndexFirst;
        size_t locationIndexLast;
    };

    class ScannerAVXImplementation : public ScannerImplementation {
    public:
        Memory::Location Scan(const std::span<std::byte> target, std::string pattern, std::intptr_t offset) override {
            const ScanData data(pattern);

            const auto locationMaskFirst = _mm256_set1_epi8(static_cast<std::uint8_t>(data.bytes[data.locationIndexFirst]));
            const auto locationMaskLast = _mm256_set1_epi8(static_cast<std::uint8_t>(data.bytes[data.locationIndexLast]));

            for(size_t blockOffset = 0; blockOffset < target.size() - BLOCK_SIZE; blockOffset += BLOCK_SIZE) {
                const auto scanBlockFirst = _mm256_loadu_si256(reinterpret_cast<const Block*>(&target[blockOffset + data.locationIndexFirst]));
                const auto scanBlockLast = _mm256_loadu_si256(reinterpret_cast<const Block*>(&target[blockOffset + data.locationIndexLast]));

                const auto comparedMask = _mm256_movemask_epi8(_mm256_and_si256(
                    _mm256_cmpeq_epi8(scanBlockFirst, locationMaskFirst),
                    _mm256_cmpeq_epi8(scanBlockLast, locationMaskLast)
                ));

                if(comparedMask != 0) {
                    for(size_t bitPosition = 0; bitPosition < sizeof(comparedMask) * 8; bitPosition++) {
                        if((comparedMask & (1 << bitPosition)) != 0) {
                            if(this->MaskedCompare(std::span { &target[blockOffset + bitPosition], data.length }, data)) {
                                return reinterpret_cast<const std::uintptr_t>(&target[blockOffset + bitPosition + offset]);
                            }
                        }
                    }
                }
            }

            throw std::runtime_error("Sigsearch fail :P");
        }

    private:
        inline bool MaskedCompare(const std::span<std::byte> target, const ScanData& data) {
            // don't do first compares as integer / byte
            // because the chance of both the target and our scan data being aligned the same way is low
            // also usually our scan datavector should be aligned so it *shouldn't* cache miss *that* much
            const auto blockSizeAligned = (target.size() & ~(BLOCK_SIZE - 1));
            for(size_t blockOffset = 0; blockOffset < blockSizeAligned; blockOffset += BLOCK_SIZE) {
                const auto targetBlock = _mm256_loadu_si256(reinterpret_cast<const Block*>(&target[blockOffset]));
                const auto bytesBlock = _mm256_loadu_si256(reinterpret_cast<const Block*>(&data.bytes[blockOffset]));
                const auto maskBlock = _mm256_loadu_si256(reinterpret_cast<const Block*>(&data.mask[blockOffset]));

                const auto compareMask = _mm256_cmpeq_epi8(targetBlock, bytesBlock);

                if(!_mm256_testc_si256(compareMask, maskBlock)) {
                    // early return when non-match hit
                    return false;
                }
            }

            // last few bytes get a normal comparison
            // runs less than BLOCK_SIZE times so shouldn't eat cpu
            for(size_t blockOffset = blockSizeAligned; blockOffset < target.size(); blockOffset++) {
                if(target[blockOffset] != data.bytes[blockOffset] && data.mask[blockOffset] == MASK_FULL) {
                    return false;
                }
            }

            return true;
        }

        using Block = __m256i;
        static constexpr size_t BLOCK_SIZE = sizeof(Block);
    };

    class ScannerSSEImplementation : public ScannerImplementation {
    public:
        Memory::Location Scan(const std::span<std::byte> target, std::string pattern, std::intptr_t offset) override {
            const ScanData data(pattern);

            const auto locationMaskFirst = _mm_set1_epi8(static_cast<std::uint8_t>(data.bytes[data.locationIndexFirst]));
            const auto locationMaskLast = _mm_set1_epi8(static_cast<std::uint8_t>(data.bytes[data.locationIndexLast]));

            for(size_t blockOffset = 0; blockOffset < target.size() - BLOCK_SIZE; blockOffset += BLOCK_SIZE) {
                const auto scanBlockFirst = _mm_loadu_si128(reinterpret_cast<const Block*>(&target[blockOffset + data.locationIndexFirst]));
                const auto scanBlockLast = _mm_loadu_si128(reinterpret_cast<const Block*>(&target[blockOffset + data.locationIndexLast]));

                const auto comparedMask = _mm_movemask_epi8(_mm_and_si128(
                    _mm_cmpeq_epi8(scanBlockFirst, locationMaskFirst),
                    _mm_cmpeq_epi8(scanBlockLast, locationMaskLast)
                ));

                if(comparedMask != 0) {
                    for(size_t bitPosition = 0; bitPosition < sizeof(comparedMask) * 8; bitPosition++) {
                        if((comparedMask & (1 << bitPosition)) != 0) {
                            if(this->MaskedCompare(std::span { &target[blockOffset + bitPosition], data.length }, data)) {
                                return reinterpret_cast<const std::uintptr_t>(&target[blockOffset + bitPosition + offset]);
                            }
                        }
                    }
                }
            }

            throw std::runtime_error("Sigsearch fail :P");
        }

    private:
        inline bool MaskedCompare(const std::span<std::byte> target, const ScanData& data) {
            // don't do first compares as integer / byte
            // because the chance of both the target and our scan data being aligned the same way is low
            // also usually our scan datavector should be aligned so it *shouldn't* cache miss *that* much
            const auto blockSizeAligned = (target.size() & ~(BLOCK_SIZE - 1));
            for(size_t blockOffset = 0; blockOffset < blockSizeAligned; blockOffset += BLOCK_SIZE) {
                const auto targetBlock = _mm_loadu_si128(reinterpret_cast<const Block*>(&target[blockOffset]));
                const auto bytesBlock = _mm_loadu_si128(reinterpret_cast<const Block*>(&data.bytes[blockOffset]));
                const auto maskBlock = _mm_loadu_si128(reinterpret_cast<const Block*>(&data.mask[blockOffset]));

                const auto compareMask = _mm_cmpeq_epi8(targetBlock, bytesBlock);

                if(!_mm_testc_si128(compareMask, maskBlock)) {
                    // early return when non-match hit
                    return false;
                }
            }

            // last few bytes get a normal comparison
            // runs less than BLOCK_SIZE times so shouldn't eat cpu
            for(size_t blockOffset = blockSizeAligned; blockOffset < target.size(); blockOffset++) {
                if(target[blockOffset] != data.bytes[blockOffset] && data.mask[blockOffset] == MASK_FULL) {
                    return false;
                }
            }

            return true;
        }

        using Block = __m128i;
        static constexpr size_t BLOCK_SIZE = sizeof(Block);
    };

    Memory::Location Scanner::Scan(const std::span<std::byte> target, std::string pattern, std::intptr_t offset) {
        return Scanner::Implementation().get()->Scan(target, pattern, offset);
    }

    std::unique_ptr<ScannerImplementation>& Scanner::Implementation() {
        static std::unique_ptr<ScannerImplementation> implementation;

        if(implementation == nullptr) {
            int cpuid[4];

#ifdef _WIN32
            __cpuidex(cpuid, 1, 0);
#else
            asm volatile("cpuid"
                : "=a" (cpuid[0]),
                "=b" (cpuid[1]),
                "=c" (cpuid[2]),
                "=d" (cpuid[3])
                : "0" (1), "2" (0)
            );
#endif

            if(cpuid[2] & (1 << 28)) {
                implementation = std::make_unique<ScannerAVXImplementation>();
            } else {
                implementation = std::make_unique<ScannerSSEImplementation>();
            }
        }

        return implementation;
    }
};
