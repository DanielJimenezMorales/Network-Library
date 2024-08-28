#include "CollisionDetectionSystem.h"
#include "EntityContainer.h"
#include "TransformComponent.h"
#include "GameEntity.hpp"
#include "CollisionUtils.h"
#include "Logger.h"

#include "Collider2DComponent.h"

void CollisionDetectionSystem::PreTick(EntityContainer& entityContainer, float elapsedTime) const
{
	std::vector<GameEntity> collision_entities = entityContainer.GetEntitiesOfBothTypes < Collider2DComponent, TransformComponent>();
	SortCollidersByLeft(collision_entities);

	for (uint32_t i = 0; i < collision_entities.size(); ++i)
	{
		const Collider2DComponent& colliderA = collision_entities[i].GetComponent<Collider2DComponent>();
		TransformComponent& transformA = collision_entities[i].GetComponent<TransformComponent>();

		for (uint32_t j = i + 1; j < collision_entities.size(); ++j)
		{
			const Collider2DComponent& colliderB = collision_entities[j].GetComponent<Collider2DComponent>();
			TransformComponent& transformB = collision_entities[j].GetComponent<TransformComponent>();

			if (colliderA.GetMaxX(transformA) < colliderB.GetMinX(transformB))
			{
				break;
			}

			MinimumTranslationVector mtv;
			if (AreTwoShapesColliding(colliderA, transformA, colliderB, transformB, mtv))
			{
				LOG_WARNING("OVERLAP: MTV: %.2f", mtv.magnitude);
				if (!colliderA.IsTrigger() && !colliderB.IsTrigger())
				{
					ApplyCollisionResponse(colliderA, transformA, colliderB, transformB, mtv);
				}
			}
		}
	}
}

bool CollisionDetectionSystem::AreTwoShapesColliding(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2, MinimumTranslationVector& outMtv) const
{
	std::vector<Vec2f> axesToCheck;
	GetAllAxes(collider1, transform1, collider2, transform2, axesToCheck);
	NormalizeAxes(axesToCheck);

	Vec2f smallestAxis;
	float smallestOverlapMagnitude = 200000.f; //TODO Set this to max float or something like that
	auto cit = axesToCheck.cbegin();
	for (; cit != axesToCheck.cend(); ++cit)
	{
		float minCollider1, maxCollider1, minCollider2, maxCollider2 = 0.f;
		collider1.ProjectAxis(transform1, *cit, minCollider1, maxCollider1);
		collider2.ProjectAxis(transform2, *cit, minCollider2, maxCollider2);

		if (!DoProjectionsOverlap(minCollider1, maxCollider1, minCollider2, maxCollider2))
		{
			return false;
		}
		else
		{
			float projectionOverlapMagnitude = GetProjectionsOverlapMagnitude(minCollider1, maxCollider1, minCollider2, maxCollider2);
			if (projectionOverlapMagnitude < smallestOverlapMagnitude)
			{
				smallestAxis = *cit;
				smallestOverlapMagnitude = projectionOverlapMagnitude;
			}
		}
	}

	outMtv.direction = smallestAxis;
	outMtv.magnitude = smallestOverlapMagnitude;
	return true;
}

void CollisionDetectionSystem::SortCollidersByLeft(std::vector<GameEntity>& collider_entities) const
{
	std::sort(collider_entities.begin(), collider_entities.end(), ReturnMinLeft);
}

bool ReturnMinLeft(const GameEntity& colliderEntityA, const GameEntity& colliderEntityB)
{
	const Collider2DComponent& colliderA = colliderEntityA.GetComponent<Collider2DComponent>();
	const TransformComponent& transformA = colliderEntityA.GetComponent<TransformComponent>();

	const Collider2DComponent& colliderB = colliderEntityB.GetComponent<Collider2DComponent>();
	const TransformComponent& transformB = colliderEntityB.GetComponent<TransformComponent>();

	return colliderA.GetMinX(transformA) < colliderB.GetMinX(transformB);
}

void CollisionDetectionSystem::GetAllAxes(const Collider2DComponent& collider1, const TransformComponent& transform1, const Collider2DComponent& collider2, const TransformComponent& transform2, std::vector<Vec2f>& outAxesVector) const
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

void CollisionDetectionSystem::NormalizeAxes(std::vector<Vec2f>& axesVector) const
{
	auto it = axesVector.begin();
	for (; it != axesVector.end(); ++it)
	{
		it->Normalize();
	}
}

bool CollisionDetectionSystem::DoProjectionsOverlap(float minProjection1, float maxProjection1, float minProjection2, float maxProjection2) const
{
	return (maxProjection1 >= minProjection2) && (maxProjection2 >= minProjection1);
}

float CollisionDetectionSystem::GetProjectionsOverlapMagnitude(float minProjection1, float maxProjection1, float minProjection2, float maxProjection2) const
{
	return std::min(maxProjection1, maxProjection2) - std::max(minProjection1, minProjection2);
}

void CollisionDetectionSystem::ApplyCollisionResponse(const Collider2DComponent& collider1, TransformComponent& transform1, const Collider2DComponent& collider2, TransformComponent& transform2, const MinimumTranslationVector& mtv) const
{
	Vec2f resultedTranslationVector = mtv.direction * mtv.magnitude;

	if (collider1.GetCollisionResponse() == CollisionResponseType::Dynamic && collider2.GetCollisionResponse() == CollisionResponseType::Static)
	{
		transform1.SetPosition(transform1.GetPosition() - resultedTranslationVector);
	}
	else if (collider1.GetCollisionResponse() == CollisionResponseType::Static && collider2.GetCollisionResponse() == CollisionResponseType::Dynamic)
	{
		transform2.SetPosition(transform2.GetPosition() + resultedTranslationVector);
	}
	else if (collider1.GetCollisionResponse() == CollisionResponseType::Dynamic && collider2.GetCollisionResponse() == CollisionResponseType::Dynamic)
	{
		transform1.SetPosition(transform1.GetPosition() - (resultedTranslationVector / 2.f));
		transform2.SetPosition(transform2.GetPosition() + (resultedTranslationVector / 2.f));
	}
	else
	{
		LOG_ERROR("Collision response not supported");
	}
}
