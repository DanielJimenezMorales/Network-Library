#include "CollisionDetectionSystem.h"
#include "EntityContainer.h"
#include "TransformComponent.h"
#include "GameEntity.hpp"
#include "CollisionUtils.h"
#include "Logger.h"
#include <vector>

#include "Collider2DComponent.h"

void CollisionDetectionSystem::PreTick(EntityContainer& entityContainer, float elapsedTime) const
{
	std::vector<GameEntity> collision_entities = entityContainer.GetEntitiesOfBothTypes < Collider2DComponent, TransformComponent>();

	for (uint32_t i = 0; i < collision_entities.size(); ++i)
	{
		const Collider2DComponent& colliderA = collision_entities[i].GetComponent<Collider2DComponent>();
		TransformComponent& transformA = collision_entities[i].GetComponent<TransformComponent>();

		for (uint32_t j = i + 1; j < collision_entities.size(); ++j)
		{
			const Collider2DComponent& colliderB = collision_entities[j].GetComponent<Collider2DComponent>();
			TransformComponent& transformB = collision_entities[j].GetComponent<TransformComponent>();

			if (AreTwoShapesColliding(colliderA, transformA, colliderB, transformB))
			{
				LOG_WARNING("OVERLAP");
			}
		}
	}
}

void GetAllAxes(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2, std::vector<Vec2f>& outAxesVector)
{
	if (collider1.GetShapeType() == CollisionShapeType::Convex && collider2.GetShapeType() == CollisionShapeType::Convex)
	{
		collider1.GetAxes(transform1, outAxesVector);
		collider2.GetAxes(transform2, outAxesVector);
	}
	else if (collider1.GetShapeType() == CollisionShapeType::Convex && collider2.GetShapeType() == CollisionShapeType::Circle)
	{
		collider1.GetAxes(transform1, outAxesVector);
		const Vec2f centerToClosestVertexAxis = collider1.GetClosestVertex(transform1, transform2.GetPosition());
		outAxesVector.push_back(centerToClosestVertexAxis);
	}
	else if (collider1.GetShapeType() == CollisionShapeType::Circle && collider2.GetShapeType() == CollisionShapeType::Convex)
	{
		const Vec2f centerToClosestVertexAxis = collider2.GetClosestVertex(transform2, transform1.GetPosition());
		outAxesVector.push_back(centerToClosestVertexAxis);
		collider2.GetAxes(transform2, outAxesVector);
	}
	else if (collider1.GetShapeType() == CollisionShapeType::Circle && collider2.GetShapeType() == CollisionShapeType::Circle)
	{
		const Vec2f centerToCenterAxis = transform2.GetPosition() - transform1.GetPosition();
		outAxesVector.push_back(centerToCenterAxis);
	}
}

void NormalizeAxes(std::vector<Vec2f>& axesVector)
{
	auto it = axesVector.begin();
	for (; it != axesVector.end(); ++it)
	{
		it->Normalize();
	}
}

bool AreTwoShapesColliding(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2)
{
	std::vector<Vec2f> axesToCheck;
	GetAllAxes(collider1, transform1, collider2, transform2, axesToCheck);
	NormalizeAxes(axesToCheck);

	auto cit = axesToCheck.cbegin();
	for (; cit != axesToCheck.cend(); ++cit)
	{
		float minCollider1, maxCollider1, minCollider2, maxCollider2 = 0.f;
		collider1.ProjectAxis(transform1, *cit, minCollider1, maxCollider1);
		collider2.ProjectAxis(transform2, *cit, minCollider2, maxCollider2);

		bool doProjectionsOverlap = (maxCollider1 >= minCollider2) && (maxCollider2 >= minCollider1);

		if (!doProjectionsOverlap)
		{
			return false;
		}
	}

	return true;
}
