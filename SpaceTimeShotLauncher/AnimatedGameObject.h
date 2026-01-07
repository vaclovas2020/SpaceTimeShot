#pragma once
#include "GameObject.h"

class AnimatedGameObject : public GameObject {
private:
    int totalFrames;      // Total frames in the sprite sheet row
    int currentFrame;     // Index of the frame currently being displayed
    float frameDuration;  // Time (in seconds) each frame stays on screen
    float timer;          // Accumulates deltaTime to track frame swaps
    float frameWidth;     // Width of a single frame in the PNG
    float frameHeight;    // Height of a single frame in the PNG

public:
    AnimatedGameObject(float x, float y, float w, float h, int frames, float duration);

    // Override Update to handle the animation timer
    void Update(float deltaTime) override;

    // Override Render to draw only the sub-rectangle (current frame)
    void Render(ID2D1RenderTarget* pRT) override;
};
