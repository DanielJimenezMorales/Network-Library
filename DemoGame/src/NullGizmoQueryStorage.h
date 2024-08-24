#pragma once
#include "IGizmoQueryStorage.h"
#include "Logger.h"

class NullGizmoQueryStorage : public IGizmoQueryStorage
{
public:
	NullGizmoQueryStorage() : IGizmoQueryStorage()
	{
	}

	void AddGizmo(Gizmo* gizmo) override
	{
		LOG_WARNING("NullGizmoQueryStorage calling to AddGizmo");
		if (gizmo != nullptr)
		{
			delete gizmo;
		}
	}

	const std::vector<Gizmo*>& GetGizmos() const override
	{
		LOG_WARNING("NullGizmoQueryStorage calling to GetGizmos");
		return std::vector<Gizmo*>();
	}

	void ClearGizmos() override
	{
		LOG_WARNING("NullGizmoQueryStorage calling to ClearGizmos");
	}
};
