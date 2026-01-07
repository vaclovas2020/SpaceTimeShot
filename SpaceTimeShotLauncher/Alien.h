#pragma once
#include "AnimatedGameObject.h"

// Specific game entity for the Alien
class Alien : public AnimatedGameObject {
private:
    float speed;        // Movement speed specific to Aliens
    int health;         // Health points
    bool movingRight;   // Simple state for AI movement

public:
    // Constructor passes parameters to AnimatedGameObject
    // For example: 4 frames of animation, 0.1f seconds per frame
    Alien(float startX, float startY)
        : AnimatedGameObject(startX, startY, 256.0f, 256.0f, 6, 0.1f),
        speed(100.0f), health(3), movingRight(true)
    {
    }

    // Override Update to add Alien-specific AI movement
    void Update(float deltaTime) override;

    // You can also add specific methods like TakeDamage
    void TakeDamage(int amount);
};
