/*
 * _COPYRIGHT_
 */

#ifndef _TYPE_LISTS_HPP__
#define _TYPE_LISTS_HPP__

#include <stdint.h>
#include <tuple>
#include <set>
#include <algorithm>

namespace Util {
	namespace TL {

		// for tuple
		template<class T, typename>
		struct append_tail;

		template<class T, typename... Ts>
		struct append_tail<T, std::tuple<Ts...>>
		{
			using Result = std::tuple<Ts..., T>;
		};

		template<template <typename> class T, typename ... Ts>
		struct apply_t;
		template<template <typename> class T, typename ... Ts>
		struct apply_t<T, std::tuple<Ts...>>
		{
			using Result = std::tuple<typename T<Ts>::Result...>;
		};

		template<std::size_t I = 0, typename F, typename... Tp>
		inline typename std::enable_if<I == sizeof...(Tp), void>::type
		tuple_for_each(F& f, std::tuple<Tp...>& t)
		{ }

		template<std::size_t I = 0, typename F, typename... Tp>
		inline typename std::enable_if<I < sizeof...(Tp), void>::type
		tuple_for_each(F& f, std::tuple<Tp...>& t)
		{
			f(std::get<I>(t));
			tuple_for_each<I + 1, F, Tp...>(f, t);
		}

		template<std::size_t I = 0, typename F, typename T1, typename T2>
		inline typename std::enable_if<I == std::tuple_size<T1>::value, void>::type
		tuple_for_each2(F& f, T1& t1, T2& t2)
		{ }

		template<std::size_t I = 0, typename F, typename T1, typename T2>
		inline typename std::enable_if<I < std::tuple_size<T1>::value, void>::type
		tuple_for_each2(F& f, T1& t1, T2& t2)
		{
			f(std::get<I>(t1), std::get<I>(t2));
			tuple_for_each2<I + 1, F, T1, T2>(f, t1, t2);
		}

		template < typename... T >
		std::tuple<const T&...> ctie( const T&... args )
		{
			return std::tie( args... );
		}

	} // namespace TL
} // namespace Util

#endif /* _TYPE_LISTS_HPP__ */


