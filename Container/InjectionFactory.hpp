#pragma once

#include "Injector.hpp"
#include <functional>
#include <memory>
#include <tuple>
#include <type_traits>

namespace Epic::IoC
{
	class Container;

	class IDependencyFactory;
	template<class T> class DependencyFactory;
	template<class T> class NewDependencyFactory;
	template<class T> class FunctionDependencyFactory;
	template<class T> class InjectDependencyFactory;
}

class Epic::IoC::IDependencyFactory
{
public:
	virtual ~IDependencyFactory() = default;
};

template<class T>
class Epic::IoC::DependencyFactory : public IDependencyFactory
{
public:
	virtual ~DependencyFactory() = default;

public:
	virtual T* Create() = 0;
};

template<class T>
class Epic::IoC::NewDependencyFactory : public DependencyFactory<T>
{
public:
	T* Create() override
	{
		return new T();
	}
};

template<class T>
class Epic::IoC::FunctionDependencyFactory : public DependencyFactory<T>
{
private:
	std::function<T* ()> _factoryFn;

public:
	template<class F>
	FunctionDependencyFactory(F&& resolverFn)
		: _factoryFn{ std::forward<F>(resolverFn) }
	{ }

public:
	T* Create() override
	{
		return _factoryFn();
	}
};

template<class T>
class Epic::IoC::InjectDependencyFactory : public DependencyFactory<T>
{
private:
	Container* _pContainer;

public:
	InjectDependencyFactory(Container* pContainer)
		: _pContainer(pContainer)
	{ }

private:
	template<size_t... Ns>
	constexpr auto MakeInjectorTuple(std::index_sequence<Ns...>)
	{
		return std::make_tuple(detail::DependencyInjector<Ns>{ _pContainer }...);
	}

	constexpr auto MakeInjectorTuple()
	{
		return MakeInjectorTuple(std::make_index_sequence<detail::GetConstructorArgCount<T>()>{});
	}

public:
	T* Create() override
	{
		auto injectedNew = [&](auto&&... dependencies)
		{
			return new T(std::forward<decltype(dependencies)>(dependencies)...);
		};

		return std::apply(injectedNew, MakeInjectorTuple());
	}
};
