#pragma once

//libraries to include
#include <SDL.h>
#include <SDL_image.h>
#include <iostream>


const int SCREEN_WIDTH = 800; const int SCREEN_HEIGHT = 600;

struct Vector2 { float x, y; };

float Lerp(float min, float max, float value);
float InverseLerp(float min, float max, float value);