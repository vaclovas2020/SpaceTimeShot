#pragma once
#include "GameObject.h"

class Bullet : public GameObject
{
private:
    float speed;
    bool active;

public:
    Bullet(float startX, float startY);
    virtual ~Bullet();

    // Override core methods
    virtual void Update(float deltaTime) override;

    // We override Render to draw a primitive dot instead of a bitmap
    virtual void Render(ID2D1RenderTarget* pRenderTarget) override;

    bool IsActive() const { return active; }
    void SetActive(bool state) { active = state; }
};
