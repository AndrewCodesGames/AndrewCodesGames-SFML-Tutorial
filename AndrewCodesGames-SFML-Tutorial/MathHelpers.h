#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

#include <SFML/Graphics.hpp>

namespace MathHelpers
{
    // Helper function to get the length of a 2d vector
    float Length(const sf::Vector2f& rVector)
    {
        float fLength = sqrt(rVector.x * rVector.x + rVector.y * rVector.y);
        return fLength;
    }

    // Helper function to normalize a 2d vector, "normalize" means create a vector with length of 1.0f
    sf::Vector2f Normalize(const sf::Vector2f& rVector)
    {
        if (rVector.x == 0.0f && rVector.y == 0.0f)
        {
			return rVector;
		}

        float fLength = Length(rVector);
        sf::Vector2f vNormalizedVector(rVector.x / fLength, rVector.y / fLength);
        return vNormalizedVector;
    }
}

#endif // MATH_HELPERS_H