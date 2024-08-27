#pragma once
#include <vector>

class Gizmo;

class IGizmoQueryStorage
{
public:
	IGizmoQueryStorage()
	{
	}

	virtual ~IGizmoQueryStorage() {}

	virtual void AddGizmo(Gizmo* gizmo) = 0;
	virtual const std::vector<Gizmo*>& GetGizmos() const = 0;
	virtual void ClearGizmos() = 0;
};
