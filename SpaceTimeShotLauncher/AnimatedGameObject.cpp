#include "AnimatedGameObject.h"

AnimatedGameObject::AnimatedGameObject(float x, float y, float w, float h, int frames, float duration)
    : GameObject(x, y, w, h), totalFrames(frames), frameDuration(duration),
    currentFrame(0), timer(0.0f) {
    // Assuming a horizontal sprite sheet layout
    frameWidth = w;
    frameHeight = h;
}

// Override Update to handle the animation timer
void AnimatedGameObject::Update(float deltaTime) {
    timer += deltaTime;

    if (timer >= frameDuration) {
        timer = 0.0f;
        currentFrame = (currentFrame + 1) % totalFrames; // Loop the animation
    }
}

    // Override Render to draw only the sub-rectangle (current frame)
void AnimatedGameObject::Render(ID2D1RenderTarget* pRT) {
    if (!pBitmap) return;

    // 1. Define where in the PNG the current frame is (Source Rectangle)
    D2D1_RECT_F srcRect = D2D1::RectF(
        currentFrame * frameWidth, 0.0f,
        (currentFrame + 1) * frameWidth, frameHeight
    );

    // 2. Define where on the screen to draw it (Destination Rectangle)
    D2D1_RECT_F destRect = D2D1::RectF(x, y, x + width, y + height);

    // 3. Draw only the source portion of the bitmap
    pRT->DrawBitmap(pBitmap, destRect, 1.0f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, srcRect);
}
