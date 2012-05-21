
// Copyright 2005-2011 Daniel James.
// Copyright 2009 Pablo Halpern.
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org/libs/unordered for documentation

#ifndef BOOST_UNORDERED_ALLOCATE_HPP
#define BOOST_UNORDERED_ALLOCATE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
# pragma once
#endif

#include <boost/unordered/detail/fwd.hpp>
#include <boost/move/move.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/inc.hpp>
#include <boost/preprocessor/dec.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/type_traits/is_class.hpp>
#include <boost/type_traits/add_lvalue_reference.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/utility/addressof.hpp>
#include <boost/detail/select_type.hpp>
#include <boost/assert.hpp>
#include <utility>

#if !defined(BOOST_NO_0X_HDR_TUPLE)
#include <tuple>
#endif

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4512) // assignment operator could not be generated.
#pragma warning(disable:4345) // behavior change: an object of POD type
                              // constructed with an initializer of the form ()
                              // will be default-initialized.
#endif

#define BOOST_UNORDERED_EMPLACE_LIMIT 10

namespace boost { namespace unordered { namespace detail {

    ////////////////////////////////////////////////////////////////////////////
    // Bits and pieces for implementing traits

    template <typename T> typename boost::add_lvalue_reference<T>::type make();
    struct choice9 { typedef char (&type)[9]; };
    struct choice8 : choice9 { typedef char (&type)[8]; };
    struct choice7 : choice8 { typedef char (&type)[7]; };
    struct choice6 : choice7 { typedef char (&type)[6]; };
    struct choice5 : choice6 { typedef char (&type)[5]; };
    struct choice4 : choice5 { typedef char (&type)[4]; };
    struct choice3 : choice4 { typedef char (&type)[3]; };
    struct choice2 : choice3 { typedef char (&type)[2]; };
    struct choice1 : choice2 { typedef char (&type)[1]; };
    choice1 choose();

    typedef choice1::type yes_type;
    typedef choice2::type no_type;

    struct private_type
    {
       private_type const &operator,(int) const;
    };

    template <typename T>
    no_type is_private_type(T const&);
    yes_type is_private_type(private_type const&);

    struct convert_from_anything {
        template <typename T>
        convert_from_anything(T const&);
    };

    ////////////////////////////////////////////////////////////////////////////
    // emplace_args
    //
    // Either forwarding variadic arguments, or storing the arguments in
    // emplace_args##n

#if !defined(BOOST_NO_VARIADIC_TEMPLATES)

#define BOOST_UNORDERED_EMPLACE_TEMPLATE typename... Args
#define BOOST_UNORDERED_EMPLACE_ARGS BOOST_FWD_REF(Args)... args
#define BOOST_UNORDERED_EMPLACE_FORWARD boost::forward<Args>(args)...

#else

#define BOOST_UNORDERED_EMPLACE_TEMPLATE typename Args
#define BOOST_UNORDERED_EMPLACE_ARGS Args const& args
#define BOOST_UNORDERED_EMPLACE_FORWARD args

#define BOOST_UNORDERED_FWD_PARAM(z, n, a) \
    BOOST_FWD_REF(BOOST_PP_CAT(A, n)) BOOST_PP_CAT(a, n)

#define BOOST_UNORDERED_CALL_FORWARD(z, i, a) \
    boost::forward<BOOST_PP_CAT(A,i)>(BOOST_PP_CAT(a,i))

#define BOOST_UNORDERED_EARGS(z, n, _)                                      \
    template <BOOST_PP_ENUM_PARAMS_Z(z, n, typename A)>                     \
    struct BOOST_PP_CAT(emplace_args, n)                                    \
    {                                                                       \
        BOOST_PP_REPEAT_##z(n, BOOST_UNORDERED_EARGS_MEMBER, _)             \
        BOOST_PP_CAT(emplace_args, n) (                                     \
            BOOST_PP_ENUM_BINARY_PARAMS_Z(z, n, Arg, b)                     \
        ) : BOOST_PP_ENUM_##z(n, BOOST_UNORDERED_EARGS_INIT, _)             \
        {}                                                                  \
                                                                            \
    };                                                                      \
                                                                            \
    template <BOOST_PP_ENUM_PARAMS_Z(z, n, typename A)>                     \
    inline BOOST_PP_CAT(emplace_args, n) <                                  \
        BOOST_PP_ENUM_PARAMS_Z(z, n, A)                                     \
    > create_emplace_args(                                                  \
        BOOST_PP_ENUM_##z(n, BOOST_UNORDERED_FWD_PARAM, b)                  \
    )                                                                       \
    {                                                                       \
        BOOST_PP_CAT(emplace_args, n) <                                     \
            BOOST_PP_ENUM_PARAMS_Z(z, n, A)                                 \
        > e(BOOST_PP_ENUM_PARAMS_Z(z, n, b));                               \
        return e;                                                           \
    }

#if defined(BOOST_NO_RVALUE_REFERENCES)

#define BOOST_UNORDERED_EARGS_MEMBER(z, n, _)                               \
    typedef BOOST_FWD_REF(BOOST_PP_CAT(A, n)) BOOST_PP_CAT(Arg, n);         \
    BOOST_PP_CAT(Arg, n) BOOST_PP_CAT(a, n);

#define BOOST_UNORDERED_EARGS_INIT(z, n, _)                                 \
    BOOST_PP_CAT(a, n)(                                                     \
        boost::forward<BOOST_PP_CAT(A,n)>(BOOST_PP_CAT(b, n)))

#else

#define BOOST_UNORDERED_EARGS_MEMBER(z, n, _)                               \
    typedef typename boost::add_lvalue_reference<BOOST_PP_CAT(A, n)>::type  \
        BOOST_PP_CAT(Arg, n);                                               \
    BOOST_PP_CAT(Arg, n) BOOST_PP_CAT(a, n);

#define BOOST_UNORDERED_EARGS_INIT(z, n, _)                                 \
    BOOST_PP_CAT(a, n)(BOOST_PP_CAT(b, n))

#endif

BOOST_PP_REPEAT_FROM_TO(1, BOOST_UNORDERED_EMPLACE_LIMIT, BOOST_UNORDERED_EARGS,
    _)

#undef BOOST_UNORDERED_DEFINE_EMPLACE_ARGS
#undef BOOST_UNORDERED_EARGS_MEMBER
#undef BOOST_UNORDERED_EARGS_INIT

#endif

    ////////////////////////////////////////////////////////////////////////////
    // rvalue parameters when type can't be a BOOST_RV_REF(T) parameter
    // e.g. for int

#if !defined(BOOST_NO_RVALUE_REFERENCES)
#   define BOOST_UNORDERED_RV_REF(T) BOOST_RV_REF(T)
#else
    struct please_ignore_this_overload {
        typedef please_ignore_this_overload type;
    };

    template <typename T>
    struct rv_ref_impl {
        typedef BOOST_RV_REF(T) type;
    };

    template <typename T>
    struct rv_ref :
        boost::detail::if_true<
            boost::is_class<T>::value
        >::BOOST_NESTED_TEMPLATE then <
            boost::unordered::detail::rv_ref_impl<T>,
            please_ignore_this_overload
        >::type
    {};

#   define BOOST_UNORDERED_RV_REF(T) \
        typename boost::unordered::detail::rv_ref<T>::type
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Construct from tuple
    //
    // Used for piecewise construction.

#if !defined(__SUNPRO_CC)

#   define BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE(n, namespace_)              \
    template<typename T>                                                    \
    void construct_from_tuple(T* ptr, namespace_ tuple<>)                   \
    {                                                                       \
        new ((void*) ptr) T();                                              \
    }                                                                       \
                                                                            \
    BOOST_PP_REPEAT_FROM_TO(1, n,                                           \
        BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE_IMPL, namespace_)

#   define BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE_IMPL(z, n, namespace_)      \
    template<typename T, BOOST_PP_ENUM_PARAMS_Z(z, n, typename A)>          \
    void construct_from_tuple(T* ptr,                                       \
            namespace_ tuple<BOOST_PP_ENUM_PARAMS_Z(z, n, A)> const& x)     \
    {                                                                       \
        new ((void*) ptr) T(                                                \
            BOOST_PP_ENUM_##z(n, BOOST_UNORDERED_GET_TUPLE_ARG, namespace_) \
        );                                                                  \
    }

#   define BOOST_UNORDERED_GET_TUPLE_ARG(z, n, namespace_)                  \
    namespace_ get<n>(x)

#else

    template <int N> struct length {};

#   define BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE(n, namespace_)              \
    template<typename T>                                                    \
    void construct_from_tuple_impl(                                         \
            boost::unordered::detail::length<0>, T* ptr,                    \
            namespace_ tuple<>)                                             \
    {                                                                       \
        new ((void*) ptr) T();                                              \
    }                                                                       \
                                                                            \
    BOOST_PP_REPEAT_FROM_TO(1, n,                                           \
        BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE_IMPL, namespace_)

#   define BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE_IMPL(z, n, namespace_)      \
    template<typename T, BOOST_PP_ENUM_PARAMS_Z(z, n, typename A)>          \
    void construct_from_tuple_impl(                                         \
            boost::unordered::detail::length<n>, T* ptr,                    \
            namespace_ tuple<BOOST_PP_ENUM_PARAMS_Z(z, n, A)> const& x)     \
    {                                                                       \
        new ((void*) ptr) T(                                                \
            BOOST_PP_ENUM_##z(n, BOOST_UNORDERED_GET_TUPLE_ARG, namespace_) \
        );                                                                  \
    }

#   define BOOST_UNORDERED_GET_TUPLE_ARG(z, n, namespace_)                  \
    namespace_ get<n>(x)

#endif

BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE(10, boost::)

#if !defined(__SUNPRO_CC) && !defined(BOOST_NO_0X_HDR_TUPLE)
   BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE(10, std::)
#endif

#undef BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE
#undef BOOST_UNORDERED_CONSTRUCT_FROM_TUPLE_IMPL
#undef BOOST_UNORDERED_GET_TUPLE_ARG

#if defined(__SUNPRO_CC)

    template <typename T, typename Tuple>
    void construct_from_tuple(T* ptr, Tuple const& x)
    {
        construct_from_tuple_impl(
            boost::unordered::detail::length<
                boost::tuples::length<Tuple>::value>(),
            ptr, x);
    }

#endif

    ////////////////////////////////////////////////////////////////////////////
    // SFINAE traits for construction.

    // Decide which construction method to use for a three argument
    // call. Note that this is difficult to do using overloads because
    // the arguments are packed into 'emplace_args3'.
    //
    // The decision is made on the first argument.


#if defined(BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT)
    template <typename A, typename B, typename A0>
    struct emulation1 {
        static choice1::type test(choice1, std::pair<A, B> const&);
        static choice2::type test(choice2, A const&);
        static choice3::type test(choice3, convert_from_anything const&);

        enum { value =
            sizeof(test(choose(), boost::unordered::detail::make<A0>())) ==
                sizeof(choice2::type) };
    };
#endif

    template <typename A, typename B, typename A0>
    struct check3_base {
        static choice1::type test(choice1,
            boost::unordered::piecewise_construct_t);

#if defined(BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT)
        static choice2::type test(choice2, A const&);
#endif

        static choice3::type test(choice3, ...);

        enum { value =
            sizeof(test(choose(), boost::unordered::detail::make<A0>())) };
    };

    template <typename A, typename B, typename A0>
    struct piecewise3 {
        enum { value = check3_base<A,B,A0>::value == sizeof(choice1::type) };
    };

#if defined(BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT)
    template <typename A, typename B, typename A0>
    struct emulation3 {
        enum { value = check3_base<A,B,A0>::value == sizeof(choice2::type) };
    };

#endif

#if !defined(BOOST_NO_VARIADIC_TEMPLATES)

    ////////////////////////////////////////////////////////////////////////////
    // Construct from variadic parameters

    template <typename T, typename... Args>
    inline void construct_impl(T* address, BOOST_FWD_REF(Args)... args)
    {
        new((void*) address) T(boost::forward<Args>(args)...);
    }

    template <typename A, typename B, typename A0, typename A1, typename A2>
    inline typename enable_if<piecewise3<A, B, A0>, void>::type
        construct_impl(std::pair<A, B>* address, A0&&, A1&& a1, A2&& a2)
    {
        boost::unordered::detail::construct_from_tuple(
            boost::addressof(address->first), a1);
        boost::unordered::detail::construct_from_tuple(
            boost::addressof(address->second), a2);
    }

#if defined(BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT)

    template <typename A, typename B, typename A0>
    inline typename enable_if<emulation1<A, B, A0>, void>::type
        construct_impl(std::pair<A, B>* address, A0&& a0)
    {
        new((void*) boost::addressof(address->first)) A(boost::forward<A0>(a0));
        new((void*) boost::addressof(address->second)) B();
   }

    template <typename A, typename B, typename A0, typename A1, typename A2>
    inline typename enable_if<emulation3<A, B, A0>, void>::type
        construct_impl(std::pair<A, B>* address, A0&& a0, A1&& a1, A2&& a2)
    {
        new((void*) boost::addressof(address->first)) A(boost::forward<A0>(a0));
        new((void*) boost::addressof(address->second)) B(
            boost::forward<A1>(a1),
            boost::forward<A2>(a2));
    }

    template <typename A, typename B,
            typename A0, typename A1, typename A2, typename A3,
            typename... Args>
    inline void construct_impl(std::pair<A, B>* address,
            A0&& a0, A1&& a1, A2&& a2, A3&& a3, Args&&... args)
    {
        new((void*) boost::addressof(address->first)) A(boost::forward<A0>(a0));

        new((void*) boost::addressof(address->second)) B(
            boost::forward<A1>(a1),
            boost::forward<A2>(a2),
            boost::forward<A3>(a3),
            boost::forward<Args>(args)...);
    }

#endif // BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT
#else // BOOST_NO_VARIADIC_TEMPLATES

////////////////////////////////////////////////////////////////////////////////
// Construct from emplace_args

#define BOOST_UNORDERED_CONSTRUCT_IMPL(z, num_params, _)                    \
    template <                                                              \
        typename T,                                                         \
        BOOST_PP_ENUM_PARAMS_Z(z, num_params, typename A)                   \
    >                                                                       \
    inline void construct_impl(T* address,                                  \
        boost::unordered::detail::BOOST_PP_CAT(emplace_args,num_params) <   \
            BOOST_PP_ENUM_PARAMS_Z(z, num_params, A)                        \
        > const& args)                                                      \
    {                                                                       \
        new((void*) address) T(                                             \
            BOOST_PP_ENUM_##z(num_params, BOOST_UNORDERED_CALL_FORWARD,     \
                args.a));                                                   \
    }

    template <typename T, typename A0>
    inline void construct_impl(T* address, emplace_args1<A0> const& args)
    {
        new((void*) address) T(boost::forward<A0>(args.a0));
    }

    template <typename T, typename A0, typename A1>
    inline void construct_impl(T* address, emplace_args2<A0, A1> const& args)
    {
        new((void*) address) T(
            boost::forward<A0>(args.a0),
            boost::forward<A1>(args.a1)
        );
    }

    template <typename T, typename A0, typename A1, typename A2>
    inline void construct_impl(T* address, emplace_args3<A0, A1, A2> const& args)
    {
        new((void*) address) T(
            boost::forward<A0>(args.a0),
            boost::forward<A1>(args.a1),
            boost::forward<A2>(args.a2)
        );
    }

    BOOST_PP_REPEAT_FROM_TO(4, BOOST_UNORDERED_EMPLACE_LIMIT,
        BOOST_UNORDERED_CONSTRUCT_IMPL, _)

#undef BOOST_UNORDERED_CONSTRUCT_IMPL

    template <typename A, typename B, typename A0, typename A1, typename A2>
    inline typename enable_if<piecewise3<A, B, A0>, void>::type
        construct_impl(std::pair<A, B>* address,
            boost::unordered::detail::emplace_args3<A0, A1, A2> const& args)
    {
        boost::unordered::detail::construct_from_tuple(
            boost::addressof(address->first), args.a1);
        boost::unordered::detail::construct_from_tuple(
            boost::addressof(address->second), args.a2);
    }

#if defined(BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT)

    template <typename A, typename B, typename A0>
    inline typename enable_if<emulation1<A, B, A0>, void>::type
        construct_impl(std::pair<A, B>* address,
            boost::unordered::detail::emplace_args1<A0> const& args)
    {
        new((void*) boost::addressof(address->first)) A(
            boost::forward<A0>(args.a0));
        new((void*) boost::addressof(address->second)) B();
    }

    template <typename A, typename B, typename A0, typename A1, typename A2>
    inline typename enable_if<emulation3<A, B, A0>, void>::type
        construct_impl(std::pair<A, B>* address,
            boost::unordered::detail::emplace_args3<A0, A1, A2> const& args)
    {
        new((void*) boost::addressof(address->first)) A(
            boost::forward<A0>(args.a0));
        new((void*) boost::addressof(address->second)) B(
            boost::forward<A1>(args.a1),
            boost::forward<A2>(args.a2));
    }

#define BOOST_UNORDERED_CONSTRUCT_PAIR_IMPL(z, num_params, _)               \
    template <typename A, typename B,                                       \
        BOOST_PP_ENUM_PARAMS_Z(z, num_params, typename A)                   \
    >                                                                       \
    inline void construct_impl(std::pair<A, B>* address,                    \
        boost::unordered::detail::BOOST_PP_CAT(emplace_args, num_params) <  \
                BOOST_PP_ENUM_PARAMS_Z(z, num_params, A)                    \
            > const& args)                                                  \
    {                                                                       \
        new((void*) boost::addressof(address->first)) A(                    \
            boost::forward<A0>(args.a0));                                   \
        new((void*) boost::addressof(address->second)) B(                   \
            BOOST_PP_ENUM_##z(BOOST_PP_DEC(num_params),                     \
                BOOST_UNORDERED_CALL_FORWARD2, args.a));                    \
    }

#define BOOST_UNORDERED_CALL_FORWARD2(z, i, a) \
    BOOST_UNORDERED_CALL_FORWARD(z, BOOST_PP_INC(i), a)

    BOOST_UNORDERED_CONSTRUCT_PAIR_IMPL(1, 2, _)
    BOOST_PP_REPEAT_FROM_TO(4, BOOST_UNORDERED_EMPLACE_LIMIT,
        BOOST_UNORDERED_CONSTRUCT_PAIR_IMPL, _)

#undef BOOST_UNORDERED_CONSTRUCT_PAIR_IMPL
#undef BOOST_UNORDERED_CALL_FORWARD2

#endif // BOOST_UNORDERED_DEPRECATED_PAIR_CONSTRUCT
#endif // BOOST_NO_VARIADIC_TEMPLATES

    ////////////////////////////////////////////////////////////////////////////
    // Construct without using the emplace args mechanism.

    template <typename T, typename A0>
    inline void construct_impl2(T* address, BOOST_FWD_REF(A0) a0)
    {
        new((void*) address) T(
            boost::forward<A0>(a0)
        );
    }

}}}

////////////////////////////////////////////////////////////////////////////////
//
// Pick which version of allocator_traits to use
//
// 0 = Own partial implementation
// 1 = std::allocator_traits
// 2 = boost::container::allocator_traits

#if !defined(BOOST_UNORDERED_USE_ALLOCATOR_TRAITS)
#   if defined(__GXX_EXPERIMENTAL_CXX0X__) && \
            (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
#       define BOOST_UNORDERED_USE_ALLOCATOR_TRAITS 0
#   elif defined(BOOST_MSVC)
#       if BOOST_MSVC < 1400
            // Use container's allocator_traits for older versions of Visual
            // C++ as I don't test with them.
#           define BOOST_UNORDERED_USE_ALLOCATOR_TRAITS 2
#       elif BOOST_MSVC >= 1700
#           define BOOST_UNORDERED_USE_ALLOCATOR_TRAITS 1
#       endif
#   endif
#endif

#if !defined(BOOST_UNORDERED_USE_ALLOCATOR_TRAITS)
#   define BOOST_UNORDERED_USE_ALLOCATOR_TRAITS 0
#endif

////////////////////////////////////////////////////////////////////////////////
//
// Some utilities for implementing allocator_traits, but useful elsewhere so
// they're always defined.

#if !defined(BOOST_NO_0X_HDR_TYPE_TRAITS)
#  include <type_traits>
#endif

namespace boost { namespace unordered { namespace detail {

    ////////////////////////////////////////////////////////////////////////////
    // Integral_constrant, true_type, false_type
    //
    // Uses the standard versions if available.

#if !defined(BOOST_NO_0X_HDR_TYPE_TRAITS)

    using std::integral_constant;
    using std::true_type;
    using std::false_type;

#else

    template <typename T, T Value>
    struct integral_constant { enum { value = Value }; };

    typedef boost::unordered::detail::integral_constant<bool, true> true_type;
    typedef boost::unordered::detail::integral_constant<bool, false> false_type;

#endif

    ////////////////////////////////////////////////////////////////////////////
    // Explicitly call a destructor

#if defined(BOOST_MSVC)
#pragma warning(push)
#pragma warning(disable:4100) // unreferenced formal parameter
#endif

    template <class T>
    inline void destroy(T* x) {
        x->~T();
    }

#if defined(BOOST_MSVC)
#pragma warning(pop)
#endif

    ////////////////////////////////////////////////////////////////////////////
    // Expression test mechanism
    //
    // When SFINAE expressions are available, define
    // BOOST_UNORDERED_HAS_FUNCTION which can check if a function call is
    // supported by a class, otherwise define BOOST_UNORDERED_HAS_MEMBER which
    // can detect if a class has the specified member, but not that it has the
    // correct type, this is good enough for a passable impression of
    // allocator_traits.

#if !defined(BOOST_NO_SFINAE_EXPR)

    template <typename T, unsigned int> struct expr_test;
    template <typename T> struct expr_test<T, sizeof(char)> : T {};
    template <typename U> static char for_expr_test(U const&);

#   define BOOST_UNORDERED_CHECK_EXPRESSION(count, result, expression)      \
        template <typename U>                                               \
        static typename boost::unordered::detail::expr_test<                \
            BOOST_PP_CAT(choice, result),                                   \
            sizeof(boost::unordered::detail::for_expr_test((                \
                (expression),                                               \
            0)))>::type test(                                               \
            BOOST_PP_CAT(choice, count))

#   define BOOST_UNORDERED_DEFAULT_EXPRESSION(count, result)                \
        template <typename U>                                               \
        static BOOST_PP_CAT(choice, result)::type test(                     \
            BOOST_PP_CAT(choice, count))

#   define BOOST_UNORDERED_HAS_FUNCTION(name, thing, args, _)               \
    struct BOOST_PP_CAT(has_, name)                                         \
    {                                                                       \
        BOOST_UNORDERED_CHECK_EXPRESSION(1, 1,                              \
            boost::unordered::detail::make< thing >().name args);           \
        BOOST_UNORDERED_DEFAULT_EXPRESSION(2, 2);                           \
                                                                            \
        enum { value = sizeof(test<T>(choose())) == sizeof(choice1::type) };\
    }

#else

    template <typename T> struct identity { typedef T type; };

#   define BOOST_UNORDERED_CHECK_MEMBER(count, result, name, member)        \
                                                                            \
    typedef typename boost::unordered::detail::identity<member>::type       \
        BOOST_PP_CAT(check, count);                                         \
                                                                            \
    template <BOOST_PP_CAT(check, count) e>                                 \
    struct BOOST_PP_CAT(test, count) {                                      \
        typedef BOOST_PP_CAT(choice, result) type;                          \
    };                                                                      \
                                                                            \
    template <class U> static typename                                      \
        BOOST_PP_CAT(test, count)<&U::name>::type                           \
        test(BOOST_PP_CAT(choice, count))

#   define BOOST_UNORDERED_DEFAULT_MEMBER(count, result)                    \
    template <class U> static BOOST_PP_CAT(choice, result)::type            \
        test(BOOST_PP_CAT(choice, count))

#   define BOOST_UNORDERED_HAS_MEMBER(name)                                 \
    struct BOOST_PP_CAT(has_, name)                                         \
    {                                                                       \
        struct impl {                                                       \
            struct base_mixin { int name; };                                \
            struct base : public T, public base_mixin {};                   \
                                                                            \
            BOOST_UNORDERED_CHECK_MEMBER(1, 1, name, int base_mixin::*);    \
            BOOST_UNORDERED_DEFAULT_MEMBER(2, 2);                           \
                                                                            \
            enum { value = sizeof(choice2::type) ==                         \
                sizeof(test<base>(choose()))                                \
            };                                                              \
        };                                                                  \
                                                                            \
        enum { value = impl::value };                                       \
    }

#endif

}}}

////////////////////////////////////////////////////////////////////////////////
//
// Allocator traits
//
// First our implementation, then later light wrappers around the alternatives

#if BOOST_UNORDERED_USE_ALLOCATOR_TRAITS == 0

#   include <boost/limits.hpp>
#   include <boost/utility/enable_if.hpp>
#   include <boost/pointer_to_other.hpp>
#   if defined(BOOST_NO_SFINAE_EXPR)
#       include <boost/type_traits/is_same.hpp>
#   endif

#   if !defined(BOOST_NO_VARIADIC_TEMPLATES) && \
        !defined(BOOST_NO_SFINAE_EXPR)
#       define BOOST_UNORDERED_DETAIL_FULL_CONSTRUCT 1
#   else
#       define BOOST_UNORDERED_DETAIL_FULL_CONSTRUCT 0
#   endif

namespace boost { namespace unordered { namespace detail {

    // TODO: Does this match std::allocator_traits<Alloc>::rebind_alloc<T>?
    template <typename Alloc, typename T>
    struct rebind_wrap
    {
        typedef typename Alloc::BOOST_NESTED_TEMPLATE rebind<T>::other type;
    };

#   if defined(BOOST_MSVC) && BOOST_MSVC <= 1400

#       define BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(tname)                    \
    template <typename Tp, typename Default>                                \
    struct default_type_ ## tname {                                         \
                                                                            \
        template <typename X>                                               \
        static choice1::type test(choice1, typename X::tname* = 0);         \
                                                                            \
        template <typename X>                                               \
        static choice2::type test(choice2, void* = 0);                      \
                                                                            \
        struct DefaultWrap { typedef Default tname; };                      \
                                                                            \
        enum { value = (1 == sizeof(test<Tp>(choose()))) };                 \
                                                                            \
        typedef typename boost::detail::if_true<value>::                    \
            BOOST_NESTED_TEMPLATE then<Tp, DefaultWrap>                     \
            ::type::tname type;                                             \
    }

#   else

    template <typename T, typename T2>
    struct sfinae : T2 {};

#       define BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(tname)                    \
    template <typename Tp, typename Default>                                \
    struct default_type_ ## tname {                                         \
                                                                            \
        template <typename X>                                               \
        static typename boost::unordered::detail::sfinae<                   \
                typename X::tname, choice1>::type                           \
            test(choice1);                                                  \
                                                                            \
        template <typename X>                                               \
        static choice2::type test(choice2);                                 \
                                                                            \
        struct DefaultWrap { typedef Default tname; };                      \
                                                                            \
        enum { value = (1 == sizeof(test<Tp>(choose()))) };                 \
                                                                            \
        typedef typename boost::detail::if_true<value>::                    \
            BOOST_NESTED_TEMPLATE then<Tp, DefaultWrap>                     \
            ::type::tname type;                                             \
    }

#   endif

#   define BOOST_UNORDERED_DEFAULT_TYPE(T,tname, arg)                   \
    typename default_type_ ## tname<T, arg>::type

    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(pointer);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(const_pointer);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(void_pointer);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(const_void_pointer);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(difference_type);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(size_type);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(propagate_on_container_copy_assignment);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(propagate_on_container_move_assignment);
    BOOST_UNORDERED_DEFAULT_TYPE_TMPLT(propagate_on_container_swap);

#   if !defined(BOOST_NO_SFINAE_EXPR)

    template <typename T>
    BOOST_UNORDERED_HAS_FUNCTION(
        select_on_container_copy_construction, U const, (), 0
    );

    template <typename T>
    BOOST_UNORDERED_HAS_FUNCTION(
        max_size, U const, (), 0
    );

#       if !defined(BOOST_NO_VARIADIC_TEMPLATES)

    template <typename T, typename ValueType, typename... Args>
    BOOST_UNORDERED_HAS_FUNCTION(
    construct, U, (
        boost::unordered::detail::make<ValueType*>(),
        boost::unordered::detail::make<Args const>()...), 2
    );

#       else

    template <typename T, typename ValueType>
    BOOST_UNORDERED_HAS_FUNCTION(
    construct, U, (
        boost::unordered::detail::make<ValueType*>(),
        boost::unordered::detail::make<ValueType const>()), 2
    );

#       endif

    template <typename T, typename ValueType>
    BOOST_UNORDERED_HAS_FUNCTION(
        destroy, U, (boost::unordered::detail::make<ValueType*>()), 1
    );

#   else

    template <typename T>
    BOOST_UNORDERED_HAS_MEMBER(select_on_container_copy_construction);

    template <typename T>
    BOOST_UNORDERED_HAS_MEMBER(max_size);

    template <typename T, typename ValueType>
    BOOST_UNORDERED_HAS_MEMBER(construct);

    template <typename T, typename ValueType>
    BOOST_UNORDERED_HAS_MEMBER(destroy);

#   endif

    template <typename Alloc>
    inline typename boost::enable_if_c<
            boost::unordered::detail::
            has_select_on_container_copy_construction<Alloc>::value, Alloc
        >::type call_select_on_container_copy_construction(const Alloc& rhs)
    {
        return rhs.select_on_container_copy_construction();
    }

    template <typename Alloc>
    inline typename boost::disable_if_c<
            boost::unordered::detail::
            has_select_on_container_copy_construction<Alloc>::value, Alloc
        >::type call_select_on_container_copy_construction(const Alloc& rhs)
    {
        return rhs;
    }

    template <typename SizeType, typename Alloc>
    inline typename boost::enable_if_c<
            boost::unordered::detail::has_max_size<Alloc>::value, SizeType
        >::type call_max_size(const Alloc& a)
    {
        return a.max_size();
    }

    template <typename SizeType, typename Alloc>
    inline typename boost::disable_if_c<
            boost::unordered::detail::has_max_size<Alloc>::value, SizeType
        >::type call_max_size(const Alloc&)
    {
        return (std::numeric_limits<SizeType>::max)();
    }

    template <typename Alloc>
    struct allocator_traits
    {
        typedef Alloc allocator_type;
        typedef typename Alloc::value_type value_type;

        typedef BOOST_UNORDERED_DEFAULT_TYPE(Alloc, pointer, value_type*)
            pointer;

        template <typename T>
        struct pointer_to_other : boost::pointer_to_other<pointer, T> {};

        typedef BOOST_UNORDERED_DEFAULT_TYPE(Alloc, const_pointer,
            typename pointer_to_other<const value_type>::type)
            const_pointer;

        //typedef BOOST_UNORDERED_DEFAULT_TYPE(Alloc, void_pointer,
        //    typename pointer_to_other<void>::type)
        //    void_pointer;
        //
        //typedef BOOST_UNORDERED_DEFAULT_TYPE(Alloc, const_void_pointer,
        //    typename pointer_to_other<const void>::type)
        //    const_void_pointer;

        typedef BOOST_UNORDERED_DEFAULT_TYPE(Alloc, difference_type,
            std::ptrdiff_t) difference_type;

        typedef BOOST_UNORDERED_DEFAULT_TYPE(Alloc, size_type, std::size_t)
            size_type;

        // TODO: rebind_alloc and rebind_traits

        static pointer allocate(Alloc& a, size_type n)
            { return a.allocate(n); }

        // I never use this, so I'll just comment it out for now.
        //
        //static pointer allocate(Alloc& a, size_type n,
        //        const_void_pointer hint)
        //    { return DEFAULT_FUNC(allocate, pointer)(a, n, hint); }

        static void deallocate(Alloc& a, pointer p, size_type n)
            { a.deallocate(p, n); }

    public:

#   if BOOST_UNORDERED_DETAIL_FULL_CONSTRUCT

        template <typename T, typename... Args>
        static typename boost::enable_if_c<
                boost::unordered::detail::has_construct<Alloc, T, Args...>
                ::value>::type
            construct(Alloc& a, T* p, BOOST_FWD_REF(Args)... x)
        {
            a.construct(p, boost::forward<Args>(x)...);
        }

        template <typename T, typename... Args>
        static typename boost::disable_if_c<
                boost::unordered::detail::has_construct<Alloc, T, Args...>
                ::value>::type
            construct(Alloc&, T* p, BOOST_FWD_REF(Args)... x)
        {
            new ((void*) p) T(boost::forward<Args>(x)...);
        }

        template <typename T>
        static typename boost::enable_if_c<
                boost::unordered::detail::has_destroy<Alloc, T>::value>::type
            destroy(Alloc& a, T* p)
        {
            a.destroy(p);
        }

        template <typename T>
        static typename boost::disable_if_c<
                boost::unordered::detail::has_destroy<Alloc, T>::value>::type
            destroy(Alloc&, T* p)
        {
            boost::unordered::detail::destroy(p);
        }

#   elif !defined(BOOST_NO_SFINAE_EXPR)

        template <typename T>
        static typename boost::enable_if_c<
                boost::unordered::detail::has_construct<Alloc, T>::value>::type
            construct(Alloc& a, T* p, T const& x)
        {
            a.construct(p, x);
        }

        template <typename T>
        static typename boost::disable_if_c<
                boost::unordered::detail::has_construct<Alloc, T>::value>::type
            construct(Alloc&, T* p, T const& x)
        {
            new ((void*) p) T(x);
        }

        template <typename T>
        static typename boost::enable_if_c<
                boost::unordered::detail::has_destroy<Alloc, T>::value>::type
            destroy(Alloc& a, T* p)
        {
            a.destroy(p);
        }

        template <typename T>
        static typename boost::disable_if_c<
                boost::unordered::detail::has_destroy<Alloc, T>::value>::type
            destroy(Alloc&, T* p)
        {
            boost::unordered::detail::destroy(p);
        }

#   else

        // If we don't have SFINAE expressions, only call construct for the
        // copy constructor for the allocator's value_type - as that's
        // the only construct method that old fashioned allocators support.

        template <typename T>
        static typename boost::enable_if_c<
                boost::unordered::detail::has_construct<Alloc, T>::value &&
                boost::is_same<T, value_type>::value
            >::type
            construct(Alloc& a, T* p, T const& x)
        {
            a.construct(p, x);
        }

        template <typename T>
        static typename boost::disable_if_c<
                boost::unordered::detail::has_construct<Alloc, T>::value &&
                boost::is_same<T, value_type>::value
            >::type
            construct(Alloc&, T* p, T const& x)
        {
            new ((void*) p) T(x);
        }

        template <typename T>
        static typename boost::enable_if_c<
                boost::unordered::detail::has_destroy<Alloc, T>::value &&
                boost::is_same<T, value_type>::value
            >::type
            destroy(Alloc& a, T* p)
        {
            a.destroy(p);
        }

        template <typename T>
        static typename boost::disable_if_c<
                boost::unordered::detail::has_destroy<Alloc, T>::value &&
                boost::is_same<T, value_type>::value
            >::type
            destroy(Alloc&, T* p)
        {
            boost::unordered::detail::destroy(p);
        }

#   endif

        static size_type max_size(const Alloc& a)
        {
            return boost::unordered::detail::call_max_size<size_type>(a);
        }

        // Allocator propagation on construction

        static Alloc select_on_container_copy_construction(Alloc const& rhs)
        {
            return boost::unordered::detail::
                call_select_on_container_copy_construction(rhs);
        }

        // Allocator propagation on assignment and swap.
        // Return true if lhs is modified.
        typedef BOOST_UNORDERED_DEFAULT_TYPE(
            Alloc, propagate_on_container_copy_assignment, false_type)
            propagate_on_container_copy_assignment;
        typedef BOOST_UNORDERED_DEFAULT_TYPE(
            Alloc,propagate_on_container_move_assignment, false_type)
            propagate_on_container_move_assignment;
        typedef BOOST_UNORDERED_DEFAULT_TYPE(
            Alloc,propagate_on_container_swap,false_type)
            propagate_on_container_swap;
    };
}}}

#   undef BOOST_UNORDERED_DEFAULT_TYPE_TMPLT
#   undef BOOST_UNORDERED_DEFAULT_TYPE

////////////////////////////////////////////////////////////////////////////////
//
// std::allocator_traits

#elif BOOST_UNORDERED_USE_ALLOCATOR_TRAITS == 1

#   include <memory>

#   define BOOST_UNORDERED_DETAIL_FULL_CONSTRUCT 1

namespace boost { namespace unordered { namespace detail {

    template <typename Alloc>
    struct allocator_traits : std::allocator_traits<Alloc> {};

    template <typename Alloc, typename T>
    struct rebind_wrap
    {
        typedef typename std::allocator_traits<Alloc>::
            template rebind_alloc<T> type;
    };
}}}

////////////////////////////////////////////////////////////////////////////////
//
// boost::container::allocator_traits

#elif BOOST_UNORDERED_USE_ALLOCATOR_TRAITS == 2

#   include <boost/container/allocator_traits.hpp>

#   define BOOST_UNORDERED_DETAIL_FULL_CONSTRUCT 0

namespace boost { namespace unordered { namespace detail {

    template <typename Alloc>
    struct allocator_traits :
        boost::container::allocator_traits<Alloc> {};

    template <typename Alloc, typename T>
    struct rebind_wrap :
        boost::container::allocator_traits<Alloc>::
            template portable_rebind_alloc<T>
    {};

}}}

#else

#error "Invalid BOOST_UNORDERED_USE_ALLOCATOR_TRAITS value."

#endif

////////////////////////////////////////////////////////////////////////////////
//
// Some helper functions for allocating & constructing

namespace boost { namespace unordered { namespace detail {

    ////////////////////////////////////////////////////////////////////////////
    //
    // construct_node/destroy_node
    //
    // Construct a node using the best available method.

#if BOOST_UNORDERED_DETAIL_FULL_CONSTRUCT

    template <typename Alloc, typename T, BOOST_UNORDERED_EMPLACE_TEMPLATE>
    inline void construct_node(Alloc& a, T* p, BOOST_UNORDERED_EMPLACE_ARGS)
    {
        boost::unordered::detail::allocator_traits<Alloc>::construct(
            a, p, BOOST_UNORDERED_EMPLACE_FORWARD);
    }

    template <typename Alloc, typename T>
    inline void destroy_node(Alloc& a, T* p)
    {
        boost::unordered::detail::allocator_traits<Alloc>::destroy(a, p);
    }

#else

    template <typename Alloc, typename T, BOOST_UNORDERED_EMPLACE_TEMPLATE>
    inline void construct_node(Alloc& a, T* p, BOOST_UNORDERED_EMPLACE_ARGS)
    {
        boost::unordered::detail::allocator_traits<Alloc>::construct(a, p, T());
        try {
            boost::unordered::detail::construct_impl(
                p->value_ptr(), BOOST_UNORDERED_EMPLACE_FORWARD);
        } catch(...) {
            boost::unordered::detail::allocator_traits<Alloc>::destroy(a, p);
            throw;
        }
    }

    template <typename Alloc, typename T>
    inline void destroy_node(Alloc& a, T* p)
    {
        boost::unordered::detail::destroy(p->value_ptr());
        boost::unordered::detail::allocator_traits<Alloc>::destroy(a, p);
    }

#endif

    ////////////////////////////////////////////////////////////////////////////
    //
    // array_constructor
    //
    // Allocate and construct an array in an exception safe manner, and
    // clean up if an exception is thrown before the container takes charge
    // of it.

    template <typename Allocator>
    struct array_constructor
    {
        typedef boost::unordered::detail::allocator_traits<Allocator> traits;
        typedef typename traits::pointer pointer;

        Allocator& alloc_;
        pointer ptr_;
        pointer constructed_;
        std::size_t length_;

        array_constructor(Allocator& a)
            : alloc_(a), ptr_(), constructed_(), length_(0)
        {
            constructed_ = pointer();
            ptr_ = pointer();
        }

        ~array_constructor() {
            if (ptr_) {
                for(pointer p = ptr_; p != constructed_; ++p)
                    traits::destroy(alloc_, boost::addressof(*p));

                traits::deallocate(alloc_, ptr_, length_);
            }
        }

        template <typename V>
        void construct(V const& v, std::size_t l)
        {
            BOOST_ASSERT(!ptr_);
            length_ = l;
            ptr_ = traits::allocate(alloc_, length_);
            pointer end = ptr_ + static_cast<std::ptrdiff_t>(length_);
            for(constructed_ = ptr_; constructed_ != end; ++constructed_)
                traits::construct(alloc_, boost::addressof(*constructed_), v);
        }

        pointer get() const
        {
            return ptr_;
        }

        pointer release()
        {
            pointer p(ptr_);
            ptr_ = pointer();
            return p;
        }

    private:

        array_constructor(array_constructor const&);
        array_constructor& operator=(array_constructor const&);
    };
}}}

#if defined(BOOST_MSVC)
#pragma warning(pop)
#endif

#endif
