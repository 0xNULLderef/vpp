#ifndef LOCATION_HPP
#define LOCATION_HPP

#include <cstdint>

namespace Memory {
    class Location {
    public:
        Location(std::uintptr_t location) : location(location) {}
        template<typename T> inline T As() const { return reinterpret_cast<T>(this->location); }
        inline std::uintptr_t Value() const { return this->location; }

        template<typename TargetType, typename OffsetType = std::uint32_t> inline TargetType ReadAsRelativePointer() const {
            return reinterpret_cast<TargetType>(this->Value() + *this->As<OffsetType*>() + sizeof(OffsetType));
        }

        template<typename TargetType> inline TargetType ReadAsPointer() const {
            return *reinterpret_cast<TargetType*>(this->Value());
        }

    private:
        std::uintptr_t location;
    };
};

#endif // LOCATION_HPP
