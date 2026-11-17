#pragma once
#include "raylib.h"
#include "ai/HeroTraits.h"

struct Sprites;

struct MenuInfo {
    const char* fxName = "";
    bool        musicOn = true;
    bool        hasMemory = false;
    HeroTraits  traits;
    float       bossScale = 1.f;
};

class MainMenu {
public:
    enum class Action { None, Start, Wipe, Codex };

    Action Update(float dt, Vector2 mouse, bool click);
    void   Draw(float time, const MenuInfo& info, const Sprites& sp, Color tint) const;
    bool   ConsumeMoved() { bool r = moved_; moved_ = false; return r; }
    // Efecto visual del cambio de dificultad (sin texto): tiñe el menú de rojo y lanza un pulso al cambiar.
    void   SetHard(bool hard, bool pulse) { hard_ = hard; if (pulse) pulseT_ = 1.f; }
    float  HardMix() const { return hardMix_; }

private:
    static constexpr int COUNT = 3;
    Action Activate(int i);

    int     sel_ = 0;
    float   confirmT_ = 0.f, anim_[COUNT] = {};
    bool    moved_ = false;
    bool    hard_ = false;
    float   hardMix_ = 0.f, pulseT_ = 0.f;
    Vector2 lastMouse_{-1.f, -1.f};
};
