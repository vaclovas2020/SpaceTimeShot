#include "GameObject.h"

// Constructor initializes basic transform data
GameObject::GameObject(float x, float y, float w, float h)
    : x(x), y(y), width(w), height(h), pBitmap(nullptr)
{
}

GameObject::~GameObject()
{
    // Note: pBitmap is usually managed by a central SpriteManager/Loader.
    // If this object owns the bitmap, call pBitmap->Release() here.
}

void GameObject::SetBitmap(ID2D1Bitmap* bitmap)
{
    pBitmap = bitmap;
}

// Logic updates (physics, AI, input)
void GameObject::Update(float deltaTime)
{
    // Default implementation: stationary
    // In child classes, you might do: x += speed * deltaTime;
}

// Draw the object using the provided Direct2D Render Target
void GameObject::Render(ID2D1RenderTarget* pRenderTarget)
{
    if (pBitmap != nullptr && pRenderTarget != nullptr)
    {
        // Define the area on screen where the bitmap will be drawn
        D2D1_RECT_F destinationRect = D2D1::RectF(
            x,
            y,
            x + width,
            y + height
        );

        // Draw the bitmap to the render target
        pRenderTarget->DrawBitmap(pBitmap, destinationRect);
    }
}
