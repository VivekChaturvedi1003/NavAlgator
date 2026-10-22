#ifndef THEME_H
#define THEME_H

#include "raylib.h"
#include <cmath>


namespace Theme {

    inline const Color Background = {14, 16, 30, 255};
    inline const Color Panel      = {22, 25, 46, 255};

    inline const Color FloorA     = {40, 45, 72, 255};
    inline const Color FloorB     = {35, 40, 66, 255};

    inline const Color Exit       = {60, 230, 140, 255};
    inline const Color PlayerBody = {70, 170, 255, 255};
    inline const Color EnemyBody  = {235, 70, 80, 255};

    inline const Color Crate      = {150, 90, 60, 255};
    inline const Color CrateEdge  = {90, 50, 30, 255};
    inline const Color Slow       = {160, 120, 60, 255};

    inline const Color TextMain   = {235, 238, 250, 255};
    inline const Color TextDim    = {150, 158, 190, 255};
    inline const Color Gold       = {255, 205, 80, 255};
    inline const Color Danger     = {255, 90, 90, 255};

   
    inline Color hsv(float hue, float saturation, float value) {
        hue = std::fmod(hue, 360.0f);
        if (hue < 0.0f) hue += 360.0f;

        float c = value * saturation;
        float hp = hue / 60.0f;
        float x = c * (1.0f - std::fabs(std::fmod(hp, 2.0f) - 1.0f));

        float r = 0.0f, g = 0.0f, b = 0.0f;
        if      (hp < 1.0f) { r = c; g = x; }
        else if (hp < 2.0f) { r = x; g = c; }
        else if (hp < 3.0f) { g = c; b = x; }
        else if (hp < 4.0f) { g = x; b = c; }
        else if (hp < 5.0f) { r = x; b = c; }
        else                { r = c; b = x; }

        float m = value - c;
        return Color{ (unsigned char)((r + m) * 255.0f),
                      (unsigned char)((g + m) * 255.0f),
                      (unsigned char)((b + m) * 255.0f),
                      255 };
    }
}

#endif
