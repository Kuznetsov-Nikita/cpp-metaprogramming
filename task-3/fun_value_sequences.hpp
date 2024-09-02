#pragma once

#include "type_lists.hpp"
#include <value_types.hpp>

template<class T>
concept ValueType = requires { T::Value; };

namespace detail {
    template<ValueType T>
    using Increment = value_types::ValueTag<T::Value + 1>;

    template<ValueType T>
    struct FibonacciHelper {
        using type = value_types::ValueTag<FibonacciHelper<value_types::ValueTag<T::Value - 1>>::type::Value + 
                                           FibonacciHelper<value_types::ValueTag<T::Value - 2>>::type::Value>;
    };

    template<>
    struct FibonacciHelper<value_types::ValueTag<0>> {
        using type = value_types::ValueTag<0>;
    };

    template<>
    struct FibonacciHelper<value_types::ValueTag<1>> {
        using type = value_types::ValueTag<1>;
    };

    template<ValueType T>
    using Fibonacci = FibonacciHelper<T>::type;

    template<ValueType T>
    struct IsPrime {
        static constexpr bool isPrime(int number) {
            if (number < 2) {
                return false;
            }
            if (number == 2) {
                return true;
            }
            if (number % 2 == 0) {
                return false;
            }
            for (int i = 3; i * i <= number; i += 2) {
                if (number % i == 0) {
                    return false;
                }
            }
            return true;
        }

        static constexpr bool Value = isPrime(T::Value);
    };
}

using Nats = type_lists::Iterate<detail::Increment, value_types::ValueTag<0>>;
using Fib = type_lists::Map<detail::Fibonacci, Nats>;
using Primes = type_lists::Filter<detail::IsPrime, Nats>;
