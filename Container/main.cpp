#include "Container.hpp"

#include <iostream>
#include <memory>
#include <string>


struct Printer
{ 
	virtual ~Printer() = default;
	virtual void Print(const std::string&) = 0;
};

struct MyPrinter : public Printer
{
	void Print(const std::string& data) override 
	{ 
		std::cout << data << std::endl; 
	}
};

struct Duplicator
{
	Printer* _pPrinter = nullptr;

	Duplicator(Printer* pPrinter) : _pPrinter(pPrinter) { }
	
	virtual ~Duplicator() { delete _pPrinter; }

	virtual void Duplicate(const std::string&) = 0;
};

struct MyDuplicator : public Duplicator
{
	MyDuplicator(Printer* pPrinter) : Duplicator(pPrinter) { }

	void Duplicate(const std::string& data) override
	{
		std::cout << "1. ";
		_pPrinter->Print(data);
		
		std::cout << "2. ";
		_pPrinter->Print(data);
		
		std::cout << "3. ";
		_pPrinter->Print(data);
	}
};

struct Reader
{
	virtual ~Reader() = default;
	virtual std::string Read() = 0;
};

struct MyReader : public Reader
{
	std::string Read() override 
	{ 
		std::string s; 
		std::getline(std::cin, s); 
		return s; 
	}
};

struct IOService
{
	Reader* _pReader;
	Duplicator* _pDuplicator;

	IOService(Reader* pReader, Duplicator* pDuplicator)
		: _pReader(pReader), _pDuplicator(pDuplicator)
	{ }

	~IOService()
	{
		delete _pReader;
		delete _pDuplicator;
	}

	void Handle()
	{
		_pDuplicator->Duplicate(_pReader->Read());
	}
};

int main()
{
	using namespace Epic::IoC;

	auto pContainer = std::make_unique<Container>();

	// Transient Abstract => Concrete
	pContainer->Register<Duplicator, MyDuplicator>();
	pContainer->Register<Printer, MyPrinter>();

	// Transient Concrete
	pContainer->Register<IOService>();

	// Factory Abstract
	pContainer->Register<Reader>([]() { return new MyReader(); });

	// Resolve with automatic dependency injection
	auto pService = pContainer->Resolve<IOService>();
	pService->Handle();
	delete pService;

	std::cin.get();
	return 0;
}