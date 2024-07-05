#ifndef TILEOPTION_H
#define TILEOPTION_H

#include <SFML/Graphics.hpp>

class TileOption : public sf::Drawable
{
public:
	enum TileType
	{
		Null = -1,
		Aesthetic,
		Spawn,
		End,
		Path
	};

	TileOption(TileType eTileType);

	void SetSprite(const sf::Sprite& sprite) { m_Sprite = sprite; };
    const sf::Sprite& getSprite() const { return m_Sprite; };
	void setPosition(const sf::Vector2f& position) { m_Sprite.setPosition(position); };

	TileType GetTyleType() const { return m_eTileType; };

	void draw(sf::RenderTarget& target, sf::RenderStates states) const override
	{
		target.draw(m_Sprite, states);
	}

private:
	sf::Sprite m_Sprite;
	TileType m_eTileType;
};

#endif // TILEOPTION_H