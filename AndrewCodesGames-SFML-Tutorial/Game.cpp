#include "Game.h"

#include <iostream>
#include <SFML/Graphics.hpp>
#include "MathHelpers.h"

Game::Game()
	: m_Window(sf::VideoMode(1920, 1080), "SFML Tutorial")
	, m_eGameMode(Play)
	, m_vRequestedPlayerMovement(0.0f, 0.0f)
	, m_fPlayerSpeed(100.0f)
	, m_eScrollWheelInput(None)
	, m_iTileOptionIndex(0)
{
	// Load the player texture
	m_PlayerTexture.loadFromFile("Images/Player.png");
	// Create a sprite with the player texture
	m_Player.setTexture(m_PlayerTexture);
	// Our texture is pretty small compared to the screen size, so we scale it up.
	m_Player.setScale(sf::Vector2f(10, 10));
	// The default origin for a sprite is the top left hand corner, but it's easier to think about moving stuff when it's centered :)
	m_Player.setOrigin(sf::Vector2f(8, 8));

	// Do the same stuff for the enemy
	m_EnemyTexture.loadFromFile("Images/Enemy.png");
	m_Enemy.setTexture(m_EnemyTexture);
	m_Enemy.setScale(sf::Vector2f(10, 10));
	m_Enemy.setPosition(sf::Vector2f(960, 540));
	m_Enemy.setOrigin(sf::Vector2f(8, 8));

	// And for the axe
	m_AxeTexture.loadFromFile("Images/Axe.png");
	m_Axe.setTexture(m_AxeTexture);
	m_Axe.setScale(sf::Vector2f(10, 10));
	m_Axe.setOrigin(sf::Vector2f(8, 8));

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
		case LevelEditor:
			UpdateLevelEditor();
		}

		Draw();
	}
}

void Game::UpdatePlay()
{
	// Move the player's sprite
	m_Player.move(m_vRequestedPlayerMovement * m_DeltaTime.asSeconds() * m_fPlayerSpeed);

	// Set the axe's position to be towards the mouse position
	sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(m_Window);
	sf::Vector2f vPlayerToMouse = vMousePosition - m_Player.getPosition();
	sf::Vector2f vPlayerToMouseNormalized = MathHelpers::Normalize(vPlayerToMouse);
	m_Axe.setPosition(m_Player.getPosition() + vPlayerToMouseNormalized * 160.0f);

	// Did the axe hit the enemy?
	sf::Vector2f vAxeToEnemy = m_Enemy.getPosition() - m_Axe.getPosition();
	float fLengthFromAxeToEnemy = MathHelpers::Length(vAxeToEnemy);
	// If our axe "hit" the enemy, then move the enemy to a new random spot
	if (fLengthFromAxeToEnemy < 160.0f)
	{
		// The axe has hit the enemy!
		sf::Vector2f vNewPosition(std::rand() % 1920, std::rand() % 1080);
		m_Enemy.setPosition(vNewPosition);
	}

	// Have enemy move towards player
	sf::Vector2f vEnemyToPlayer = m_Player.getPosition() - m_Enemy.getPosition();
	vEnemyToPlayer = MathHelpers::Normalize(vEnemyToPlayer);
	float fEnemySpeed = 50.0f;

	m_Enemy.move(vEnemyToPlayer * m_DeltaTime.asSeconds() * fEnemySpeed);
}

void Game::UpdateLevelEditor()
{

}

void Game::Draw()
{
	// Erases everything that was drawn last frame
	m_Window.clear();

	// Draw all of the tiles
	for (const sf::Sprite& tile : m_Tiles)
	{
		m_Window.draw(tile);
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
	m_vRequestedPlayerMovement = sf::Vector2f(0.0f, 0.0f);
	// Gather the player's input here
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
	{
		m_vRequestedPlayerMovement += sf::Vector2f(0.0f, -1.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		m_vRequestedPlayerMovement += sf::Vector2f(1.0f, 0.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		m_vRequestedPlayerMovement += sf::Vector2f(0.0f, 1.0f);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		m_vRequestedPlayerMovement += sf::Vector2f(-1.0f, 0.0f);
	}
}

void Game::HandleLevelEditorInput()
{
	if(m_eScrollWheelInput == ScrollUp)
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
		if (m_Tiles[i].getPosition() == tile.getPosition())
		{
			// if there is, delete it
			m_Tiles[i] = m_Tiles.back();
			m_Tiles.pop_back();
			break;
		}
	}

	m_Tiles.push_back(tile);
}

void Game::DeleteTileAtPosition(const sf::Vector2f& position)
{
	int x = position.x / 160;
	int y = position.y / 160;

	sf::Vector2f tilePosition(x * 160 + 80, y * 160 + 80);

	for (int i = 0; i < m_Tiles.size(); ++i)
	{
		// check if there is already a tile at this position
		if (m_Tiles[i].getPosition() == tilePosition)
		{
			// if there is, delete it
			m_Tiles[i] = m_Tiles.back();
			m_Tiles.pop_back();
			break;
		}
	}
}
