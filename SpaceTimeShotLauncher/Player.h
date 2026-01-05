#pragma once
#include "GameObject.h"

class Player : public GameObject
{
private:
    float speed;
    // We can track velocity for smoother movement
    float velocityX;
    float velocityY;

public:
    Player(float x, float y);

    // Override the Update method to handle movement logic
    virtual void Update(float deltaTime) override;

    // Method to handle input (usually called from your WindowProc or a KeyManager)
    void HandleInput(bool up, bool down, bool left, bool right);
};
