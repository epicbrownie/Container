#pragma once

#include "InjectionFactory.hpp"

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <cassert>

namespace Epic::IoC
{
	class Container;
}

/////
class Epic::IoC::Container
{
private:
	using FactoryPtr = std::unique_ptr<IDependencyFactory>;
	using FactoryMap = std::unordered_map<std::type_index, FactoryPtr>;

private:
	FactoryMap _Registry;

public:
	template<class Contract, class Implementation>
	void Register()
	{
		auto typeIndex = std::type_index(typeid(Contract));

		assert(_Registry.find(typeIndex) == _Registry.cend());

		if constexpr (std::is_default_constructible_v<Implementation>)
		{
			auto pFactory = std::make_unique<NewDependencyFactory<Implementation>>();
			_Registry[typeIndex] = std::move(pFactory);
		}
		else
		{
			auto pFactory = std::make_unique<InjectDependencyFactory<Implementation>>(this);
			_Registry[typeIndex] = std::move(pFactory);
		}
	}

	template<class Type>
	void Register()
	{
		Register<Type, Type>();
	}

	template<class Contract, class Callable>
	void Register(Callable&& factory)
	{
		auto typeIndex = std::type_index(typeid(Contract));

		assert(_Registry.find(typeIndex) == _Registry.cend());

		auto pFactory = std::make_unique<FunctionDependencyFactory<Contract>>(std::forward<Callable>(factory));

		_Registry[typeIndex] = std::move(pFactory);
	}

	template<class Contract>
	Contract* Resolve()
	{
		auto typeIndex = std::type_index(typeid(Contract));

		assert(_Registry.find(typeIndex) != _Registry.cend());

		auto pFactory = static_cast<DependencyFactory<Contract>*>(_Registry[typeIndex].get());

		return pFactory->Create();
	}
};
