#include "Game.h"

#include <iostream>
#include <SFML/Graphics.hpp>
#include "MathHelpers.h"
#include <queue>
#include <set>
#include <algorithm>

#include <cassert>

#include "DamageTextManager.h"

Game::Game()
	: m_Window(sf::VideoMode(1920, 1080), "SFML Tutorial")
	, m_eGameMode(Play)
	, m_eScrollWheelInput(None)
	, m_iTileOptionIndex(0)
	, m_TowerTemplate(Entity::PhysicsData::Type::Static)
	, m_AxeTemplate(Entity::PhysicsData::Type::Dynamic)
	, m_EnemyTemplate(Entity::PhysicsData::Type::Dynamic)
	, m_bDrawPath(true)
	, m_iPlayerHealth(10)
	, m_iPlayerGold(10)
	, m_fTimeInPlayMode(0.0f)
{
	// Load the tower texture
	m_TowerTexture.loadFromFile("Images/Player.png");
	// Create a sprite with the player texture
	m_TowerTemplate.SetTexture(m_TowerTexture);
	// Our texture is pretty small compared to the screen size, so we scale it up.
	m_TowerTemplate.SetScale(sf::Vector2f(5, 5));
	// The default origin for a sprite is the top left hand corner, but it's easier to think about moving stuff when it's centered :)
	m_TowerTemplate.SetOrigin(sf::Vector2f(8, 8));
	m_TowerTemplate.SetCirclePhysics(40.0f);
	m_TowerTemplate.GetPhysicsDataNonConst().SetLayers(Entity::PhysicsData::Layer::Tower);

	// Do the same stuff for the enemy
	m_EnemyTexture.loadFromFile("Images/Enemy.png");
	m_EnemyTemplate.SetTexture(m_EnemyTexture);
	m_EnemyTemplate.SetScale(sf::Vector2f(5, 5));
	m_EnemyTemplate.SetPosition(sf::Vector2f(960, 540));
	m_EnemyTemplate.SetOrigin(sf::Vector2f(8, 8));
	m_EnemyTemplate.SetCirclePhysics(40.0f);
	m_EnemyTemplate.GetPhysicsDataNonConst().SetLayers(Entity::PhysicsData::Layer::Enemy);
	m_EnemyTemplate.SetHealth(3);

	// And for the axe
	m_AxeTexture.loadFromFile("Images/Axe.png");
	m_AxeTemplate.SetTexture(m_AxeTexture);
	m_AxeTemplate.SetScale(sf::Vector2f(5, 5));
	m_AxeTemplate.SetOrigin(sf::Vector2f(8, 8));
	m_AxeTemplate.SetCirclePhysics(40.0f);
	m_AxeTemplate.GetPhysicsDataNonConst().SetLayers(Entity::PhysicsData::Layer::Projectile);
	m_AxeTemplate.GetPhysicsDataNonConst().SetLayersToIgnore(Entity::PhysicsData::Layer::Tower | Entity::PhysicsData::Layer::Projectile);

	m_Font.loadFromFile("Fonts/Kreon-Medium.ttf");

	m_GameModeText.setPosition(sf::Vector2f(960, 100));
	m_GameModeText.setString("Play Mode");
	m_GameModeText.setFont(m_Font);

	m_PlayerText.setPosition(sf::Vector2f(1500, 100));
	m_PlayerText.setString("Player");
	m_PlayerText.setFont(m_Font);

	m_GameOverText.setPosition(sf::Vector2f(960, 540));
	m_GameOverText.setString("Game Over");
	m_GameOverText.setFont(m_Font);
	m_GameOverText.setCharacterSize(100);

	m_TileMapTexture.loadFromFile("Images/TileMap.png");

	for (int y = 0; y < 4; ++y)
	{
		for (int x = 0; x < 4; ++x)
		{
			sf::Sprite tile;
			tile.setTexture(m_TileMapTexture);
			tile.setTextureRect(sf::IntRect(x * 16, y * 16, 16, 16));
			tile.setScale(sf::Vector2f(10, 10));
			tile.setOrigin(sf::Vector2f(8, 8));

			TileOption::TileType eTileType = TileOption::TileType::Null;

			if (y == 0)
			{
				eTileType = TileOption::TileType::Aesthetic;
			}
			else
			{
				if (y == 1)
				{
					if (x == 0)
					{
						eTileType = TileOption::TileType::Spawn;
					}
					else if (x == 1)
					{
						eTileType = TileOption::TileType::End;
					}
					else if (x == 2)
					{
						eTileType = TileOption::TileType::Path;
					}
				}
			}

			TileOption& tileOption = m_TileOptions.emplace_back(eTileType);
			tileOption.SetSprite(tile);
		}
	}
}

Game::~Game()
{
}

void Game::Run()
{
	// This will help us keep track of the time between frames
	sf::Clock clock;
	while (m_Window.isOpen())
	{
		m_DeltaTime = clock.restart();

		HandleInput();

		switch (m_eGameMode)
		{
		case Play:
			UpdatePlay();
			break;
		case LevelEditor:
			UpdateLevelEditor();
			break;
		}

		Draw();
	}
}

void Game::UpdatePlay()
{
	m_fTimeInPlayMode += m_DeltaTime.asSeconds();

	if (m_iPlayerHealth <= 0)
	{
		return;
	}

	DamageTextManager::GetInstanceNonConst().Update(m_DeltaTime);

	// Set the axe's position to be towards the mouse position

	// Did the axe hit the enemy?
	//sf::Vector2f vAxeToEnemy = m_EnemyTemplate.getPosition() - m_Axe.getPosition();
	//float fLengthFromAxeToEnemy = MathHelpers::Length(vAxeToEnemy);
	//// If our axe "hit" the enemy, then move the enemy to a new random spot
	//if (fLengthFromAxeToEnemy < 160.0f)
	//{
	//	// The axe has hit the enemy!
	//	sf::Vector2f vNewPosition(std::rand() % 1920, std::rand() % 1080);
	//	m_EnemyTemplate.setPosition(vNewPosition);
	//}

	UpdateTowers();
	UpdateAxes();

	const int iMaxEnemies = 30;
	if (m_SpawnTiles.size() > 0 && !m_Paths.empty())
	{
		m_EnemyTemplate.SetPosition(m_SpawnTiles[0].GetPosition());
		if (m_Enemies.size() < iMaxEnemies)
		{
			static float fSpawnTimer = 0.0f;
			int fSpawnTimerMultiplier = m_fTimeInPlayMode / 5;
			float fSpawnRate = 1.0f + fSpawnTimerMultiplier / 10.0f;
			// After 1 minute, the spawn rate will be 2.2f
			fSpawnTimer += m_DeltaTime.asSeconds()* fSpawnRate;
			if (fSpawnTimer > 1.0f)
			{
				// spawn enemy
				Entity& newEnemy = m_Enemies.emplace_back(m_EnemyTemplate);
				newEnemy.SetPathIndex(std::rand() % m_Paths.size());
				fSpawnTimer = 0.0f;
			}
		}
	}

	for (int i = m_Enemies.size() - 1; i >= 0; --i)
	{
		Entity& rEnemy = m_Enemies[i];
		// Get the path that this enemy is on
		Path& path = m_Paths[rEnemy.GetPathIndex()];

		// Find the the closest path tile to the enemy
		PathTile* pClosestPathTile = nullptr;
		float fClosestDistance = std::numeric_limits<float>::max();

		for (PathTile& pathTile : path)
		{
			// Find the distance from the enemy to this path tile
			sf::Vector2f vEnemyToPathTile = pathTile.pCurrentTile->GetPosition() - rEnemy.GetPosition();
			float fDistance = MathHelpers::Length(vEnemyToPathTile);

			if (fDistance < fClosestDistance)
			{
				fClosestDistance = fDistance;
				pClosestPathTile = &pathTile;
			}
		}

		// Find the next path tile
		const Entity* pNextTile = pClosestPathTile->pNextTile;
		if (!pNextTile)
		{
			pNextTile = pClosestPathTile->pCurrentTile;
		}

		if (pNextTile->GetClosestGridCoordinates() == m_EndTiles[0].GetClosestGridCoordinates())
		{
			const float fDistanceToEnd = MathHelpers::Length(pNextTile->GetPosition() - rEnemy.GetPosition());
			if (fDistanceToEnd < 40.0f)
			{
				m_Enemies.erase(m_Enemies.begin() + i);
				m_iPlayerHealth -= 1;
				continue;
			}
		}

		float fEnemySpeed = 250.0f;
		sf::Vector2f vEnemyToNextTile = pNextTile->GetPosition() - rEnemy.GetPosition();
		vEnemyToNextTile = MathHelpers::Normalize(vEnemyToNextTile);
		rEnemy.SetVelocity(vEnemyToNextTile * fEnemySpeed);
	}

	UpdatePhysics();

	CheckForDeletionRequests();
}

void Game::UpdateTowers()
{
	for (Entity& tower : m_Towers)
	{
		// Check if it's time to throw an axe
		tower.m_fAttackTimer -= m_DeltaTime.asSeconds();
		if (tower.m_fAttackTimer > 0.0f)
		{
			// Not ready to attack yet
			continue;
		}

		// Find the nearest enemy
		Entity* pNearestEnemy = nullptr;
		float fNearestDistance = std::numeric_limits<float>::max();
		for (Entity& enemy : m_Enemies)
		{
			sf::Vector2f vTowerToEnemy = enemy.GetPosition() - tower.GetPosition();
			float fDistance = MathHelpers::Length(vTowerToEnemy);
			if (fDistance < fNearestDistance)
			{
				fNearestDistance = fDistance;
				pNearestEnemy = &enemy;
			}
		}

		if (!pNearestEnemy)
		{
			// No enemies to attack
			continue;
		}

		// Rotate the tower to face the enemy
		sf::Vector2f vTowerToEnemy = pNearestEnemy->GetPosition() - tower.GetPosition();
		float fAngle = MathHelpers::Angle(vTowerToEnemy);

		tower.GetSpriteNonConst().setRotation(fAngle);

		// Create an axe and set its velocity
		Entity& newAxe = m_Axes.emplace_back(m_AxeTemplate);
		newAxe.SetPosition(tower.GetPosition());
		vTowerToEnemy = MathHelpers::Normalize(vTowerToEnemy);
		newAxe.SetVelocity(vTowerToEnemy * 500.0f);

		// Reset their axe throwing timer
		tower.m_fAttackTimer = 1.0f;
	}
}

void Game::UpdateAxes()
{
	for (Entity& axe : m_Axes)
	{
		axe.m_fAxeTimer -= m_DeltaTime.asSeconds();
		const float fAxeRotationSpeed = 360.0f;
		axe.GetSpriteNonConst().rotate(fAxeRotationSpeed *m_DeltaTime.asSeconds());
		if (axe.m_fAxeTimer <= 0.0f)
		{
			axe.RequestDeletion();
		}
	}
}

void Game::CheckForDeletionRequests()
{
	// Check for deletion requests
	for (int i = m_Axes.size() - 1; i >= 0; --i)
	{
		Entity& axe = m_Axes[i];
		if (axe.IsDeletionRequested())
		{
			m_Axes.erase(m_Axes.begin() + i);
		}
	}

	for (int i = m_Enemies.size() - 1; i >= 0; --i)
	{
		Entity& enemy = m_Enemies[i];
		if (enemy.IsDeletionRequested())
		{
			m_Enemies.erase(m_Enemies.begin() + i);
			m_iPlayerGold += 1;
		}
	}
}

void Game::UpdateLevelEditor()
{
	m_Enemies.clear();
	m_Axes.clear();
	m_Towers.clear();

	m_iPlayerHealth = 10;
	m_iPlayerGold = 10;
	m_fTimeInPlayMode = 0.0f;
}

void Game::UpdatePhysics()
{
	const float fMaxDeltaTime = 0.1f;
	const float fDeltaTime = std::min(m_DeltaTime.asSeconds(), fMaxDeltaTime);

	std::vector<Entity*> allEntities;

	for (Entity& tower : m_Towers)
	{
		allEntities.push_back(&tower);
	}

	for (Entity& enemy : m_Enemies)
	{
		allEntities.push_back(&enemy);
	}

	for (Entity& axe : m_Axes)
	{
		allEntities.push_back(&axe);
	}

	for (Entity* entity : allEntities)
	{
		entity->GetPhysicsDataNonConst().ClearCollisions();
	}

	for (Entity* entity : allEntities)
	{
		// If the entity is dynamic, move it
		if (entity->GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic)
		{
			entity->Move(entity->GetPhysicsData().m_vVelocity * fDeltaTime + entity->GetPhysicsData().m_vImpulse);
			entity->GetPhysicsDataNonConst().ClearImpulse();

			// Check for collisions
			for (Entity* otherEntity : allEntities)
			{
				// If the entity is colliding with itself, skip this iteration
				if (entity == otherEntity)
				{
					continue;
				}

				if (entity->ShouldIgnoreEntityForPhysics(otherEntity))
				{
					continue;
				}

				if (!entity->GetPhysicsDataNonConst().HasCollidedThisUpdate(otherEntity) && IsColliding(*entity, *otherEntity))
				{
					entity->OnCollision(*otherEntity);
					otherEntity->OnCollision(*entity);

					entity->GetPhysicsDataNonConst().AddEntityCollision(otherEntity);
					otherEntity->GetPhysicsDataNonConst().AddEntityCollision(entity);
				}

				ProcessCollision(*entity, *otherEntity);
			}
		}
	}
}

void Game::ProcessCollision(Entity& entity1, Entity& entity2)
{
	assert(entity1.GetPhysicsData().m_eType != Entity::PhysicsData::Type::Static);

	if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
	{
		// We're a circle
		if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
		{
			// We're both circles
			const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
			const float fDistanceBetweenEntities = MathHelpers::Length(vEntity1ToEntity2);
			const float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;
			if (fDistanceBetweenEntities < fSumOfRadii)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// We only need to move entity1
					entity1.Move(-MathHelpers::Normalize(vEntity1ToEntity2) * (fSumOfRadii - fDistanceBetweenEntities));
				}
				else
				{
					// We need to move both entities
					const sf::Vector2f vEntity1ToEntity2Normalized = MathHelpers::Normalize(vEntity1ToEntity2);
					const sf::Vector2f vEntity1Movement = vEntity1ToEntity2Normalized * (fSumOfRadii - fDistanceBetweenEntities) * 0.5f;
					entity1.Move(-vEntity1Movement);
					entity2.Move(vEntity1Movement);
				}
			}
		}
		else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
		{
			// We're a circle, they're a rectangle
			float fClosestX = std::clamp(entity1.GetPosition().x, entity2.GetPosition().x - entity2.GetPhysicsData().m_fWidth / 2, entity2.GetPosition().x + entity2.GetPhysicsData().m_fWidth / 2);
			float fClosestY = std::clamp(entity1.GetPosition().y, entity2.GetPosition().y - entity2.GetPhysicsData().m_fHeight / 2, entity2.GetPosition().y + entity2.GetPhysicsData().m_fHeight / 2);

			sf::Vector2f vClosestPoint(fClosestX, fClosestY);
			sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity1.GetPosition();
			float fDistance = MathHelpers::Length(vCircleToClosestPoint);

			if (fDistance < entity1.GetPhysicsData().m_fRadius)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// We only need to move entity1
					entity1.Move(-MathHelpers::Normalize(vCircleToClosestPoint) * (entity1.GetPhysicsData().m_fRadius - fDistance));
				}
				else
				{
					// We need to move both entities
					const sf::Vector2f vEntity1ToEntity2Normalized = MathHelpers::Normalize(vCircleToClosestPoint);
					const sf::Vector2f vEntity1Movement = vEntity1ToEntity2Normalized * (entity1.GetPhysicsData().m_fRadius - fDistance) * 0.5f;
					entity1.Move(-vEntity1Movement);
					entity2.Move(vEntity1Movement);
				}
			}
		}
	}
	else if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
	{
		// We're a rectangle
		if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
		{
			// We're a rectangle, they're a circle
			float fClosestX = std::clamp(entity2.GetPosition().x, entity1.GetPosition().x - entity1.GetPhysicsData().m_fWidth / 2, entity1.GetPosition().x + entity1.GetPhysicsData().m_fWidth / 2);
			float fClosestY = std::clamp(entity2.GetPosition().y, entity1.GetPosition().y - entity1.GetPhysicsData().m_fHeight / 2, entity1.GetPosition().y + entity1.GetPhysicsData().m_fHeight / 2);

			sf::Vector2f vClosestPoint(fClosestX, fClosestY);
			sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity2.GetPosition();
			float fDistance = MathHelpers::Length(vCircleToClosestPoint);

			if (fDistance < entity2.GetPhysicsData().m_fRadius)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				if (!bIsEntity2Dynamic)
				{
					// We only need to move entity1
					entity1.Move(MathHelpers::Normalize(vCircleToClosestPoint) * (entity2.GetPhysicsData().m_fRadius - fDistance));
				}
				else
				{
					// We need to move both entities
					const sf::Vector2f vEntity2ToEntity1Normalized = MathHelpers::Normalize(vCircleToClosestPoint);
					const sf::Vector2f vEntity2Movement = vEntity2ToEntity1Normalized * (entity2.GetPhysicsData().m_fRadius - fDistance) * 0.5f;
					entity2.Move(-vEntity2Movement);
					entity1.Move(vEntity2Movement);
				}
			}

		}
		else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
		{
			// We're both rectangles
			float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
			float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

			float fOverlapX = entity1.GetPhysicsData().m_fWidth / 2 + entity2.GetPhysicsData().m_fWidth / 2 - fDistanceX;
			float fOverlapY = entity1.GetPhysicsData().m_fHeight / 2 + entity2.GetPhysicsData().m_fHeight / 2 - fDistanceY;

			if (fOverlapX > 0 && fOverlapY > 0)
			{
				const bool bIsEntity2Dynamic = entity2.GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic;
				// guaranteed a collision
				if (fOverlapX < fOverlapY)
				{
					if (entity1.GetPosition().x < entity2.GetPosition().x)
					{
						if (bIsEntity2Dynamic)
						{
							entity1.Move(sf::Vector2f(-fOverlapX / 2, 0));
							entity2.Move(sf::Vector2f(fOverlapX / 2, 0));
						}
						else
						{
							entity1.Move(sf::Vector2f(-fOverlapX, 0));
						}

					}
					else
					{
						if (bIsEntity2Dynamic)
						{
							entity1.Move(sf::Vector2f(fOverlapX / 2, 0));
							entity2.Move(sf::Vector2f(-fOverlapX / 2, 0));
						}
						else
						{
							entity1.Move(sf::Vector2f(fOverlapX, 0));
						}
					}
				}
				else
				{
					if (entity1.GetPosition().y < entity2.GetPosition().y)
					{
						if (bIsEntity2Dynamic)
						{
							entity1.Move(sf::Vector2f(0, -fOverlapY / 2));
							entity2.Move(sf::Vector2f(0, fOverlapY / 2));
						}
						else
						{
							entity1.Move(sf::Vector2f(0, -fOverlapY));
						}
					}
					else
					{
						if (bIsEntity2Dynamic)
						{
							entity1.Move(sf::Vector2f(0, fOverlapY / 2));
							entity2.Move(sf::Vector2f(0, -fOverlapY / 2));
						}
						else
						{
							entity1.Move(sf::Vector2f(0, fOverlapY));
						}
					}
				}
			}
		}
	}
}

bool Game::IsColliding(const Entity& entity1, const Entity& entity2)
{
	if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
	{
		// We're a circle
		if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
		{
			// We're both circles
			const sf::Vector2f vEntity1ToEntity2 = entity2.GetPosition() - entity1.GetPosition();
			const float fDistanceBetweenEntities = MathHelpers::Length(vEntity1ToEntity2);
			const float fSumOfRadii = entity1.GetPhysicsData().m_fRadius + entity2.GetPhysicsData().m_fRadius;
			if (fDistanceBetweenEntities < fSumOfRadii)
			{
				return true;
			}
		}
		else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
		{
			// We're a circle, they're a rectangle
			float fClosestX = std::clamp(entity1.GetPosition().x, entity2.GetPosition().x - entity2.GetPhysicsData().m_fWidth / 2, entity2.GetPosition().x + entity2.GetPhysicsData().m_fWidth / 2);
			float fClosestY = std::clamp(entity1.GetPosition().y, entity2.GetPosition().y - entity2.GetPhysicsData().m_fHeight / 2, entity2.GetPosition().y + entity2.GetPhysicsData().m_fHeight / 2);

			sf::Vector2f vClosestPoint(fClosestX, fClosestY);
			sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity1.GetPosition();
			float fDistance = MathHelpers::Length(vCircleToClosestPoint);

			if (fDistance < entity1.GetPhysicsData().m_fRadius)
			{
				return true;
			}
		}
	}
	else if (entity1.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
	{
		// We're a rectangle
		if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Circle)
		{
			// We're a rectangle, they're a circle
			float fClosestX = std::clamp(entity2.GetPosition().x, entity1.GetPosition().x - entity1.GetPhysicsData().m_fWidth / 2, entity1.GetPosition().x + entity1.GetPhysicsData().m_fWidth / 2);
			float fClosestY = std::clamp(entity2.GetPosition().y, entity1.GetPosition().y - entity1.GetPhysicsData().m_fHeight / 2, entity1.GetPosition().y + entity1.GetPhysicsData().m_fHeight / 2);

			sf::Vector2f vClosestPoint(fClosestX, fClosestY);
			sf::Vector2f vCircleToClosestPoint = vClosestPoint - entity2.GetPosition();
			float fDistance = MathHelpers::Length(vCircleToClosestPoint);

			if (fDistance < entity2.GetPhysicsData().m_fRadius)
			{
				return true;
			}

		}
		else if (entity2.GetPhysicsData().m_eShape == Entity::PhysicsData::Shape::Rectangle)
		{
			// We're both rectangles
			float fDistanceX = std::abs(entity1.GetPosition().x - entity2.GetPosition().x);
			float fDistanceY = std::abs(entity1.GetPosition().y - entity2.GetPosition().y);

			float fOverlapX = entity1.GetPhysicsData().m_fWidth / 2 + entity2.GetPhysicsData().m_fWidth / 2 - fDistanceX;
			float fOverlapY = entity1.GetPhysicsData().m_fHeight / 2 + entity2.GetPhysicsData().m_fHeight / 2 - fDistanceY;

			if (fOverlapX > 0 && fOverlapY > 0)
			{
				return true;
			}
		}
	}

	return false;
}

void Game::Draw()
{
	// Erases everything that was drawn last frame
	m_Window.clear();

	// Draw all of the tiles
	for (const Entity& entity : m_AestheticTiles)
	{
		m_Window.draw(entity);
	}

	// Draw words on the screens
	m_Window.draw(m_GameModeText);

	switch (m_eGameMode)
	{
	case Play:
		DrawPlay();
		break;
	case LevelEditor:
		DrawLevelEditor();
		break;
	}

	// Draws the new stuff :)
	m_Window.display();
}

void Game::DrawLevelEditor()
{
	sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	m_TileOptions[m_iTileOptionIndex].setPosition(vMousePosition);

	TileOption::TileType eTileType = m_TileOptions[m_iTileOptionIndex].GetTyleType();

	// Draw spawn tiles
	if (m_bDrawPath)
	{
		for (const Entity& entity : m_PathTiles)
		{
			m_Window.draw(entity);
		}

		for (const Entity& entity : m_SpawnTiles)
		{
			m_Window.draw(entity);
		}

		for (const Entity& entity : m_EndTiles)
		{
			m_Window.draw(entity);
		}
	}

	m_Window.draw(m_TileOptions[m_iTileOptionIndex]);
}

void Game::DrawPlay()
{
	sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	m_TowerTemplate.SetPosition(vMousePosition);

	if (CanPlaceTowerAtPosition(vMousePosition))
	{
		m_TowerTemplate.SetColor(sf::Color::Green);
	}
	else
	{
		m_TowerTemplate.SetColor(sf::Color::Red);
	}

	// Add everything we want to draw, in order that we want it drawn.
	for (const Entity& enemie : m_Enemies)
	{
		m_Window.draw(enemie);
	}

	for (const Entity& tower : m_Towers)
	{
		m_Window.draw(tower);
	}

	for (const Entity& axe : m_Axes)
	{
		m_Window.draw(axe);
	}

	DamageTextManager::GetInstanceConst().Draw(m_Window);

	m_Window.draw(m_TowerTemplate);

	if (m_iPlayerHealth <= 0)
	{
		// draw the game over text
		m_Window.draw(m_GameOverText);
	}

	m_PlayerText.setString("Player Health: " + std::to_string(m_iPlayerHealth) + "\nPlayer Gold: " + std::to_string(m_iPlayerGold));
	m_Window.draw(m_PlayerText);
}

void Game::HandleInput()
{
	static bool bTWasPressedLastUpdate = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::T))
	{
		if (!bTWasPressedLastUpdate)
		{
			if (m_eGameMode == Play)
			{
				m_eGameMode = LevelEditor;
				m_GameModeText.setString("Level Editor Mode");
			}
			else
			{
				m_eGameMode = Play;
				m_GameModeText.setString("Play Mode");
			}
		}
		bTWasPressedLastUpdate = true;
	}
	else
	{
		bTWasPressedLastUpdate = false;
	}

	static bool bYWasPressedLastUpdate = false;
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Y))
	{
		if (!bYWasPressedLastUpdate)
		{
			m_bDrawPath = !m_bDrawPath;
		}
		bYWasPressedLastUpdate = true;
	}
	else
	{
		bYWasPressedLastUpdate = false;
	}

	sf::Event event;
	m_eScrollWheelInput = None;
	while (m_Window.pollEvent(event))
	{
		switch (event.type)
		{
			// Check if the user clicked the close window button
		case sf::Event::Closed:
			m_Window.close();
			break;
		case sf::Event::MouseWheelScrolled:
		{
			if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
			{
				if (event.mouseWheelScroll.delta > 0)
				{
					m_eScrollWheelInput = ScrollUp;
				}
				else
				{
					m_eScrollWheelInput = ScrollDown;
				}
			}
		}
		}
	}

	switch (m_eGameMode)
	{
	case Play:
		HandlePlayInput();
		break;
	case LevelEditor:
		HandleLevelEditorInput();
		break;
	}
}

void Game::HandlePlayInput()
{
	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);

		if (m_iPlayerGold >= 3)
		{
			if (CreateTowerAtPosition(vMousePosition))
			{
				m_iPlayerGold -= 3;
			}
		}
	}
}

void Game::HandleLevelEditorInput()
{
	if (m_eScrollWheelInput == ScrollUp)
	{
		m_iTileOptionIndex++;
		if (m_iTileOptionIndex >= m_TileOptions.size())
		{
			m_iTileOptionIndex = 0;
		}
	}
	else if (m_eScrollWheelInput == ScrollDown)
	{
		m_iTileOptionIndex--;
		if (m_iTileOptionIndex < 0)
		{
			m_iTileOptionIndex = m_TileOptions.size() - 1;
		}
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
	{
		sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);

		std::cout << "Mouse Position: " << vMousePosition.x << ", " << vMousePosition.y << std::endl;

		CreateTileAtPosition(vMousePosition);
	}

	if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
	{
		sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);

		DeleteTileAtPosition(vMousePosition);
	}
}

void Game::CreateTileAtPosition(const sf::Vector2f& position)
{
	int x = position.x / 160;
	int y = position.y / 160;

	TileOption::TileType eTileType = m_TileOptions[m_iTileOptionIndex].GetTyleType();
	if (eTileType == TileOption::TileType::Null)
	{
		return;
	}

	std::vector<Entity>& listOfTiles = GetListOfTiles(eTileType);

	if (eTileType == TileOption::TileType::Spawn || eTileType == TileOption::TileType::End)
	{
		// We only ever want 1 spawn or end tile (for now)
		listOfTiles.clear();
	}

	sf::Sprite tile = m_TileOptions[m_iTileOptionIndex].getSprite();
	tile.setPosition(x * 160 + 80, y * 160 + 80);

	for (int i = 0; i < listOfTiles.size(); ++i)
	{
		// check if there is already a tile at this position
		if (listOfTiles[i].GetPosition() == tile.getPosition())
		{
			// If there's already a tile at the position we want to put one, then we don't need to do anything!
			return;
		}
	}

	Entity& newTile = listOfTiles.emplace_back(Entity::PhysicsData::Type::Static);
	newTile.SetSprite(tile);
	newTile.SetRectanglePhysics(160, 160);

	ConstructPath();
}

void Game::DeleteTileAtPosition(const sf::Vector2f& position)
{
	TileOption::TileType eTileType = m_TileOptions[m_iTileOptionIndex].GetTyleType();
	if (eTileType == TileOption::TileType::Null)
	{
		return;
	}

	std::vector<Entity>& listOfTiles = GetListOfTiles(eTileType);

	int x = position.x / 160;
	int y = position.y / 160;

	sf::Vector2f tilePosition(x * 160 + 80, y * 160 + 80);

	for (int i = 0; i < listOfTiles.size(); ++i)
	{
		// check if there is already a tile at this position
		if (listOfTiles[i].GetPosition() == tilePosition)
		{
			// if there is, delete it
			listOfTiles[i] = listOfTiles.back();
			listOfTiles.pop_back();
			break;
		}
	}
}

void Game::ConstructPath()
{
	m_Paths.clear();

	if (m_SpawnTiles.empty() || m_EndTiles.empty())
	{
		return;
	}

	Path newPath;
	PathTile& start = newPath.emplace_back();
	start.pCurrentTile = &m_SpawnTiles[0];

	sf::Vector2i vEndCoords = m_EndTiles[0].GetClosestGridCoordinates();

	VisitPathNeighbors(newPath, vEndCoords);
}

void Game::VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords)
{
	const sf::Vector2i vCurrentTilePosition = path.back().pCurrentTile->GetClosestGridCoordinates();

	const sf::Vector2i vNorthCoords(vCurrentTilePosition.x, vCurrentTilePosition.y - 1);
	const sf::Vector2i vEastCoords(vCurrentTilePosition.x + 1, vCurrentTilePosition.y);
	const sf::Vector2i vSouthCoords(vCurrentTilePosition.x, vCurrentTilePosition.y + 1);
	const sf::Vector2i vWestCoords(vCurrentTilePosition.x - 1, vCurrentTilePosition.y);

	if (rEndCoords == vNorthCoords || rEndCoords == vEastCoords || rEndCoords == vSouthCoords || rEndCoords == vWestCoords)
	{
		// Set the last tile in our current path to point to the next tile
		path.back().pNextTile = &m_EndTiles[0];
		// Add the next tile, and set it.
		PathTile& newTile = path.emplace_back();
		newTile.pCurrentTile = &m_EndTiles[0];
		m_Paths.push_back(path);

		// If any of our paths are next to the end tile, they should probably go straight to end and terminate.
		// If we didn't return here, we could move around the end tile before going into it.
		return;
	}

	const std::vector<Entity>& pathTiles = GetListOfTiles(TileOption::TileType::Path);

	for (const Entity& pathTile : pathTiles)
	{
		const sf::Vector2i vPathTileCoords = pathTile.GetClosestGridCoordinates();

		// If the path already contains this tile, skip it
		if (DoesPathContainCoordinates(path, vPathTileCoords))
		{
			continue;
		}

		// Doesn't contain the coordinates, check if it's a neighbor
		if (vPathTileCoords == vNorthCoords || vPathTileCoords == vEastCoords || vPathTileCoords == vSouthCoords || vPathTileCoords == vWestCoords)
		{
			// you're a neighbor, and we havent visisted you yet

			// Create a new path (so that we can find multiple paths recursively)
			Path newPath = path;
			// Set the last tile in our current path to point to the next tile
			newPath.back().pNextTile = &pathTile;
			// Add the next tile, and set it.
			PathTile& newTile = newPath.emplace_back();
			newTile.pCurrentTile = &pathTile;

			VisitPathNeighbors(newPath, rEndCoords);
		}
	}
}

bool Game::DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coordinates)
{
	for (const PathTile& pathTile : path)
	{
		if (pathTile.pCurrentTile->GetClosestGridCoordinates() == coordinates)
		{
			return true;
		}
	}

	return false;
}

std::vector<Entity>& Game::GetListOfTiles(TileOption::TileType eTileType)
{
	switch (eTileType)
	{
	case TileOption::TileType::Aesthetic:
		return m_AestheticTiles;
	case TileOption::TileType::Spawn:
		return m_SpawnTiles;
	case TileOption::TileType::End:
		return m_EndTiles;
	case TileOption::TileType::Path:
		return m_PathTiles;
	}

	return m_AestheticTiles;
}

bool Game::CreateTowerAtPosition(const sf::Vector2f& position)
{
	if (CanPlaceTowerAtPosition(position))
	{
		Entity newTower = m_TowerTemplate;
		newTower.SetColor(sf::Color::White);
		m_Towers.push_back(newTower);

		return true;
	}

	return false;
}

bool Game::CanPlaceTowerAtPosition(const sf::Vector2f& position)
{
	// Add all the conditions where I'm not allowed to place a tower

	// If I'm not on a brick, I can't place a tower
	sf::IntRect brickRect(0, 0, 16, 16);
	std::vector<Entity>& listOfTiles = GetListOfTiles(TileOption::TileType::Aesthetic);
	bool bIsOnBrick = false;

	Entity copyOfTowerWithRadiusOf1 = m_TowerTemplate;
	copyOfTowerWithRadiusOf1.SetCirclePhysics(1.0f);

	for (const Entity& tile : listOfTiles)
	{
		const sf::Sprite& rTileSprite = tile.GetSprite();
		// Get the sprites texture rect
		sf::IntRect tileRect = rTileSprite.getTextureRect();
		if (tileRect != brickRect)
		{
			continue;
		}

		if (IsColliding(tile, copyOfTowerWithRadiusOf1))
		{
			bIsOnBrick = true;
			break;
		}
	}

	if (!bIsOnBrick)
	{
		return false;
	}

	// If I'm on top of another tower, I can't place a tower
	for (const Entity& tower : m_Towers)
	{
		if (IsColliding(tower, m_TowerTemplate))
		{
			return false;
		}
	}

	return true;
}
