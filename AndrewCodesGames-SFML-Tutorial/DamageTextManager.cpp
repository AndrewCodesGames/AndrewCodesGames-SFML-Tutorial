#include "DamageTextManager.h"

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/System/Time.hpp>

DamageTextManager DamageTextManager::m_Instance;

DamageTextManager::DamageTextManager()
{
	m_Font.loadFromFile("Fonts/Kreon-Medium.ttf");
}

DamageTextManager::~DamageTextManager()
{
}

void DamageTextManager::Update(sf::Time& rDeltaTime)
{
	// Iterate over all of the damage texts and update them
	const int iCount = m_DamageText.size();
	for (int i = 0; i < iCount; ++i)
	{
		DamageText damageText = m_DamageText.front();
		m_DamageText.pop_front();
		damageText.m_fRemainingLifeSeconds -= rDeltaTime.asSeconds();
		if (damageText.m_fRemainingLifeSeconds > 0.0f)
		{
			float fPercentageThroughLife = damageText.m_fRemainingLifeSeconds / m_fDamageTextLifeInSeconds;
			
			// Fade out the damage text over time.
			sf::Color color = damageText.m_Text.getFillColor();
			color.a = static_cast<sf::Uint8>(255.0f * fPercentageThroughLife);
			sf::Color outlineColor = damageText.m_Text.getOutlineColor();
			outlineColor.a = static_cast<sf::Uint8>(255.0f * fPercentageThroughLife);

			damageText.m_Text.setFillColor(color);
			damageText.m_Text.setOutlineColor(outlineColor);

			m_DamageText.push_back(damageText);
		}
	}
}

void DamageTextManager::Draw(sf::RenderTarget& rRenderTarget) const
{
	// Iterate over all of the damage texts and draw them
	for (const DamageText& damageText : m_DamageText)
	{
		rRenderTarget.draw(damageText.m_Text);
	}
}

void DamageTextManager::AddDamageText(int damage, const sf::Vector2f& position)
{
	sf::Text text;
	text.setFont(m_Font);
	text.setString(std::to_string(damage));
	text.setCharacterSize(36);
	text.setOutlineColor(sf::Color::Black);
	text.setOutlineThickness(2.0f);
	text.setPosition(position);
	text.setOrigin(text.getLocalBounds().width / 2.0f, text.getLocalBounds().height / 2.0f);

	DamageText damageText;
	damageText.m_Text = text;
	damageText.m_fRemainingLifeSeconds = m_fDamageTextLifeInSeconds;

	m_DamageText.push_back(damageText);
}
