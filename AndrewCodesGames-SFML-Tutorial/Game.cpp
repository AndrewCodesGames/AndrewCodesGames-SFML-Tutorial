#include "Game.h"

#include <iostream>
#include <SFML/Graphics.hpp>
#include "MathHelpers.h"

#include <algorithm>

Game::Game()
	: m_Window(sf::VideoMode(1920, 1080), "SFML Tutorial")
	, m_eGameMode(Play)
	, m_vRequestedPlayerMovementDirection(0.0f, 0.0f)
	, m_fPlayerSpeed(100.0f)
	, m_eScrollWheelInput(None)
	, m_iTileOptionIndex(0)
	, m_Player(Entity::PhysicsData::Type::Dynamic)
	, m_Enemy(Entity::PhysicsData::Type::Dynamic)
	, m_Axe(Entity::PhysicsData::Type::Static)
{
	// Load the player texture
	m_PlayerTexture.loadFromFile("Images/Player.png");
	// Create a sprite with the player texture
	m_Player.SetTexture(m_PlayerTexture);
	// Our texture is pretty small compared to the screen size, so we scale it up.
	m_Player.SetScale(sf::Vector2f(10, 10));
	// The default origin for a sprite is the top left hand corner, but it's easier to think about moving stuff when it's centered :)
	m_Player.SetOrigin(sf::Vector2f(8, 8));

	m_Player.SetCirclePhysics(80.0f);
	m_Player.AddEntityToIgnore(&m_Axe);

	// Do the same stuff for the enemy
	m_EnemyTexture.loadFromFile("Images/Enemy.png");
	m_Enemy.SetTexture(m_EnemyTexture);
	m_Enemy.SetScale(sf::Vector2f(10, 10));
	m_Enemy.SetPosition(sf::Vector2f(960, 540));
	m_Enemy.SetOrigin(sf::Vector2f(8, 8));

	m_Enemy.SetCirclePhysics(80.0f);

	// And for the axe
	m_AxeTexture.loadFromFile("Images/Axe.png");
	m_Axe.SetTexture(m_AxeTexture);
	m_Axe.SetScale(sf::Vector2f(10, 10));
	m_Axe.SetOrigin(sf::Vector2f(8, 8));

	m_Axe.SetCirclePhysics(80.0f);

	m_Font.loadFromFile("Fonts/Kreon-Medium.ttf");

	m_GameModeText.setPosition(sf::Vector2f(960, 100));
	m_GameModeText.setString("Play Mode");
	m_GameModeText.setFont(m_Font);

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
			m_TileOptions.push_back(tile);
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
	// Move the player's sprite
	// TODO: change this to be handled in physics
	///m_Player.move(m_vRequestedPlayerMovementDirection * m_DeltaTime.asSeconds() * m_fPlayerSpeed);
	m_Player.SetVelocity(m_vRequestedPlayerMovementDirection * m_fPlayerSpeed);

	// Set the axe's position to be towards the mouse position
	sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	sf::Vector2f vPlayerToMouse = vMousePosition - m_Player.GetPosition();
	sf::Vector2f vPlayerToMouseNormalized = MathHelpers::Normalize(vPlayerToMouse);
	m_Axe.SetPosition(m_Player.GetPosition() + vPlayerToMouseNormalized * 160.0f);

	// Did the axe hit the enemy?
	//sf::Vector2f vAxeToEnemy = m_Enemy.getPosition() - m_Axe.getPosition();
	//float fLengthFromAxeToEnemy = MathHelpers::Length(vAxeToEnemy);
	//// If our axe "hit" the enemy, then move the enemy to a new random spot
	//if (fLengthFromAxeToEnemy < 160.0f)
	//{
	//	// The axe has hit the enemy!
	//	sf::Vector2f vNewPosition(std::rand() % 1920, std::rand() % 1080);
	//	m_Enemy.setPosition(vNewPosition);
	//}

	// Have enemy move towards player
	sf::Vector2f vEnemyToPlayer = m_Player.GetPosition() - m_Enemy.GetPosition();
	vEnemyToPlayer = MathHelpers::Normalize(vEnemyToPlayer);
	float fEnemySpeed = 50.0f;

	// TODO: change this to be handled in physics
	m_Enemy.SetVelocity(vEnemyToPlayer * fEnemySpeed);

	UpdatePhysics();
}

void Game::UpdateLevelEditor()
{

}

void Game::UpdatePhysics()
{
	std::vector<Entity*> allEntities;

	allEntities.push_back(&m_Player);
	allEntities.push_back(&m_Enemy);
	allEntities.push_back(&m_Axe);
	for (Entity& entity : m_Tiles)
	{
		allEntities.push_back(&entity);
	}

	for (Entity* entity : allEntities)
	{
		// If the entity is dynamic, move it
		if (entity->GetPhysicsData().m_eType == Entity::PhysicsData::Type::Dynamic)
		{
			entity->Move(entity->GetPhysicsData().m_vVelocity * m_DeltaTime.asSeconds());

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

				ProcessCollision(*entity, *otherEntity);
			}
		}
	}
}

void Game::ProcessCollision(Entity& entity1, Entity& entity2)
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

void Game::Draw()
{
	// Erases everything that was drawn last frame
	m_Window.clear();

	// Draw all of the tiles
	for (const Entity& entity : m_Tiles)
	{
		m_Window.draw(entity);
	}

	// Add everything we want to draw, in order that we want it drawn.
	m_Window.draw(m_Enemy);
	m_Window.draw(m_Player);
	m_Window.draw(m_Axe);

	// Draw words on the screens
	m_Window.draw(m_GameModeText);

	switch (m_eGameMode)
	{
	case Play:
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

	m_Window.draw(m_TileOptions[m_iTileOptionIndex]);
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
	m_vRequestedPlayerMovementDirection = sf::Vector2f(0.0f, 0.0f);
	// Gather the player's input here
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		m_vRequestedPlayerMovementDirection += sf::Vector2f(0.0f, -1.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		m_vRequestedPlayerMovementDirection += sf::Vector2f(1.0f, 0.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		m_vRequestedPlayerMovementDirection += sf::Vector2f(0.0f, 1.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		m_vRequestedPlayerMovementDirection += sf::Vector2f(-1.0f, 0.0f);
	}

	// Normalize the player's movement so that they don't move faster when moving diagonally
	m_vRequestedPlayerMovementDirection = MathHelpers::Normalize(m_vRequestedPlayerMovementDirection);
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

	sf::Sprite tile = m_TileOptions[m_iTileOptionIndex];
	tile.setPosition(x * 160 + 80, y * 160 + 80);

	for (int i = 0; i < m_Tiles.size(); ++i)
	{
		// check if there is already a tile at this position
		if (m_Tiles[i].GetPosition() == tile.getPosition())
		{
			// if there is, delete it
			m_Tiles[i] = m_Tiles.back();
			m_Tiles.pop_back();
			break;
		}
	}

	Entity& newTile = m_Tiles.emplace_back(Entity::PhysicsData::Type::Static);
	newTile.SetSprite(tile);
	newTile.SetRectanglePhysics(160, 160);
}

void Game::DeleteTileAtPosition(const sf::Vector2f& position)
{
	int x = position.x / 160;
	int y = position.y / 160;

	sf::Vector2f tilePosition(x * 160 + 80, y * 160 + 80);

	for (int i = 0; i < m_Tiles.size(); ++i)
	{
		// check if there is already a tile at this position
		if (m_Tiles[i].GetPosition() == tilePosition)
		{
			// if there is, delete it
			m_Tiles[i] = m_Tiles.back();
			m_Tiles.pop_back();
			break;
		}
	}
}
