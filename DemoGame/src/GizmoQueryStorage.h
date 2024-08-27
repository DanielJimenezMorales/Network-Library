#pragma once
#include "IGizmoQueryStorage.h"
#include <vector>

class Gizmo;

class GizmoQueryStorage : public IGizmoQueryStorage
{
public:
	GizmoQueryStorage() : IGizmoQueryStorage(), _gizmoQueries()
	{
	}

	void AddGizmo(Gizmo* gizmo) override;
	const std::vector<Gizmo*>& GetGizmos() const override;
	void ClearGizmos() override;

private:
	std::vector<Gizmo*> _gizmoQueries;
};