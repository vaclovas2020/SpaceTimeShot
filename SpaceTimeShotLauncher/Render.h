#pragma once
#include "global.h"

void InitGame();
bool LoadPNGFromResource(int resourceId, ID2D1Bitmap** ppBitmap);
bool CreateText();
void Update();
void Render();