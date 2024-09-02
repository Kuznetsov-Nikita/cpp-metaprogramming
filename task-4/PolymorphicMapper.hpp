#pragma once

#include <concepts>
#include <optional>


template<class From, auto target>
struct Mapping {
    using key = const From;
    static constexpr auto value = target;
};

namespace detail {
    struct Empty {};

    template<class T>
    concept EmptyMapperType = std::derived_from<T, Empty>;

    template<class T, class Base, class Target>
    concept MapperTypeFrom = requires { typename T::key; T::value; } && 
                             std::derived_from<typename T::key, Base> &&
                             std::same_as<Target, std::remove_const_t<typeof(T::value)>>;

    template<class T, class Base, class Target>
    concept Mapper = EmptyMapperType<T> || MapperTypeFrom<T, Base, Target>;

    template<class Base, class Target, Mapper<Base, Target> Mapping, class... OtherMappings>
    struct PolymorphicMapperHelper {
        static std::optional<Target> map(const Base& object) {
            if (dynamic_cast<Mapping::key*>(&object) != nullptr) {
                return std::optional<Target>(Mapping::value);
            } else {
                return PolymorphicMapperHelper<Base, Target, OtherMappings...>::map(object);
            }
        }
    };

    template<class Base, class Target, Mapper<Base, Target> Mapping>
    struct PolymorphicMapperHelper<Base, Target, Mapping> {
        static std::optional<Target> map(const Base& object) {
            if (dynamic_cast<Mapping::key*>(&object) != nullptr) {
                return std::optional<Target>(Mapping::value);
            } else {
                return std::nullopt;
            }
        }
    };

    template<class Base, class Target>
    struct PolymorphicMapperHelper<Base, Target, Empty> {
        static std::optional<Target> map(const Base&) {
            return std::nullopt;
        }
    };
}

template<class Base, class Target, detail::Mapper<Base, Target>... Mappings>
struct PolymorphicMapper {
    static std::optional<Target> map(const Base& object) {
        return detail::PolymorphicMapperHelper<Base, Target, Mappings..., detail::Empty>::map(object);
    }
};
