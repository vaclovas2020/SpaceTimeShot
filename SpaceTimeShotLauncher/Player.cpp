#include "Player.h"

Player::Player(float x, float y)
    : GameObject(x, y, 256.0f, 256.0f), // Initializing with your 256x256 size
    speed(400.0f),
    velocityX(0),
    velocityY(0)
{
}

void Player::HandleInput(bool up, bool down, bool left, bool right)
{
    velocityX = 0;
    velocityY = 0;

    if (up)    velocityY = -speed;
    if (down)  velocityY = speed;
    if (left)  velocityX = -speed;
    if (right) velocityX = speed;
}

void Player::Update(float deltaTime, int screenWidth, int screenHeight)
{
    // 1. Apply Movement
    // Multiplying by deltaTime ensures consistent speed across all monitor refresh rates
    x += velocityX * deltaTime;
    y += velocityY * deltaTime;

    // 2. Dynamic Boundary Checking
    // Keep the 256x256 spaceship within the current window bounds

    // Left & Right
    if (x < 0) x = 0;
    if (x > (float)screenWidth - width)
        x = (float)screenWidth - width;

    // Top & Bottom
    if (y < 0) y = 0;
    if (y > (float)screenHeight - height)
        y = (float)screenHeight - height;
}
