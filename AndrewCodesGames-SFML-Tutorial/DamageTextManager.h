#ifndef DAMAGE_TEXT_MANAGER_H
#define DAMAGE_TEXT_MANAGER_H

#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Vector2.hpp>
#include <deque>

namespace sf
{
	class RenderTarget;
	class Time;
}

class DamageTextManager
{
private:
	DamageTextManager();
	~DamageTextManager();
public:

	void Update(sf::Time& rDeltaTime);
	void Draw(sf::RenderTarget& rRenderTarget) const;

	void AddDamageText(int damage, const sf::Vector2f& position);

	// Singleton accessor
	static const DamageTextManager& GetInstanceConst()
	{
		return m_Instance;
	}

	static DamageTextManager& GetInstanceNonConst()
	{
		return m_Instance;
	}

private:
	static DamageTextManager m_Instance;
	static constexpr float m_fDamageTextLifeInSeconds = 1.0f;

	struct DamageText
	{
		sf::Text m_Text;
		float m_fRemainingLifeSeconds;
	};

	sf::Font m_Font;
	std::deque<DamageText> m_DamageText;
};

#endif // DAMAGE_TEXT_MANAGER_H
