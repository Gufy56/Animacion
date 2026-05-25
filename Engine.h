#pragma once

// ============================================================
//  Engine.h  –  Motor de animacion 2D para OpenGL / freeglut
//  Proyecto: Sin Gravedad
//  Coordenadas: origen al centro de pantalla
//               X: -640 (izq) a +640 (der)
//               Y: -360 (abajo) a +360 (arriba)
// ============================================================

#include <GL/freeglut.h>
#include <fmod.hpp>

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include <cmath>
#include <iostream>

// ─────────────────────────────────────────────────────────────
//  Primitivas
// ─────────────────────────────────────────────────────────────

struct Vec2 { float x = 0.f, y = 0.f; };
struct Color { float r = 1.f, g = 1.f, b = 1.f, a = 1.f; };

// ─────────────────────────────────────────────────────────────
//  Easing  –  tipos de interpolacion
// ─────────────────────────────────────────────────────────────

enum class Ease { Linear, EaseIn, EaseOut, EaseInOut };
float applyEase(float t, Ease e);

// ─────────────────────────────────────────────────────────────
//  Tween  –  anima UN float* de A a B en N segundos
// ─────────────────────────────────────────────────────────────

struct Tween {
    float* target = nullptr;
    float     from = 0.f;
    float     to = 0.f;
    float     elapsed = 0.f;
    float     duration = 1.f;
    Ease      ease = Ease::Linear;
    bool      done = false;
    std::function<void()> onDone;          // callback al terminar (opcional)

    void update(float dt);
};

// ─────────────────────────────────────────────────────────────
//  Texture  –  wrapper de textura OpenGL
// ─────────────────────────────────────────────────────────────

struct Texture {
    GLuint id = 0;
    int    width = 0;
    int    height = 0;
};

// ─────────────────────────────────────────────────────────────
//  Sprite  –  objeto 2D dibujable y animable
//
//  Uso tipico:
//    Sprite* sp = engine.addSprite("nombre","textura", x, y, w, h, z);
//    sp->show();
//    sp->moveTo(100, 200, 1.5f, Ease::EaseOut);
//    sp->fadeTo(0.f, 0.5f);
// ─────────────────────────────────────────────────────────────

class Sprite {
public:
    std::string name;
    Texture* texture = nullptr;

    Vec2  pos;                          // posicion del centro en coords mundo
    Vec2  scl = { 1.f, 1.f };     // escala (1 = sin cambio)
    float rot = 0.f;              // rotacion en grados (CCW)
    float alpha = 1.f;              // opacidad  0=transparente, 1=solido
    float w = 0.f;              // ancho de display en pixeles
    float h = 0.f;              // alto  de display en pixeles
    int   zOrder = 0;                // orden de dibujado (mayor = encima)
    bool  visible = false;            // empieza oculto por defecto

    // Flotar arriba y abajo con seno (para objetos en gravedad 0)
    bool  bobEnabled = false;
    float bobAmp = 8.f;            // amplitud en pixeles
    float bobSpeed = 1.2f;           // velocidad (ciclos/seg)
    float bobOffset = 0.f;            // offset de fase (para variar entre sprites)
    float bobBase = 0.f;            // posicion Y base (se auto-asigna al activar bob)

    std::vector<Tween> tweens;

    // ── lifecycle ──
    void show() { visible = true; }
    void hide() { visible = false; }
    void setSize(float width, float height) { w = width; h = height; }

    // Activa el efecto de flotacion suave. Llama ANTES de show().
    void enableBob(float amplitude = 8.f, float speed = 1.2f, float phaseOffset = 0.f);

    // ── animaciones ──
    // Mover a posicion absoluta
    void moveTo(float x, float y, float dur, Ease e = Ease::Linear,
        std::function<void()> cb = nullptr);
    // Mover relativo a posicion actual
    void moveBy(float dx, float dy, float dur, Ease e = Ease::Linear);
    // Rotar a angulo absoluto
    void rotateTo(float angle, float dur, Ease e = Ease::Linear);
    // Rotar relativo al angulo actual
    void rotateBy(float da, float dur, Ease e = Ease::Linear);
    // Escalar a tama~no absoluto
    void scaleTo(float sx, float sy, float dur, Ease e = Ease::Linear,
        std::function<void()> cb = nullptr);
    // Cambiar alpha directamente
    void fadeTo(float a, float dur, Ease e = Ease::Linear,
        std::function<void()> cb = nullptr);
    // Aparecer con fade desde alpha=0
    void fadeIn(float dur);
    // Desaparecer y ocultarse al terminar
    void fadeOut(float dur, std::function<void()> cb = nullptr);

    // ── interno ──
    void addTween(float* tgt, float from, float to, float dur, Ease e,
        std::function<void()> cb = nullptr);
    void update(float dt, float globalTime);
    void draw();
    void clearTweens() { tweens.clear(); }
};

// ─────────────────────────────────────────────────────────────
//  Camera  –  pan, zoom y rotacion de camara
// ─────────────────────────────────────────────────────────────

class Camera {
public:
    Vec2  pos;              // punto del mundo al que apunta la camara
    float zoom = 1.f;       // 1 = normal, 2 = doble de grande
    float rot = 0.f;       // rotacion de camara en grados

    std::vector<Tween> tweens;

    void moveTo(float x, float y, float dur, Ease e = Ease::EaseInOut);
    void zoomTo(float z, float dur, Ease e = Ease::EaseInOut);
    void rotateTo(float angle, float dur, Ease e = Ease::EaseInOut);

    // Efecto de temblor (para el final caotico)
    void shake(float intensity, float duration);

    void addTween(float* tgt, float from, float to, float dur, Ease e,
        std::function<void()> cb = nullptr);
    void update(float dt);
    void apply();     // push matrix
    void revert();    // pop matrix

private:
    float shakeIntensity = 0.f;
    float shakeDuration = 0.f;
    float shakeElapsed = 0.f;
};

// ─────────────────────────────────────────────────────────────
//  AudioManager  –  wrapper delgado sobre FMOD
// ─────────────────────────────────────────────────────────────

class AudioManager {
public:
    FMOD::System* sys = nullptr;

    std::map<std::string, FMOD::Sound*>   sounds;
    std::map<std::string, FMOD::Channel*> channels;

    bool init();
    void shutdown();

    // Cargar un sonido. loop=true para musica de fondo
    void load(const std::string& name, const std::string& path, bool loop = false);

    // Reproducir (devuelve el channel para control fino)
    FMOD::Channel* play(const std::string& name, float volume = 1.f);

    void stop(const std::string& name);
    void setVolume(const std::string& name, float vol);
    void fadeOut(const std::string& name, float durationSecs); // fade gradual

    void update();
    void updateFades(float dt);   // llamado por Engine::frame con el dt real

    struct AudioFade {
        std::string name;
        float       from, to;
        float       elapsed, duration;
    };
    std::vector<AudioFade> fades; // publico para acceso del motor
};

// ─────────────────────────────────────────────────────────────
//  Subtitle  –  texto de dialogo en pantalla
// ─────────────────────────────────────────────────────────────

struct Subtitle {
    std::string text;
    std::string speaker;   // nombre del personaje (puede quedar vacio)
    float       alpha = 0.f;
    float       duration = 0.f;
    float       elapsed = 0.f;
    bool        active = false;
};

// ─────────────────────────────────────────────────────────────
//  Timeline  –  cola de eventos por tiempo (segundos)
//
//  Uso:
//    timeline.at(3.5f, [&]{ sprite->show(); }, "casa aparece");
// ─────────────────────────────────────────────────────────────

struct TLEvent {
    float                 time;
    std::function<void()> action;
    bool                  fired = false;
    std::string           label;           // solo para debug
};

class Timeline {
public:
    float              t = 0.f;
    bool               running = false;
    std::vector<TLEvent> events;

    // Registrar un evento en t=time segundos
    void at(float time, std::function<void()> action, const std::string& label = "");

    void start() { running = true; }
    void pause() { running = false; }
    void resume() { running = true; }

    void update(float dt);
    float time() const { return t; }
};

// ─────────────────────────────────────────────────────────────
//  AssetManager  –  cache de texturas
// ─────────────────────────────────────────────────────────────

class AssetManager {
public:
    std::map<std::string, Texture> cache;

    // Carga un PNG/JPG/BMP desde disco y lo sube a GPU
    Texture* load(const std::string& name, const std::string& path);
    Texture* get(const std::string& name);

    void unloadAll();
};

// ─────────────────────────────────────────────────────────────
//  Engine  –  singleton principal
//
//  Uso minimo en main():
//    Engine& e = Engine::get();
//    e.init(argc, argv, 1280, 720, "Titulo");
//    e.onSetup = [&]{ /* cargar assets, armar timeline */ };
//    e.run();
// ─────────────────────────────────────────────────────────────

class Engine {
public:
    static Engine& get();

    static constexpr int W = 1280;
    static constexpr int H = 720;

    AssetManager assets;
    AudioManager audio;
    Timeline     timeline;
    Camera       camera;
    Subtitle     subtitle;

    // Lista de sprites ordenada por zOrder
    std::vector<Sprite*>           spriteList;
    std::map<std::string, Sprite*> spriteMap;

    // Hooks de usuario
    std::function<void()>        onSetup;      // se llama una vez al inicio
    std::function<void(float)>   onFrame;      // ANTES de dibujar (logica/update)
    std::function<void()>        onPostFrame;  // DESPUES de dibujar (overlays, etc)

    bool init(int argc, char** argv, int w, int h, const std::string& title);
    void run();

    // Crear un sprite a partir de una textura ya cargada
    Sprite* addSprite(const std::string& name,
        const std::string& textureName,
        float x, float y,
        float w, float h,
        int   z = 0);

    // Shortcut para fondo de pantalla completa centrado en (0,0)
    Sprite* addBackground(const std::string& name,
        const std::string& textureName,
        int z = -100);

    Sprite* getSprite(const std::string& name);
    void    removeSprite(const std::string& name);

    // Mostrar texto de dialogo en pantalla
    void showSubtitle(const std::string& speaker,
        const std::string& text,
        float duration,
        float fadeIn = 0.3f,
        float fadeOut = 0.5f);

    // Overlay de color solido (util para fundidos a negro)
    // alpha=0 invisible, alpha=1 pantalla completamente cubierta
    void drawColorOverlay(float alpha,
        float r = 0.f, float g = 0.f, float b = 0.f);

    float getTime() const { return timeline.time(); }

    // Tiempo global acumulado (para efectos de seno en bob, etc.)
    float globalTime = 0.f;

private:
    Engine() = default;
    float lastMs = 0.f;

    static void cbDisplay();
    static void cbTimer(int);
    static void cbKeyboard(unsigned char key, int, int);

    void frame();
    void render();
    void renderSubtitle();
    void sortSprites();

    static Engine* _inst;
};