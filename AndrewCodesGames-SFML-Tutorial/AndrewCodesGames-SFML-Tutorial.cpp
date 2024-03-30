#include "SFML/Graphics.hpp"
#include <iostream>
#include <random>

// Helper function to get the length of a 2d vector
float Length(const sf::Vector2f& rVector)
{
    float fLength = sqrt(rVector.x * rVector.x + rVector.y * rVector.y);
    return fLength;
}

// Helper function to normalize a 2d vector, "normalize" means create a vector with length of 1.0f
sf::Vector2f Normalize(const sf::Vector2f& rVector)
{
    float fLength = Length(rVector);
    sf::Vector2f vNormalizedVector(rVector.x / fLength, rVector.y / fLength);
    return vNormalizedVector;
}

int main()
{
    // Create a window to draw stuff to
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "Our SFML Project");

    sf::Texture playerTexture;
    // Load the player texture
    playerTexture.loadFromFile("Images/Player.png");
    // Create a sprite with the player texture
    sf::Sprite player(playerTexture);
    // Our texture is pretty small compared to the screen size, so we scale it up.
    player.setScale(sf::Vector2f(10, 10));
    // The default origin for a sprite is the top left hand corner, but it's easier to think about moving stuff when it's centered :)
    player.setOrigin(sf::Vector2f(8, 8));

    // Do the same stuff for the enemy
    sf::Texture enemyTexture;
    enemyTexture.loadFromFile("Images/Enemy.png");
    sf::Sprite enemy(enemyTexture);
    enemy.setScale(sf::Vector2f(10, 10));
    enemy.setPosition(sf::Vector2f(960, 540));
    enemy.setOrigin(sf::Vector2f(8, 8));

    // And for the axe
    sf::Texture axeTexture;
    axeTexture.loadFromFile("Images/Axe.png");
    sf::Sprite axe(axeTexture);
    axe.setScale(sf::Vector2f(10, 10));
    axe.setOrigin(sf::Vector2f(8, 8));

    // This will help us keep track of the time between frames
    sf::Clock clock;
    while (window.isOpen())
    {
        sf::Time timeSinceLastFrame = clock.restart();

        sf::Event event;
        while (window.pollEvent(event))
        {
            // Check if the user clicked the close window button
            if (event.type == sf::Event::Closed)
            {
                window.close();
                // Without a window, it's probably safe to exit the whole program.
                return 0;
            }
        }

        sf::Vector2f vRequestedPlayerMovement(0.0f, 0.0f);
        float fSpeed = 100.0f;
        // Gather the player's input here
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W))
        {
            vRequestedPlayerMovement += sf::Vector2f(0.0f, -1.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
        {
            vRequestedPlayerMovement += sf::Vector2f(1.0f, 0.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
        {
            vRequestedPlayerMovement += sf::Vector2f(0.0f, 1.0f);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
        {
            vRequestedPlayerMovement += sf::Vector2f(-1.0f, 0.0f);
        }
        // Move the player's sprite
        player.move(vRequestedPlayerMovement * timeSinceLastFrame.asSeconds() * fSpeed);

        // Set the axe's position to be towards the mouse position
        sf::Vector2f vMousePosition = (sf::Vector2f)sf::Mouse::getPosition(window);
        sf::Vector2f vPlayerToMouse = vMousePosition - player.getPosition();
        sf::Vector2f vPlayerToMouseNormalized = Normalize(vPlayerToMouse);
        axe.setPosition(player.getPosition() + vPlayerToMouseNormalized*160.0f);

        // Did the axe hit the enemy?
        sf::Vector2f vAxeToEnemy = enemy.getPosition() - axe.getPosition();
        float fLengthFromAxeToEnemy = Length(vAxeToEnemy);
        // If our axe "hit" the enemy, then move the enemy to a new random spot
        if (fLengthFromAxeToEnemy < 160.0f)
        {
            // The axe has hit the enemy!
            sf::Vector2f vNewPosition(std::rand() % 1920, std::rand() % 1080);
            enemy.setPosition(vNewPosition);
        }

        // Have enemy move towards player
        sf::Vector2f vEnemyToPlayer = player.getPosition() - enemy.getPosition();
        vEnemyToPlayer = Normalize(vEnemyToPlayer);
        float fEnemySpeed = 50.0f;

        enemy.move(vEnemyToPlayer * timeSinceLastFrame.asSeconds() * fEnemySpeed);

        // Erases everything that was drawn last frame
        window.clear();
        // Add everything we want to draw, in order that we want it drawn.
        window.draw(enemy);
        window.draw(player);
        window.draw(axe);
        // Draws the new stuff :)
        window.display();
    }

    return 0;
}