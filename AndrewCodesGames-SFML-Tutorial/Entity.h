#ifndef ENTITY_H
#define ENTITY_H

#include "SFML/System/Vector2.hpp"
#include "SFML/Graphics/Sprite.hpp"
#include "SFML/Graphics/RenderTarget.hpp"

#include <vector>

class Entity : public sf::Drawable
{
public:

	struct PhysicsData
	{
		PhysicsData()
		{
			m_vImpulse = sf::Vector2f(0.0f, 0.0f);
		}

		enum Layer
		{
			Enemy = 1,// 0b0001
			Tower = 2,// 0b0010
			Projectile = 4,// 0b0100
		};

		enum class Shape
		{
			Circle,
			Rectangle
		};

		enum class Type
		{
			Static,
			Dynamic
		};

		Shape m_eShape;
		Type m_eType;

		int m_iMyLayer;
		int m_iLayersToIgnore;

		void SetLayers(int layers)
		{
			m_iMyLayer = layers;
		}

		void SetLayersToIgnore(int layers)
		{
			m_iLayersToIgnore = layers;
		}

		int GetLayersToIgnore() const
		{
			return m_iLayersToIgnore;
		}

		bool IsInAnyLayer(int layer) const
		{
			return (m_iMyLayer & layer) != 0;
		}

		void ClearCollisions()
		{
			m_EntitiesThatICollidedWithAlready.clear();
		}

		bool HasCollidedThisUpdate(Entity* pOtherEntity) const
		{
			for (Entity* pEntity : m_EntitiesThatICollidedWithAlready)
			{
				if (pEntity == pOtherEntity)
				{
					return true;
				}
			}

			return false;
		}

		void AddEntityCollision(Entity* pOtherEntity)
		{
			m_EntitiesThatICollidedWithAlready.push_back(pOtherEntity);
		}

		void ClearImpulse()
		{
			m_vImpulse = sf::Vector2f(0.0f, 0.0f);
		}

		void AddImpulse(const sf::Vector2f& impulse)
		{
			m_vImpulse += impulse;
		}

		float m_fRadius;
		float m_fWidth;
		float m_fHeight;

		sf::Vector2f m_vVelocity;
		sf::Vector2f m_vImpulse;
		std::vector<Entity*> m_EntitiesToIgnore;
		std::vector<Entity*> m_EntitiesThatICollidedWithAlready;
	};

	Entity(PhysicsData::Type ePhysicsType);
	~Entity() {};

	void SetCirclePhysics(float radius)
	{
		m_PhysicsData.m_eShape = PhysicsData::Shape::Circle;
		m_PhysicsData.m_fRadius = radius;
	}

	void SetRectanglePhysics(float width, float height)
	{
		m_PhysicsData.m_eShape = PhysicsData::Shape::Rectangle;
		m_PhysicsData.m_fWidth = width;
		m_PhysicsData.m_fHeight = height;
	}

	void AddEntityToIgnore(Entity* entity)
	{
		m_PhysicsData.m_EntitiesToIgnore.push_back(entity);
	}

	bool ShouldIgnoreEntityForPhysics(Entity* entity)
	{
		for (Entity* pEntity : m_PhysicsData.m_EntitiesToIgnore)
		{
			if (pEntity == entity)
			{
				return true;
			}
		}

		if (entity->GetPhysicsData().IsInAnyLayer(m_PhysicsData.GetLayersToIgnore()))
		{
			return true;
		}

		return false;
	}

	void SetVelocity(const sf::Vector2f& velocity)
	{
		m_PhysicsData.m_vVelocity = velocity;
	}

	void SetTexture(const sf::Texture& texture)
	{
		m_Sprite.setTexture(texture);
	}

	void SetScale(const sf::Vector2f& scale)
	{
		m_Sprite.setScale(scale);
	}

	void SetOrigin(const sf::Vector2f& origin)
	{
		m_Sprite.setOrigin(origin);
	}

	void SetPosition(const sf::Vector2f& position)
	{
		m_Sprite.setPosition(position);
	}

	void SetColor(const sf::Color& color)
	{
		m_Sprite.setColor(color);
	}

	void SetSprite(const sf::Sprite& sprite)
	{
		m_Sprite = sprite;
	}

	sf::Vector2f GetPosition() const
	{
		return m_Sprite.getPosition();
	}

	sf::Vector2i GetClosestGridCoordinates() const
	{
		return sf::Vector2i(GetPosition().x / 160, GetPosition().y / 160);
	}

	const sf::Sprite& GetSprite() const
	{
		return m_Sprite;
	}

	sf::Sprite& GetSpriteNonConst()
	{
		return m_Sprite;
	}

	void Move(const sf::Vector2f& offset)
	{
		m_Sprite.move(offset);
	}

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(m_Sprite, states);
	}

	const PhysicsData& GetPhysicsData() const
	{
		return m_PhysicsData;
	}

	PhysicsData& GetPhysicsDataNonConst()
	{
		return m_PhysicsData;
	}

	void SetPathIndex(int index)
	{
		m_iPathIndex = index;
	}

	int GetPathIndex() const
	{
		return m_iPathIndex;
	}

	void OnCollision(Entity& otherEntity);

	void SetHealth(int health)
	{
		m_iHealth = health;
	}

	void DealDamage(int damage)
	{
		m_iHealth -= damage;
		if (m_iHealth <= 0)
		{
			m_bDeletionRequested = true;
		}
	}

	bool IsDeletionRequested() const
	{
		return m_bDeletionRequested;
	}

	void RequestDeletion()
	{
		m_bDeletionRequested = true;
	}

private:
	sf::Sprite m_Sprite;
	PhysicsData m_PhysicsData;
	bool m_bDeletionRequested;

	// Stuff that should probably be moved somewhere so that it is exclusive to enemies
	int m_iPathIndex;
	int m_iHealth;

public:
	float m_fAxeTimer;

	// Stuff that should probably be moved somewhere so that it is exclusive to towers
	float m_fAttackTimer;
};

#endif // ENTITY_H
