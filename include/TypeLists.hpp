/*
 * _COPYRIGHT_
 */

#ifndef _TYPE_LISTS_HPP__
#define _TYPE_LISTS_HPP__

#include <Types.h>
#include <set>
#include <algorithm>

namespace Util {
	namespace TL {

		class null_type {
		};

		template<class T, class Right>
		struct TypeList {
			typedef T type_hold;
			T arg;

			typedef Right type_right;
			Right right;

			enum{ length=1+Right::length };

			bool operator<(const TypeList& r) const
			{
				if (arg < r.arg) return true;
				if (arg > r.arg) return false;
				return right<r.right;
			}
		};

		template<class T>
		struct TypeList<T, null_type> {
			typedef T type_hold;
			T arg;

			typedef null_type type_right;

			enum{ length=1 };

			bool operator<(const TypeList& r) const
			{
				return arg < r.arg;
			}
		};
		template<>
		struct TypeList<null_type, null_type> {
			typedef null_type type_hold;
			typedef null_type type_right;
			enum{ length=0 };
		};

		// Add type to list tail
		template<class List, class Type>
		struct AddTail {
			typedef TypeList<
				typename List::type_hold,
				typename AddTail<typename List::type_right, Type>::Result
			> Result;
		};
		template<class List, class Type>
		struct AddTail<TypeList<List, null_type>, Type> {
			typedef TypeList<
				List,
				TypeList<Type, null_type>
			> Result;
		};
		template<class Type>
		struct AddTail<TypeList<null_type, null_type>, Type> {
			typedef TypeList<Type, null_type> Result;
		};

		// call access function over all ements
		template<class List, class F>
		struct Apply4All{
			static void f(List& t, F& f, INT_32 len){
				f(t.arg, len);
				Apply4All<typename List::type_right,F>::f(t.right, f, len+1);
			}
			static void f(List& t, F& f){
				f(t.arg);
				Apply4All<typename List::type_right,F>::f(t.right, f);
			}

		};

		template<class List, class F>
		struct Apply4All<TypeList<List, null_type>, F>{
			static void f(TypeList<List, null_type>& t, F& f, INT_32 len){
				f(t.arg, len);
			}
			static void f(TypeList<List, null_type>& t, F& f){
				f(t.arg);
			}

		};

		template<class List, class F>
		void ForEachN(List& list, F& f){
			Apply4All<List,F>::f(list, f, 0);
		}
		template<class List, class F>
		void ForEach(List& list, F& f){
			Apply4All<List,F>::f(list, f);
		}

		// call F over 2 lists in parallel order
		template<class Key, class Idx, class F>
		struct Apply4All2{
			static void f(Key& key, Idx& idx, F& f){
				f(key.arg, idx.arg);
				Apply4All2< typename Key::type_right,
							typename Idx::type_right,
							F
				>::f(key.right, idx.right, f);
			}
		};

		// FIXME: pass key.arg and idx.arg to f
		template<class Key, class Idx, class F>
		struct Apply4All2<TypeList<Key, null_type>,
						  TypeList<Idx, null_type>,
						 F>
		{
			static void f(TypeList<Key, null_type>& key,
						  TypeList<Idx, null_type>& idx,
						  F& f)
			{
				f(key.arg, idx.arg);
			}
		};

		template<class Key, class Idx, class F>
		void ForEach2(Key& key, Idx& idx, F& f){
			Apply4All2<Key, Idx, F>::f(key, idx, f);
		}

		// main Tuple creation
		template
			<
			typename T1  = null_type, typename T2  = null_type, typename T3  = null_type,
			typename T4  = null_type, typename T5  = null_type, typename T6  = null_type,
			typename T7  = null_type, typename T8  = null_type, typename T9  = null_type,
			typename T10 = null_type, typename T11 = null_type, typename T12 = null_type,
			typename T13 = null_type, typename T14 = null_type, typename T15 = null_type,
			typename T16 = null_type, typename T17 = null_type, typename T18 = null_type
			>
		struct MakeTypelist
		{
			private:
				typedef typename MakeTypelist
					<
					T2 , T3 , T4 ,
					T5 , T6 , T7 ,
					T8 , T9 , T10,
					T11, T12, T13,
					T14, T15, T16,
					T17, T18
				> ::Result TailResult;
			public:
				 typedef TypeList<T1, TailResult> Result;
		 };

		template<>
		struct MakeTypelist<>
		{
			typedef null_type Result;
		};

		// use c++0x variadic template to simplify variable access
		template<class List, class T1, typename... Tail>
		void SetArg(List& list, T1 t1, Tail... tail)
		{
			list.arg = t1;
			SetArg(list.right, tail...);
		}
		template<class List, class T1>
		void SetArg(List& list, T1 t1)
		{
			list.arg = t1;
		}

		// get type of num- element
		template<class List, UINT_32 num>
		struct GetType{
			typedef typename GetType<typename List::type_right, num-1>::type_hold type_hold;
		};

		template<class List>
		struct GetType<List, 0>{
			typedef typename List::type_hold type_hold;
		};

		// apply function to specific element
		template<class List,class F, UINT_32 num>
		struct Recurser{
			static void f(List& t, F& f){
				Recurser<typename List::type_right,F,num - 1>::f(t.right,f);
			}
		};

		template<class List, class F>
		struct Recurser<List, F, 0>{
			static void f(List& t, F& f){
				f(t.arg);
			}
		};

		// GetRef helper

		template<class T>
		struct ValRef {
			T* t;

			ValRef() : t(NULL) {}
			void operator()(T& arg){
				t = &arg;
			}
		};

		// get reference to num- element
		template<UINT_32 num>
		struct GetRef {
			template<class List>
			static
			typename GetType<List, num>::type_hold&
			f(List& list)
			{
				typedef typename GetType<List, num>::type_hold RetType;
				ValRef<RetType> getref;
				Recurser<List, ValRef<RetType>, num>::f(list, getref);
				return *getref.t;
			}
		};

		template<UINT_32 num, class List>
		typename GetType<List, num>::type_hold&
		get(List& l)
		{
			return GetRef<num>::f(l);
		}
		template<UINT_32 num, class List>
		const typename GetType<List, num>::type_hold&
		get(const List& l)
		{
			List* l1 = const_cast<List*>(&l);
			return GetRef<num>::f(*l1);
		}

		// convert TL to another one
		template<class List, template<typename>class D>
		struct ForEachT {
			typedef TypeList<
				typename D<typename List::type_hold>::Result,
				typename ForEachT<typename List::type_right, D>::Result
			> Result;
		};
		template<class List, template<typename>class D>
		struct ForEachT<TypeList<List, null_type>, D>
		{
			typedef TypeList<
				typename D<List>::Result,
				null_type
			> Result;
		};

} // namespace TL
} // namespace Util

#endif /* _TYPE_LISTS_HPP__ */


