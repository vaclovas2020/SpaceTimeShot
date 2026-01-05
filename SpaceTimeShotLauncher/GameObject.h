#pragma once
#include <d2d1.h>

// Base class for all interactive objects in the game
class GameObject
{
protected:
    float x;          // Horizontal position
    float y;          // Vertical position
    float width;      // Object width
    float height;     // Object height

    // Direct2D Bitmaps are device-dependent resources
    ID2D1Bitmap* pBitmap;

public:
    GameObject(float x, float y, float w, float h);
    virtual ~GameObject();

    // Setup and Resource Management
    void SetBitmap(ID2D1Bitmap* bitmap);

    // Core Game Loop Methods
    virtual void Update(float deltaTime);
    virtual void Render(ID2D1RenderTarget* pRenderTarget);

    // Getters and Setters
    void SetPosition(float newX, float newY) { x = newX; y = newY; }
    float GetX() const { return x; }
    float GetY() const { return y; }
    float GetWidth() const { return width; }
    float GetHeight() const { return height; }
};
