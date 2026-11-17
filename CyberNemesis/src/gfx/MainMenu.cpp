#include "gfx/MainMenu.h"
#include "gfx/Sprites.h"
#include "gfx/HUD.h"
#include "core/Config.h"
#include "core/Util.h"
#include <algorithm>
#include <cmath>

namespace {
    constexpr int ITEM_X = 58, ITEM_Y = 150, ITEM_STEP = 26, ITEM_W = 300, ITEM_H = 22;
}

MainMenu::Action MainMenu::Activate(int i) {
    switch (i) {
        case 0: return Action::Start;
        case 1:
            if (confirmT_ > 0.f) { confirmT_ = 0.f; return Action::Wipe; }
            confirmT_ = 3.f;
            moved_ = true;
            return Action::None;
        default: return Action::Codex;
    }
}

MainMenu::Action MainMenu::Update(float dt, Vector2 mouse, bool click) {
    confirmT_ = std::max(0.f, confirmT_ - dt);
    hardMix_ += ((hard_ ? 1.f : 0.f) - hardMix_) * std::min(1.f, dt * 4.f);
    pulseT_ = std::max(0.f, pulseT_ - dt * 1.3f);
    for (int i = 0; i < COUNT; ++i) {
        const float target = (i == sel_) ? 1.f : 0.f;
        anim_[i] += (target - anim_[i]) * std::min(1.f, dt * 12.f);
    }

    const int old = sel_;
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) sel_ = (sel_ + 1) % COUNT;
    if (IsKeyPressed(KEY_UP)   || IsKeyPressed(KEY_W)) sel_ = (sel_ + COUNT - 1) % COUNT;

    bool overItem = false;
    for (int i = 0; i < COUNT; ++i) {
        const Rectangle r = { (float)ITEM_X - 6.f, (float)(ITEM_Y + i * ITEM_STEP), (float)ITEM_W, (float)ITEM_H };
        if (CheckCollisionPointRec(mouse, r)) {
            overItem = true;
            if (mouse.x != lastMouse_.x || mouse.y != lastMouse_.y) sel_ = i;
        }
    }
    lastMouse_ = mouse;

    if (sel_ != old) { confirmT_ = 0.f; moved_ = true; }

    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) return Activate(sel_);
    if (click && overItem) return Activate(sel_);
    return Action::None;
}

void MainMenu::Draw(float time, const MenuInfo& info, const Sprites& sp, Color tint) const {
    const int W = Cfg::VIEW_W, H = Cfg::VIEW_H;
    // Paleta: cian/violeta normal; al activar el modo oculto todo vira a rojo (hardMix_ 0..1).
    const Color acc = Util::LerpColor(tint, Theme::NEON_RED, hardMix_);
    const Color hi  = Util::LerpColor(acc, WHITE, 0.45f);
    const float pulse = pulseT_ * pulseT_;

    // --- Fondo: sombreado lateral para leer el menu + chispas que suben ---
    DrawRectangleGradientH(0, 0, 400, H, Fade(BLACK, 0.55f), Fade(BLACK, 0.f));
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < 26; ++i) {
        const float fx = std::fmod((float)i * 97.3f + time * (6.f + (float)(i % 5) * 3.f), (float)W);
        const float fy = (float)H - std::fmod(time * (12.f + (float)(i % 7) * 5.f) + (float)i * 53.f, (float)H);
        const float tw = 0.35f + 0.35f * std::sin(time * 2.f + (float)i);
        DrawRectangle((int)fx, (int)fy, (i % 3 == 0) ? 2 : 1, (i % 3 == 0) ? 2 : 1, Fade(hi, tw * (0.4f + 0.6f * hardMix_)));
    }
    EndBlendMode();

    // --- Marco con esquinas ---
    {
        const int L = 16; const Color fc = Fade(hi, 0.55f);
        DrawRectangle(8, 8, L, 1, fc);               DrawRectangle(8, 8, 1, L, fc);
        DrawRectangle(W - 8 - L, 8, L, 1, fc);       DrawRectangle(W - 9, 8, 1, L, fc);
        DrawRectangle(8, H - 9, L, 1, fc);           DrawRectangle(8, H - 8 - L, 1, L, fc);
        DrawRectangle(W - 8 - L, H - 9, L, 1, fc);   DrawRectangle(W - 9, H - 8 - L, 1, L, fc);
    }
    // Franjas de peligro (solo con el modo oculto activo)
    if (hardMix_ > 0.02f) {
        const int off = (int)std::fmod(time * 30.f, 16.f);
        for (int x = -16; x < W; x += 16) {
            DrawRectangle(x + off, 0, 8, 4, Fade(Theme::NEON_RED, 0.75f * hardMix_));
            DrawRectangle(x - off + 16, H - 4, 8, 4, Fade(Theme::NEON_RED, 0.75f * hardMix_));
        }
    }

    // --- Titulo con aberracion cromatica (mas nerviosa en modo oculto) ---
    const float j = (std::sin(time * 17.f) > 0.965f ? 3.f : 0.f) + (std::sin(time * 41.f) > 0.80f ? 2.f * hardMix_ : 0.f) + 4.f * pulse;
    const Color c1 = Util::LerpColor(Theme::NEON_MAGENTA, Color{255, 150, 40, 255}, hardMix_);
    const Color c2 = Util::LerpColor(Theme::NEON_CYAN, Theme::NEON_RED, hardMix_);
    const char* title = "CYBER-NEMESIS";
    DrawText(title, 54 + 2 + (int)j, 34, 40, Fade(c1, 0.85f));
    DrawText(title, 54 - 2 - (int)j, 36, 40, Fade(c2, 0.85f));
    DrawText(title, 54, 35, 40, Util::LerpColor(WHITE, Color{255, 225, 225, 255}, hardMix_));
    HUD::Text("ADAPTIVE BOSS PROTOCOL", 58, 84, 20, hi);
    DrawRectangle(58, 112, 300, 1, Fade(hi, 0.6f));
    DrawRectangle(58, 114, 120, 1, Fade(hi, 0.3f));
    {   // destello que recorre la linea
        const float sx = 58.f + std::fmod(time * 140.f, 340.f);
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleGradientH((int)sx - 30, 111, 30, 3, Fade(hi, 0.f), Fade(WHITE, 0.8f));
        EndBlendMode();
    }
    HUD::Text("ERES EL JEFE FINAL. EL HÉROE APRENDE DE TI.", 58, 122, 10, Fade(WHITE, 0.8f));   // cabe antes del panel (x=392)

    // --- Opciones como tarjetas numeradas ---
    const char* confirm = "¿SEGURO? OTRA VEZ";
    const char* labels[COUNT] = {
        "INICIAR COMBATE",
        confirmT_ > 0.f ? confirm : "BORRAR MEMORIA",
        "ARCHIVO DE LA IA",
    };
    for (int i = 0; i < COUNT; ++i) {
        const int y = ITEM_Y + i * ITEM_STEP;
        const float a = anim_[i];
        const bool sel = (i == sel_);
        DrawRectangle(ITEM_X - 8, y - 1, ITEM_W, ITEM_H, Fade(BLACK, 0.30f + 0.10f * a));
        DrawRectangle(ITEM_X - 8, y + ITEM_H - 2, ITEM_W, 1, Fade(hi, 0.12f + 0.25f * a));
        if (a > 0.02f) {
            BeginBlendMode(BLEND_ADDITIVE);
            DrawRectangleGradientH(ITEM_X - 8, y - 1, ITEM_W, ITEM_H, Fade(acc, 0.38f * a), Fade(acc, 0.f));
            EndBlendMode();
            DrawRectangle(ITEM_X - 8, y - 1, 3, ITEM_H, Fade(hi, a));
        }
        HUD::Text(TextFormat("0%d", i + 1), ITEM_X - 2, y + 7, 10, Fade(hi, 0.35f + 0.55f * a));
        Color c = Fade(WHITE, 0.50f + 0.45f * a);
        if (i == 1 && confirmT_ > 0.f) c = Fade(Theme::NEON_RED, 0.6f + 0.4f * std::fmod(time * 4.f, 1.f));
        if (sel && !(i == 1 && confirmT_ > 0.f)) c = WHITE;
        HUD::Text(labels[i], ITEM_X + 18 + (int)(6.f * a), y + 1, 20, c);
        if (sel) HUD::Text(">", ITEM_X + 5 + (int)(2.f * std::sin(time * 8.f)), y + 1, 20, hi);
    }

    // --- Panel de memoria ---
    {
        const int px = 392, py = 100, pw = 224, ph = 108;   // termina en y=208, encima de los combatientes
        DrawRectangle(px, py, pw, ph, Fade(BLACK, 0.45f));
        DrawRectangle(px, py, pw, 16, Fade(acc, 0.22f));
        DrawRectangleLines(px, py, pw, ph, Fade(hi, 0.55f));
        DrawRectangle(px, py, 36, 1, hi);
        DrawRectangle(px + pw - 36, py + ph - 1, 36, 1, hi);
        HUD::Text("MEMORIA DEL ANDROIDE", px + 8, py + 4, 10, hi);
        if (info.hasMemory) {
            HUD::Text(TextFormat("GENERACIÓN %d / %d", info.traits.generation, Cfg::MAX_GENERATION), px + 8, py + 22, 10, Theme::NEON_RED);
            for (int g = 0; g < Cfg::MAX_GENERATION; ++g) {            // barra de generaciones
                const bool on = g < info.traits.generation;
                DrawRectangle(px + pw - 8 - (Cfg::MAX_GENERATION - g) * 10, py + 24, 8, 5, on ? Fade(Theme::NEON_RED, 0.9f) : Fade(WHITE, 0.15f));
            }
            HUD::Text(TextFormat("VIDA %d   DAÑO %.1f", (int)info.traits.maxHp, info.traits.damage), px + 8, py + 34, 10, WHITE);
        } else {
            HUD::Text("SIN DATOS PREVIOS", px + 8, py + 22, 10, Fade(WHITE, 0.7f));
            HUD::Text("PRIMER ARRANQUE DE LA IA", px + 8, py + 34, 10, Fade(WHITE, 0.5f));
        }
        int tx = px + 8;
        auto pill = [&](int y, const char* label, bool on, Color col) {
            const int w = MeasureText(label, 10) + 8;
            if (tx + w > px + pw - 4) return;
            DrawRectangle(tx, y, w, 12, on ? Fade(col, 0.25f) : Fade(GRAY, 0.10f));
            DrawRectangleLines(tx, y, w, 12, on ? col : Fade(GRAY, 0.30f));
            DrawText(label, tx + 4, y + 2, 10, on ? col : Fade(GRAY, 0.45f));
            tx += w + 3;
        };
        HUD::Text("HABILIDADES", px + 8, py + 50, 10, Fade(WHITE, 0.6f));
        tx = px + 8; const int y1 = py + 62;
        pill(y1, "ESCUDO", info.traits.deflectShield, Theme::NEON_CYAN);
        pill(y1, "DASH", info.traits.dashEvasive, Theme::NEON_YELLOW);
        pill(y1, "PRED", info.traits.leadShooting, Theme::NEON_GREEN);
        pill(y1, "JETS", info.traits.jumpJets, Theme::NEON_MAGENTA);
        HUD::Text("ARSENAL", px + 8, py + 78, 10, Fade(WHITE, 0.6f));
        tx = px + 8; const int y2 = py + 90;
        pill(y2, "RAF", (info.traits.attacks & Atk::BURST) != 0, Theme::NEON_CYAN);
        pill(y2, "ESC", (info.traits.attacks & Atk::SPREAD) != 0, Theme::NEON_GREEN);
        pill(y2, "MIS", (info.traits.attacks & Atk::MISSILE) != 0, Theme::NEON_YELLOW);
        pill(y2, "ORB", (info.traits.attacks & Atk::ORBITAL) != 0, Theme::NEON_RED);
        pill(y2, "MIN", (info.traits.attacks & Atk::MINE) != 0, Theme::NEON_MAGENTA);
    }

    // --- Combatientes sobre una plataforma de luz, con un VS en medio ---
    {
        const float fy = (float)H - 50.f;                    // deja libres los ultimos 46 px para el texto
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangleGradientV(440, (int)fy - 150, 60, 150, Fade(acc, 0.f), Fade(acc, 0.10f));
        DrawRectangleGradientV(546, (int)fy - 130, 52, 130, Fade(acc, 0.f), Fade(acc, 0.10f));
        EndBlendMode();
        DrawEllipse(470, (int)fy + 2, 40.f * info.bossScale + 8.f, 6.f, Fade(BLACK, 0.5f));
        DrawEllipse(572, (int)fy + 2, 26.f, 5.f, Fade(BLACK, 0.5f));
        const float br = 0.022f * std::sin(time * 3.2f);
        sp.DrawBoss(Sprites::B_IDLE, { 470.f, fy }, Cfg::BOSS_W * info.bossScale, true, 0.f, 1.f - br * 0.6f, 1.f + br, WHITE, 0.6f + 0.2f * std::sin(time * 5.f));
        const float hr = 0.02f * std::sin(time * 3.6f + 1.f);
        sp.DrawHero(Sprites::HeroFrame(Sprites::A_READY, Sprites::L_STAND), { 572.f, fy }, Cfg::HERO_W, false, 0.f, 1.f - hr * 0.6f, 1.f + hr, WHITE, 0.6f);
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangle(430, (int)fy + 1, 180, 1, Fade(hi, 0.35f));
        EndBlendMode();
        HUD::Text("VS", 523, (int)fy - 62 + (int)(2.f * std::sin(time * 3.f)), 10, Fade(hi, 0.7f + 0.25f * std::sin(time * 4.f)));
    }

    HUD::TextCentered("W/S O FLECHAS: ELEGIR    ENTER / CLIC: ACEPTAR    ESC: SALIR", W / 2, H - 14, 10, Fade(WHITE, 0.65f));

    // Descripcion de la opcion seleccionada
    static const char* DESC[COUNT] = {
        "Empieza el experimento: tu forma de jugar entrena a la IA.",
        "Borra lo que la IA aprendió de ti y empieza de cero.",
        "Cómo aprende la IA, por qué existe el proyecto y cómo funciona por dentro.",
    };
    {   // descripcion bajo las opciones, partida en lineas que caben a la izquierda (sin pisar el panel ni los sprites)
        const int dy = ITEM_Y + COUNT * ITEM_STEP + 8;
        DrawRectangle(ITEM_X - 8, dy - 3, 3, 26, Fade(acc, 0.8f));
        int ly = dy;
        for (const std::string& line : HUD::Wrap(DESC[std::max(0, std::min(sel_, COUNT - 1))], ITEM_W - 12, 10)) {
            HUD::Text(line.c_str(), ITEM_X, ly, 10, Fade(WHITE, 0.9f));
            ly += 12;
        }
    }

    // --- Pulso al cambiar de dificultad: destello, anillos y barras de glitch ---
    if (pulseT_ > 0.01f) {
        const Color pc = hard_ ? Theme::NEON_RED : tint;
        BeginBlendMode(BLEND_ADDITIVE);
        DrawRectangle(0, 0, W, H, Fade(pc, 0.30f * pulse));
        for (int k = 0; k < 3; ++k)
            DrawCircleLines(W / 2, H / 2, (1.f - pulseT_) * 520.f + (float)k * 16.f, Fade(WHITE, 0.6f * pulseT_));
        for (int k = 0; k < 6; ++k) {
            const int gy = (int)(std::fmod((float)k * 61.f + time * 400.f, (float)H));
            DrawRectangle(0, gy, W, 2, Fade(pc, 0.35f * pulseT_));
        }
        EndBlendMode();
    }
}
