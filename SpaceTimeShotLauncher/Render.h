#pragma once
#include "global.h"
#include "Bullet.h"
#include <vector>

extern std::vector<Bullet*> g_Bullets;

void HandleShooting();
void InitGame();
bool LoadPNGFromResource(int resourceId, ID2D1Bitmap** ppBitmap);
bool CreateText();
void Update();
void Render();