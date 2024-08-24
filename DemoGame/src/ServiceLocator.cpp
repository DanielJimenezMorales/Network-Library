#include "ServiceLocator.h"
#include <cassert>

ServiceLocator& ServiceLocator::CreateInstance()
{
	assert(_instance == nullptr);
	_instance = new ServiceLocator();
	return *_instance;
}

ServiceLocator& ServiceLocator::GetInstance()
{
	assert(_instance != nullptr);
	return *_instance;
}

void ServiceLocator::DestroyInstance()
{
	assert(_instance != nullptr);
	delete _instance;
	_instance = nullptr;
}

ITextureLoader& ServiceLocator::GetTextureLoader()
{
	if (_textureLoader == nullptr)
	{
		return _nullTextureLoader;
	}

	return *_textureLoader;
}

void ServiceLocator::RegisterTextureLoader(ITextureLoader* textureLoader)
{
	assert(_textureLoader == nullptr);
	_textureLoader = textureLoader;
}

void ServiceLocator::UnregisterTextureLoader()
{
	assert(_textureLoader != nullptr);
	delete _textureLoader;
	_textureLoader = nullptr;
}

IGizmoQueryStorage& ServiceLocator::GetGizmoQueryStorage()
{
	if (_gizmoQueryStorage == nullptr)
	{
		return _nullGizmoQueryStorage;
	}

	return *_gizmoQueryStorage;
}

void ServiceLocator::RegisterGizmoQueryStorage(IGizmoQueryStorage* gizmoQueryStorage)
{
	assert(_gizmoQueryStorage == nullptr);
	_gizmoQueryStorage = gizmoQueryStorage;
}

void ServiceLocator::UnregisterGizmoQueryStorage()
{
	assert(_gizmoQueryStorage != nullptr);
	delete _gizmoQueryStorage;
	_gizmoQueryStorage = nullptr;
}

void ServiceLocator::UnregisterAll()
{
	UnregisterTextureLoader();
	UnregisterGizmoQueryStorage();
}

ServiceLocator* ServiceLocator::_instance = nullptr;
