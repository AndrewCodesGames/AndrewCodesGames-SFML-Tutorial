#include "Entity.h"
#include "MathHelpers.h"

Entity::Entity(PhysicsData::Type ePhysicsType)
	: m_bDeletionRequested(false)
	, m_fAxeTimer(3.0f)
	, m_fAttackTimer(1.0f)
{
	m_PhysicsData.m_eType = ePhysicsType;
}

void Entity::OnCollision(Entity& otherEntity)
{
	// If the other entity is an enemy
	if (otherEntity.GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Enemy))
	{
		// If we're a projectile
		if (GetPhysicsData().IsInAnyLayer(PhysicsData::Layer::Projectile))
		{
			// Find the direction from the projectile to the enemy
			sf::Vector2f direction = otherEntity.GetPosition() - GetPosition();
			direction = MathHelpers::Normalize(direction);
			otherEntity.GetPhysicsDataNonConst().AddImpulse(direction * 80.0f);

			// This means a projectil hit an enemy
			otherEntity.DealDamage(1);
			m_bDeletionRequested = true;
		}
	}
}
