#ifndef GAME_H
#define GAME_H

#include "Entity.h"
#include "TileOption.h"

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

	struct PathTile
	{
		const Entity* pCurrentTile;
		const Entity* pNextTile;
	};

	void Run();

private:
	void UpdatePlay();
	void UpdateLevelEditor();

	void UpdatePhysics();

private:
	void ProcessCollision(Entity& entity1, Entity& entity2);

	void Draw();
	void DrawPlay();
	void DrawLevelEditor();

	void HandleInput();
	void HandlePlayInput();
	void HandleLevelEditorInput();

	// Level Editor functions
	void CreateTileAtPosition(const sf::Vector2f& position);
	void DeleteTileAtPosition(const sf::Vector2f& position);

	void ConstructPath();

	std::vector<Entity>& GetListOfTiles(TileOption::TileType eTileType);

private:
	sf::RenderWindow m_Window;
	sf::Time m_DeltaTime;
	GameMode m_eGameMode;

	// Play mode
	Entity m_Player;
	Entity m_Axe;

	Entity m_EnemyTemplate;

	std::vector<Entity> m_Enemies;

	sf::Texture m_PlayerTexture;
	sf::Texture m_EnemyTexture;
	sf::Texture m_AxeTexture;

	sf::Font m_Font;
	sf::Text m_GameModeText;

	sf::Vector2f m_vRequestedPlayerMovementDirection;
	float m_fPlayerSpeed;

	// Level Editor Mode
	ScrollWheelInput m_eScrollWheelInput;
	int m_iTileOptionIndex;

	sf::Texture m_TileMapTexture;

	std::vector<TileOption> m_TileOptions;
	std::vector<Entity> m_AestheticTiles;
	std::vector<Entity> m_SpawnTiles;
	std::vector<Entity> m_EndTiles;
	std::vector<Entity> m_PathTiles;

	bool m_bDrawPath;

private:
	// Pathfinding
	typedef std::vector<PathTile> Path;

	void VisitPathNeighbors(Path path, const sf::Vector2i& rEndCoords);
	bool DoesPathContainCoordinates(const Path& path, const sf::Vector2i& coordinates);

	std::vector<Path> m_Paths;
};

#endif // GAME_H