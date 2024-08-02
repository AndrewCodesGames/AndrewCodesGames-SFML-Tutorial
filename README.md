# Explanation
This branch is for a [YouTube](https://www.youtube.com/@AndrewCodesGames) video series, where, starting from this branch, you are to fullfill the requests from your producer on a video game project. Additionally, I will also do this assignment myself and post a video showing my thoughts/coding as I progress through it.

# Assignment 1 - Email From Producer

Hey Andrew,

Nice work on setting up a basic level editor and enemy movement. Next we’d like to add the option for the player to put down towers. These towers should throw axes at enemies as they approach. Here’s a list of some specific requirements:

## **Tower Placement**
- Only allow tower placement on the “brick” looking tiles, no towers  in the enemies path.
- Towers should not be allowed to overlap each other.
- The player should see a “preview” of their placement, with some indication of whether that placement is valid or not.

## **Tower Behaviour**
- Towers should throw axes at the nearest enemy.
  - (bonus) we’d like the axes to spin as they fly through the air.
- Towers should face the enemy as they throw the axe.

## **Gameplay**
- The player should start with 10 gold and 10 lives.
- The player should lose 1 life every time an enemy reaches the end of their path.
- When the player reaches 0 life, the game ends.
- Towers should cost the player 3 gold to purchase.
- Axes should deal 1 point of damage to the enemy and disappear once they’re hit.
  - (bonus) allow axes to persist once they hit an enemy, but keep them “stuck” on them.
- Enemies should have 3 health, and should disappear once their health reaches 0.
- When an axe hits an enemy, it should knock the enemy back in the direction of the hit.
- When an enemy dies the player should gain 1 gold.
- Enemies should spawn faster over time.

If you have any questions, please reach out!

Best,
Producer

