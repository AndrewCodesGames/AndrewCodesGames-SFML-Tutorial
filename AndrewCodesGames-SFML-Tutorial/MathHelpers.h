#ifndef MATH_HELPERS_H
#define MATH_HELPERS_H

#include <SFML/Graphics.hpp>

namespace MathHelpers
{
	constexpr float DtoR = 0.0174533f;
	constexpr float RtoD = 57.2958f;
	constexpr float SQRT2 = 1.41421356237f;
	constexpr double PI = 3.14159265358979323846;
	constexpr double HALF_PI = PI / 2.0;
	constexpr double QUARTER_PI = PI / 4.0;
	constexpr double EIGHTH_PI = PI / 8.0;
	constexpr double TWO_PI = 2.0 * PI;

    // Helper function to get the length of a 2d vector
    static float Length(const sf::Vector2f& rVector)
    {
        float fLength = sqrt(rVector.x * rVector.x + rVector.y * rVector.y);
        return fLength;
    }

    // Helper function to normalize a 2d vector, "normalize" means create a vector with length of 1.0f
    static sf::Vector2f Normalize(const sf::Vector2f& rVector)
    {
        if (rVector.x == 0.0f && rVector.y == 0.0f)
        {
			return rVector;
		}

        float fLength = Length(rVector);
        sf::Vector2f vNormalizedVector(rVector.x / fLength, rVector.y / fLength);
        return vNormalizedVector;
    }

	/// <summary>
	/// Returns the clockwise angle in degrees from 0-360 relative to the down axis sf::Vector2f(0,1)
	/// </summary>
	/// <param name="a"></param>
	/// <returns></returns>
	static constexpr float Angle(const sf::Vector2f& a)
	{
		if (a.x == 0)
		{
			if (a.y > 0)
			{
				return 0.0f;
			}
			return 180.0;
		}

		float angle = atan2f(a.y, a.x) - HALF_PI;

		if (angle < 0.0f)
		{
			angle += TWO_PI;
		}

		return angle * RtoD;
	}
}

#endif // MATH_HELPERS_H