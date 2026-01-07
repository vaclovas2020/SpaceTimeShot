#include "Alien.h"
#include "global.h"

// Override Update to add Alien-specific AI movement
void Alien::Update(float deltaTime) {
    // First, call the parent Update to handle animation timing
    AnimatedGameObject::Update(deltaTime);

    D2D1_SIZE_U size = g_D2DTarget->GetPixelSize();

    // Simple "Ping-Pong" movement logic
    if (movingRight) {
        x += speed * deltaTime;

        if (x > size.width) movingRight = false; // Screen boundary check
    }
    else {
        x -= speed * deltaTime;

        if (x < speed) movingRight = true;
    }
}

// You can also add specific methods like TakeDamage
void Alien::TakeDamage(int amount) {
    health -= amount;

    if (health <= 0) {
        // Handle death logic or set a "isDead" flag
    }
}
