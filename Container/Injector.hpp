#pragma once

#include "MetaProgramming.hpp"

#define ARGUMENT_LIMIT 40

namespace Epic::IoC
{
	class Container;
}

namespace Epic::IoC::detail
{
	template<class T> struct IsInjectableType;
	template<class T> struct InjectableType;

	template<class T, size_t N> struct ConstructorArg;

	template<class T, class... Args>
	using HasConstructor = decltype(T(std::declval<Args>()...));

	template<size_t N>
	class DependencyInjector;
}

/////

namespace Epic::IoC::detail
{
	namespace
	{
		template<class T, size_t... N>
		constexpr size_t GetConstructorArgCount(std::index_sequence<N...>) noexcept
		{
			if constexpr (Epic::is_detected<detail::HasConstructor, T, detail::ConstructorArg<T, N>...>::value)
			{
				return sizeof... (N);
			}
			else if constexpr (sizeof...(N) == ARGUMENT_LIMIT)
			{
				static_assert(false, "This type's constructor has exceeded the argument limit.");
			}
			else
				return GetConstructorArgCount<T>(std::make_index_sequence<sizeof... (N)+1>{});
		}

		template<class T>
		constexpr size_t GetConstructorArgCount() noexcept
		{
			if constexpr (std::is_default_constructible_v<T>)
				return 0;
			else
				return GetConstructorArgCount<T>(std::make_index_sequence<1>{});
		}
	}
}

/////

template<class T>
struct Epic::IoC::detail::IsInjectableType : std::false_type { };

template<class T>
struct Epic::IoC::detail::IsInjectableType<T*> : std::true_type { };

template<class T>
struct Epic::IoC::detail::InjectableType { using type = T; };

template<class T>
struct Epic::IoC::detail::InjectableType<T*> { using type = T; };

template<class T, size_t N>
struct Epic::IoC::detail::ConstructorArg
{
	template<class U, typename = std::enable_if_t<!std::is_same_v<U, T>, U>>
	constexpr operator U () const noexcept
	{
		if constexpr (!IsInjectableType<U>::value)
		{
			static_assert(false, "Constructor contains non-injectable type(s).");
		}

		return U{};
	}
};

/////

template<size_t N>
class Epic::IoC::detail::DependencyInjector
{
private:
	Container* _pContainer;

public:
	DependencyInjector(Container* pContainer)
	{
		_pContainer = pContainer;
	}

	template<class U>
	operator U*() const noexcept
	{
		return _pContainer->Resolve<U>();
	}
};
