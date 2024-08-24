#include "GizmoQueryStorage.h"
#include <cassert>

void GizmoQueryStorage::AddGizmo(Gizmo* gizmo)
{
	assert(gizmo != nullptr);
	_gizmoQueries.push_back(gizmo);
}

const std::vector<Gizmo*>& GizmoQueryStorage::GetGizmos() const
{
	return _gizmoQueries;
}

void GizmoQueryStorage::ClearGizmos()
{
	auto it = _gizmoQueries.begin();
	for (; it != _gizmoQueries.end(); ++it)
	{
		delete* it;
	}

	_gizmoQueries.clear();
}
