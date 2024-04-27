#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

class Game
{
public:
	Game();
	~Game();

	enum GameMode
	{
		Play,
		LevelEditor
	};

	// Scroll wheel 
	enum ScrollWheelInput
	{
		ScrollUp,
		ScrollDown,
		None
	};

	void Run();

private:
	void UpdatePlay();
	void UpdateLevelEditor();

	void Draw();
	void DrawLevelEditor();

	void HandleInput();
	void HandlePlayInput();
	void HandleLevelEditorInput();

	// Level Editor functions
	void CreateTileAtPosition(const sf::Vector2f& position);
	void DeleteTileAtPosition(const sf::Vector2f& position);

private:
	sf::RenderWindow m_Window;
	sf::Time m_DeltaTime;
	GameMode m_eGameMode;

	// Play mode
	sf::Sprite m_Player;
	sf::Sprite m_Enemy;
	sf::Sprite m_Axe;

	sf::Texture m_PlayerTexture;
	sf::Texture m_EnemyTexture;
	sf::Texture m_AxeTexture;

	sf::Font m_Font;
	sf::Text m_GameModeText;

	sf::Vector2f m_vRequestedPlayerMovement;
	float m_fPlayerSpeed;

	// Level Editor Mode
	ScrollWheelInput m_eScrollWheelInput;
	int m_iTileOptionIndex;

	sf::Texture m_TileMapTexture;

	std::vector<sf::Sprite> m_TileOptions;
	std::vector<sf::Sprite> m_Tiles;
};
