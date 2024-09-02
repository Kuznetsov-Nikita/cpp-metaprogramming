#pragma once

#include <concepts>

#include <type_tuples.hpp>

namespace type_lists
{

template<class TL>
concept TypeSequence =
    requires {
        typename TL::Head;
        typename TL::Tail;
    };

struct Nil {};

template<class TL>
concept Empty = std::derived_from<TL, Nil>;

template<class TL>
concept TypeList = Empty<TL> || TypeSequence<TL>;

template<class T, TypeList TL>
struct Cons;

namespace detail {
    template<typename Accumulator, TypeList InputTL>
    struct ToTupleHelper;

    template<class... Ts, TypeList TL>
    struct ToTupleHelper<type_tuples::TTuple<Ts...>, TL> {
        using type = typename ToTupleHelper<type_tuples::TTuple<Ts..., typename TL::Head>, typename TL::Tail>::type;
    };

    template<class... Ts, Empty ETL>
    struct ToTupleHelper<type_tuples::TTuple<Ts...>, ETL> {
        using type = type_tuples::TTuple<Ts...>;
    };

    template<typename ReverseTL, typename TL>
    struct ReverseHelper;

    template<TypeList ReverseTL, TypeList TL>
    struct ReverseHelper<ReverseTL, TL> {
        using type = ReverseHelper<Cons<typename TL::Head, ReverseTL>, typename TL::Tail>::type;
    };

    template<TypeList TL, Empty ETL>
    struct ReverseHelper<TL, ETL> {
        using type = TL;
    };

    template<typename Accumulator, type_tuples::TypeTuple TT>
    struct FromTupleHelper;

    template<class Head, class... Tail, TypeList TL>
    struct FromTupleHelper<TL, type_tuples::TTuple<Head, Tail...>> {
        using type = typename FromTupleHelper<Cons<Head, TL>, type_tuples::TTuple<Tail...>>::type;
    };

    template<TypeList TL>
    struct FromTupleHelper<TL, type_tuples::TTuple<>> {
        using type = TL;
    };

    template<std::size_t N, TypeList TL>
    struct TakeHelper {
        using type = Cons<typename TL::Head, typename TakeHelper<N - 1, typename TL::Tail>::type>;
    };

    template<std::size_t N, Empty ETL>
    struct TakeHelper<N, ETL> {
        using type = ETL;
    };

    template<TypeList TL>
    struct TakeHelper<0, TL> {
        using type = Nil;
    };

    template<std::size_t N, TypeList TL>
    struct DropHelper {
        using type = typename DropHelper<N - 1, typename TL::Tail>::type;
    };

    template<std::size_t N, Empty ETL>
    struct DropHelper<N, ETL> {
        using type = ETL;
    };

    template<TypeList TL>
    struct DropHelper<0, TL> {
        using type = TL;
    };

    template<std::size_t N, class T>
    struct ReplicateHelper {
        using type = Cons<T, typename ReplicateHelper<N - 1, T>::type>;
    };

    template<class T>
    struct ReplicateHelper<0, T> {
        using type = Nil;
    };

    template<template<typename> typename P, TypeList TL>
    struct FilterHelper;

    template<class H, TypeList T, template<typename> typename P, bool PValue>
    struct FilterHelperIf {
        using Head = typename FilterHelper<P, T>::Head;
        using Tail = typename FilterHelper<P, T>::Tail;
    };

    template<class H, TypeList T, template<typename> typename P>
    struct FilterHelperIf<H, T, P, true> {
        using Head = H;
        using Tail = FilterHelper<P, T>;
    };

    template<template<typename> typename P, TypeList TL>
    struct FilterHelper {
        using Head = typename FilterHelperIf<typename TL::Head, typename TL::Tail, P, P<typename TL::Head>::Value>::Head;
        using Tail = typename FilterHelperIf<typename TL::Head, typename TL::Tail, P, P<typename TL::Head>::Value>::Tail;
    };

    template<template<typename> typename P, Empty ETL>
    struct FilterHelper<P, ETL>: Nil {
        using Head = Nil;
        using Tail = Nil;
    };

    template<TypeList TL, TypeList CurrentTL>
    struct CycleHelper {
        using Head = typename CurrentTL::Head;
        using Tail = CycleHelper<TL, typename CurrentTL::Tail>;
    };

    template<TypeList TL, Empty ETL>
    struct CycleHelper<TL, ETL> {
        using Head = typename TL::Head;
        using Tail = CycleHelper<TL, typename TL::Tail>;
    };
    
    template<Empty ETL, TypeList TL>
    struct CycleHelper<ETL, TL>: Nil {};

    template<Empty ETL, Empty CurrentETL>
    struct CycleHelper<ETL, CurrentETL>: Nil {};

    template<TypeList TL, TypeList Prefix>
    struct InitsHelper {
        using Head = ReverseHelper<Nil, Cons<typename TL::Head, Prefix>>::type;
        using Tail = InitsHelper<typename TL::Tail, Cons<typename TL::Head, Prefix>>;
    };

    template<Empty ETL, TypeList Prefix>
    struct InitsHelper<ETL, Prefix>: Nil {};

    template<template<typename, typename> typename OP, class T, TypeList TL>
    struct ScanlHelper {
        using Head = OP<T, typename TL::Head>;
        using Tail = ScanlHelper<OP, typename TL::Head, typename TL::Tail>;
    };

    template<template<typename, typename> typename OP, class T, Empty ETL>
    struct ScanlHelper<OP, T, ETL>: Nil {};

    template<template<typename, typename> typename OP, class T, TypeList TL>
    struct FoldlHelper {
        using type = typename FoldlHelper<OP, OP<T, typename TL::Head>, typename TL::Tail>::type;
    };

    template<template<typename, typename> typename OP, class T, Empty ETL>
    struct FoldlHelper<OP, T, ETL> {
        using type = T;
    };
}

template<class T, TypeList TL>
struct Cons {
    using Head = T;
    using Tail = TL;
};

template<TypeList TL>
using Reverse = detail::ReverseHelper<Nil, TL>::type;

template<class TL>
using ToTuple = detail::ToTupleHelper<type_tuples::TTuple<>, TL>::type;

template<type_tuples::TypeTuple TT>
using FromTuple = Reverse<typename detail::FromTupleHelper<Nil, TT>::type>;

template<class T>
struct Repeat {
    using Head = T;
    using Tail = Repeat<T>;
};

template<std::size_t N, TypeList TL>
using Take = detail::TakeHelper<N, TL>::type;

template<std::size_t N, TypeList TL>
using Drop = detail::DropHelper<N, TL>::type;

template<std::size_t N, class T>
using Replicate = detail::ReplicateHelper<N, T>::type;

template<template<typename> typename F, TypeList TL>
struct Map {
    using Head = F<typename TL::Head>;
    using Tail = Map<F, typename TL::Tail>;
};

template<template<typename> typename F, Empty ETL>
struct Map<F, ETL>: Nil {};

template<template<typename> typename P, TypeList TL>
using Filter = detail::FilterHelper<P, TL>;

template<template<typename> typename F, class T>
struct Iterate {
    using Head = T;
    using Tail = Iterate<F, F<T>>;
};

template<TypeList TL>
using Cycle = typename detail::CycleHelper<TL, TL>;

template<TypeList TL>
struct Inits {
    using Head = Nil;
    using Tail = typename detail::InitsHelper<TL, Nil>;
};

template<TypeList TL>
struct Tails {
    using Head = TL;
    using Tail = Tails<typename TL::Tail>;
};

template<Empty ETL>
struct Tails<ETL> {
    using Head = Nil;
    using Tail = Nil;
};

template<template<typename, typename> typename OP, class T, TypeList TL>
struct Scanl {
    using Head = T;
    using Tail = typename detail::ScanlHelper<OP, T, TL>;
};

template<template<typename, typename> typename OP, class T, TypeList TL>
using Foldl = detail::FoldlHelper<OP, T, TL>::type;

template<TypeList L, TypeList R>
struct Zip2 {
    using Head = type_tuples::TTuple<typename L::Head, typename R::Head>;
    using Tail = Zip2<typename L::Tail, typename R::Tail>;
};

template<Empty EL, TypeList R>
struct Zip2<EL, R>: Nil {};

template<TypeList L, Empty ER>
struct Zip2<L, ER>: Nil {};

template<Empty EL, Empty ER>
struct Zip2<EL, ER>: Nil {};

template<TypeList... TL>
struct Zip {
    using Head = type_tuples::TTuple<typename TL::Head...>;
    using Tail = Zip<typename TL::Tail...>;
};

} // namespace type_lists
