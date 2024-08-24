#pragma once
#include "NullTextureLoader.h"
#include "NullGizmoQueryStorage.h"

class ITextureLoader;
class IGizmoQueryStorage;

class ServiceLocator
{
public:
	ServiceLocator() : _textureLoader(nullptr), _nullTextureLoader(), _gizmoQueryStorage(nullptr), _nullGizmoQueryStorage() {}
	~ServiceLocator() {}

	static ServiceLocator& CreateInstance();
	static ServiceLocator& GetInstance();
	static void DestroyInstance();

	ITextureLoader& GetTextureLoader();
	void RegisterTextureLoader(ITextureLoader* textureLoader);
	void UnregisterTextureLoader();

	IGizmoQueryStorage& GetGizmoQueryStorage();
	void RegisterGizmoQueryStorage(IGizmoQueryStorage* gizmoQueryStorage);
	void UnregisterGizmoQueryStorage();

	void UnregisterAll();

private:
	static ServiceLocator* _instance;

	ITextureLoader* _textureLoader;
	NullTextureLoader _nullTextureLoader;

	IGizmoQueryStorage* _gizmoQueryStorage;
	NullGizmoQueryStorage _nullGizmoQueryStorage;
};