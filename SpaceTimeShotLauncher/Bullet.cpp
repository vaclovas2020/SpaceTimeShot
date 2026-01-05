#include "Bullet.h"

Bullet::Bullet(float startX, float startY)
    : GameObject(startX, startY, 6.0f, 6.0f), // Small 6x6 dot
    speed(900.0f),                          // Fast moving upward
    active(true)
{
}

Bullet::~Bullet()
{
    // No specific resources to release since we use the shared RenderTarget
}

void Bullet::Update(float deltaTime)
{
    // Move up (negative Y direction)
    y -= speed * deltaTime;

    // Deactivate if it leaves the top of the screen
    if (y < -height)
    {
        active = false;
    }
}

void Bullet::Render(ID2D1RenderTarget* pRenderTarget)
{
    if (!active) return;

    // Create a brush for the bullet (ideally, reuse a global brush for better performance)
    ID2D1SolidColorBrush* pBrush = nullptr;
    HRESULT hr = pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Yellow), // "Shot dot" color
        &pBrush
    );

    if (SUCCEEDED(hr))
    {
        // Draw the bullet as a filled circle (ellipse)
        D2D1_ELLIPSE bulletCircle = D2D1::Ellipse(
            D2D1::Point2F(x, y), // Center point
            width / 2,           // Radius X
            height / 2           // Radius Y
        );

        pRenderTarget->FillEllipse(bulletCircle, pBrush);

        // Brushes must be released after use if created locally
        pBrush->Release();
    }
}
