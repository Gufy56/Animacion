// ============================================================
//  Engine.cpp  –  Implementacion del motor
//
//  DEPENDENCIA EXTERNA: stb_image.h
//  Descargalo de: https://github.com/nothings/stb/blob/master/stb_image.h
//  Coloca stb_image.h en la misma carpeta que este archivo.
// ============================================================

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Engine.h"

// GL_CLAMP_TO_EDGE no esta definido en cabeceras OpenGL antiguas
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

// ─────────────────────────────────────────────────────────────
//  Ease
// ─────────────────────────────────────────────────────────────

float applyEase(float t, Ease e) {
    t = std::max(0.f, std::min(1.f, t));
    switch (e) {
    case Ease::EaseIn:    return t * t;
    case Ease::EaseOut:   return t * (2.f - t);
    case Ease::EaseInOut: return t < 0.5f ? 2.f * t * t : -1.f + (4.f - 2.f * t) * t;
    default:              return t;   // Linear
    }
}

// ─────────────────────────────────────────────────────────────
//  Tween
// ─────────────────────────────────────────────────────────────

void Tween::update(float dt) {
    if (done || !target) return;
    elapsed += dt;
    float raw = (duration > 0.f) ? elapsed / duration : 1.f;
    if (raw >= 1.f) { raw = 1.f; done = true; }
    *target = from + (to - from) * applyEase(raw, ease);
    if (done && onDone) onDone();
}

// ─────────────────────────────────────────────────────────────
//  Sprite
// ─────────────────────────────────────────────────────────────

void Sprite::enableBob(float amplitude, float speed, float phaseOffset) {
    bobEnabled = true;
    bobAmp = amplitude;
    bobSpeed = speed;
    bobOffset = phaseOffset;
    bobBase = pos.y;   // guarda la Y actual como base
}

void Sprite::addTween(float* tgt, float from, float to,
    float dur, Ease e, std::function<void()> cb) {
    Tween tw;
    tw.target = tgt;
    tw.from = from;
    tw.to = to;
    tw.duration = dur;
    tw.ease = e;
    tw.onDone = cb;
    tweens.push_back(tw);
}

void Sprite::moveTo(float x, float y, float dur, Ease e, std::function<void()> cb) {
    // Si tiene bob activo, actualizamos la base en lugar de pos.y directamente
    if (bobEnabled) bobBase = y;
    addTween(&pos.x, pos.x, x, dur, e);
    addTween(&pos.y, pos.y, y, dur, e, cb);
}

void Sprite::moveBy(float dx, float dy, float dur, Ease e) {
    moveTo(pos.x + dx, pos.y + dy, dur, e);
}

void Sprite::rotateTo(float angle, float dur, Ease e) {
    addTween(&rot, rot, angle, dur, e);
}

void Sprite::rotateBy(float da, float dur, Ease e) {
    rotateTo(rot + da, dur, e);
}

void Sprite::scaleTo(float sx, float sy, float dur, Ease e, std::function<void()> cb) {
    addTween(&scl.x, scl.x, sx, dur, e);
    addTween(&scl.y, scl.y, sy, dur, e, cb);
}

void Sprite::fadeTo(float a, float dur, Ease e, std::function<void()> cb) {
    addTween(&alpha, alpha, a, dur, e, cb);
}

void Sprite::fadeIn(float dur) {
    alpha = 0.f;
    show();
    fadeTo(1.f, dur);
}

void Sprite::fadeOut(float dur, std::function<void()> cb) {
    fadeTo(0.f, dur, Ease::Linear, [this, cb]() {
        hide();
        if (cb) cb();
        });
}

void Sprite::update(float dt, float globalTime) {
    // Actualizar tweens
    for (auto& tw : tweens) tw.update(dt);
    tweens.erase(std::remove_if(tweens.begin(), tweens.end(),
        [](const Tween& t) { return t.done; }), tweens.end());

    // Efecto bob (flotacion senoidal)
    if (bobEnabled && visible) {
        float sine = std::sinf(globalTime * bobSpeed * 2.f * 3.14159f + bobOffset);
        pos.y = bobBase + sine * bobAmp;
    }
}

void Sprite::draw() {
    if (!visible || !texture || alpha <= 0.f) return;

    glPushMatrix();
    glTranslatef(pos.x, pos.y, 0.f);
    glRotatef(rot, 0.f, 0.f, 1.f);
    glScalef(scl.x, scl.y, 1.f);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glColor4f(1.f, 1.f, 1.f, alpha);

    float hw = w * 0.5f;
    float hh = h * 0.5f;

    glBegin(GL_QUADS);
    glTexCoord2f(0.f, 0.f); glVertex2f(-hw, -hh);
    glTexCoord2f(1.f, 0.f); glVertex2f(hw, -hh);
    glTexCoord2f(1.f, 1.f); glVertex2f(hw, hh);
    glTexCoord2f(0.f, 1.f); glVertex2f(-hw, hh);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────
//  Camera
// ─────────────────────────────────────────────────────────────

void Camera::addTween(float* tgt, float from, float to,
    float dur, Ease e, std::function<void()> cb) {
    Tween tw;
    tw.target = tgt;
    tw.from = from;
    tw.to = to;
    tw.duration = dur;
    tw.ease = e;
    tw.onDone = cb;
    tweens.push_back(tw);
}

void Camera::moveTo(float x, float y, float dur, Ease e) {
    addTween(&pos.x, pos.x, x, dur, e);
    addTween(&pos.y, pos.y, y, dur, e);
}

void Camera::zoomTo(float z, float dur, Ease e) {
    addTween(&zoom, zoom, z, dur, e);
}

void Camera::rotateTo(float angle, float dur, Ease e) {
    addTween(&rot, rot, angle, dur, e);
}

void Camera::shake(float intensity, float duration) {
    shakeIntensity = intensity;
    shakeDuration = duration;
    shakeElapsed = 0.f;
}

void Camera::update(float dt) {
    for (auto& tw : tweens) tw.update(dt);
    tweens.erase(std::remove_if(tweens.begin(), tweens.end(),
        [](const Tween& t) { return t.done; }), tweens.end());

    if (shakeElapsed < shakeDuration) shakeElapsed += dt;
}

void Camera::apply() {
    glPushMatrix();

    // Temblor de camara
    float sx = 0.f, sy = 0.f;
    if (shakeElapsed < shakeDuration) {
        float progress = shakeElapsed / shakeDuration;
        float strength = shakeIntensity * (1.f - progress); // disminuye con el tiempo
        sx = ((float)(rand() % 200) / 100.f - 1.f) * strength;
        sy = ((float)(rand() % 200) / 100.f - 1.f) * strength;
    }

    glTranslatef(sx, sy, 0.f);
    glScalef(zoom, zoom, 1.f);
    glRotatef(rot, 0.f, 0.f, 1.f);
    glTranslatef(-pos.x, -pos.y, 0.f);
}

void Camera::revert() {
    glPopMatrix();
}

// ─────────────────────────────────────────────────────────────
//  AudioManager
// ─────────────────────────────────────────────────────────────

bool AudioManager::init() {
    FMOD_RESULT r = FMOD::System_Create(&sys);
    if (r != FMOD_OK) {
        std::cerr << "[Audio] FMOD::System_Create fallo\n";
        return false;
    }
    sys->init(64, FMOD_INIT_NORMAL, nullptr);
    std::cout << "[Audio] FMOD inicializado\n";
    return true;
}

void AudioManager::shutdown() {
    for (auto& [k, s] : sounds) s->release();
    sys->close();
    sys->release();
}

void AudioManager::load(const std::string& name,
    const std::string& path,
    bool loop) {
    if (!sys) return;
    FMOD::Sound* s = nullptr;
    FMOD_MODE mode = loop
        ? (FMOD_DEFAULT | FMOD_LOOP_NORMAL)
        : FMOD_DEFAULT;
    FMOD_RESULT r = sys->createSound(path.c_str(), mode, nullptr, &s);
    if (r != FMOD_OK) {
        std::cerr << "[Audio] No se pudo cargar: " << path << "\n";
        return;
    }
    sounds[name] = s;
    std::cout << "[Audio] Cargado: " << name << "\n";
}

FMOD::Channel* AudioManager::play(const std::string& name, float volume) {
    if (!sys || !sounds.count(name)) return nullptr;
    FMOD::Channel* ch = nullptr;
    sys->playSound(sounds[name], nullptr, false, &ch);
    if (ch) ch->setVolume(volume);
    channels[name] = ch;
    return ch;
}

void AudioManager::stop(const std::string& name) {
    if (channels.count(name) && channels[name])
        channels[name]->stop();
}

void AudioManager::setVolume(const std::string& name, float vol) {
    if (channels.count(name) && channels[name])
        channels[name]->setVolume(vol);
}

void AudioManager::fadeOut(const std::string& name, float durationSecs) {
    if (!channels.count(name) || !channels[name]) return;
    float current = 1.f;
    channels[name]->getVolume(&current);
    AudioFade f;
    f.name = name;
    f.from = current;
    f.to = 0.f;
    f.elapsed = 0.f;
    f.duration = durationSecs;
    fades.push_back(f);
}

void AudioManager::update() {
    if (!sys) return;
    sys->update();

    for (auto& f : fades) {
        if (f.elapsed >= f.duration) continue;
        if (channels.count(f.name) && channels[f.name]) {
            float t = std::min(f.elapsed / f.duration, 1.f);
            float vol = f.from + (f.to - f.from) * t;
            channels[f.name]->setVolume(vol);
        }
    }
}

// Avanza los timers de fade - llamado por Engine::frame con el dt real
void AudioManager::updateFades(float dt) {
    for (auto& f : fades) {
        if (f.elapsed < f.duration) {
            f.elapsed += dt;
            if (f.elapsed >= f.duration)
                stop(f.name);
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  Timeline
// ─────────────────────────────────────────────────────────────

void Timeline::at(float time, std::function<void()> action, const std::string& label) {
    TLEvent ev;
    ev.time = time;
    ev.action = action;
    ev.label = label;
    ev.fired = false;
    events.push_back(ev);
    // Mantener ordenado por tiempo
    std::stable_sort(events.begin(), events.end(),
        [](const TLEvent& a, const TLEvent& b) { return a.time < b.time; });
}

void Timeline::update(float dt) {
    if (!running) return;
    t += dt;
    for (auto& ev : events) {
        if (!ev.fired && t >= ev.time) {
            ev.fired = true;
            if (!ev.label.empty())
                std::cout << "[Timeline] t=" << t << "  " << ev.label << "\n";
            if (ev.action) ev.action();
        }
    }
}

// ─────────────────────────────────────────────────────────────
//  AssetManager
// ─────────────────────────────────────────────────────────────

Texture* AssetManager::load(const std::string& name, const std::string& path) {
    int w, h, channels;
    // stb_image carga con origen arriba-izquierda.
    // OpenGL tiene origen abajo-izquierda, pero como usamos
    // coordenadas Y-arriba y los UVs los ajustamos en el quad,
    // dejamos flip=true para que las imagenes salgan correctas.
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path.c_str(), &w, &h, &channels, 4);
    if (!data) {
        std::cerr << "[Assets] No se pudo cargar: " << path << "\n";
        return nullptr;
    }

    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);

    cache[name] = { id, w, h };
    std::cout << "[Assets] Cargado: " << name << " (" << w << "x" << h << ")\n";
    return &cache[name];
}

Texture* AssetManager::get(const std::string& name) {
    auto it = cache.find(name);
    if (it == cache.end()) {
        std::cerr << "[Assets] Textura no encontrada: " << name << "\n";
        return nullptr;
    }
    return &it->second;
}

void AssetManager::unloadAll() {
    for (auto& [k, tex] : cache)
        glDeleteTextures(1, &tex.id);
    cache.clear();
}

// ─────────────────────────────────────────────────────────────
//  Engine  –  singleton
// ─────────────────────────────────────────────────────────────

Engine* Engine::_inst = nullptr;

Engine& Engine::get() {
    if (!_inst) _inst = new Engine();
    return *_inst;
}

// ── static GLUT callbacks ─────────────────────────────────────

void Engine::cbDisplay() {
    Engine::get().frame();
}

void Engine::cbTimer(int) {
    glutPostRedisplay();
    glutTimerFunc(16, cbTimer, 0);   // ~60 fps
}

void Engine::cbKeyboard(unsigned char key, int, int) {
    if (key == 27) {                 // ESC
        Engine::get().audio.shutdown();
        exit(0);
    }
}

// ── init ─────────────────────────────────────────────────────

bool Engine::init(int argc, char** argv, int w, int h, const std::string& title) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(w, h);
    glutInitWindowPosition(
        (glutGet(GLUT_SCREEN_WIDTH) - w) / 2,
        (glutGet(GLUT_SCREEN_HEIGHT) - h) / 2);
    glutCreateWindow(title.c_str());

    // Estado OpenGL
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glDisable(GL_DEPTH_TEST);

    // Proyeccion ortografica centrada
    // (0,0) = centro pantalla
    // X: -640 a +640   Y: -360 a +360
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-W / 2.f, W / 2.f, -H / 2.f, H / 2.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    // GLUT callbacks
    glutDisplayFunc(cbDisplay);
    glutKeyboardFunc(cbKeyboard);
    glutTimerFunc(16, cbTimer, 0);

    // FMOD
    audio.init();

    std::cout << "[Engine] Iniciado. Ventana: " << w << "x" << h << "\n";
    return true;
}

// ── run ──────────────────────────────────────────────────────

void Engine::run() {
    if (onSetup) onSetup();
    timeline.start();
    lastMs = (float)glutGet(GLUT_ELAPSED_TIME);
    glutMainLoop();
}

// ── sprites ──────────────────────────────────────────────────

Sprite* Engine::addSprite(const std::string& name,
    const std::string& texName,
    float x, float y,
    float sw, float sh,
    int   z) {
    Sprite* sp = new Sprite();
    sp->name = name;
    sp->texture = assets.get(texName);
    sp->pos = { x, y };
    sp->w = sw;
    sp->h = sh;
    sp->zOrder = z;
    sp->visible = false;
    sp->alpha = 1.f;

    spriteList.push_back(sp);
    spriteMap[name] = sp;
    sortSprites();
    return sp;
}

Sprite* Engine::addBackground(const std::string& name,
    const std::string& texName,
    int z) {
    // Fondo centrado en el origen, tama~no igual a la ventana
    return addSprite(name, texName, 0.f, 0.f, (float)W, (float)H, z);
}

Sprite* Engine::getSprite(const std::string& name) {
    auto it = spriteMap.find(name);
    return (it != spriteMap.end()) ? it->second : nullptr;
}

void Engine::removeSprite(const std::string& name) {
    auto it = spriteMap.find(name);
    if (it == spriteMap.end()) return;
    Sprite* sp = it->second;
    spriteMap.erase(it);
    spriteList.erase(std::remove(spriteList.begin(), spriteList.end(), sp),
        spriteList.end());
    delete sp;
}

void Engine::sortSprites() {
    std::stable_sort(spriteList.begin(), spriteList.end(),
        [](Sprite* a, Sprite* b) { return a->zOrder < b->zOrder; });
}

// ── helpers visuales ─────────────────────────────────────────

void Engine::drawColorOverlay(float alpha, float r, float g, float b) {
    if (alpha <= 0.f) return;
    glDisable(GL_TEXTURE_2D);
    glColor4f(r, g, b, alpha);
    glBegin(GL_QUADS);
    glVertex2f(-W / 2.f, -H / 2.f);
    glVertex2f(W / 2.f, -H / 2.f);
    glVertex2f(W / 2.f, H / 2.f);
    glVertex2f(-W / 2.f, H / 2.f);
    glEnd();
}

void Engine::showSubtitle(const std::string& speaker,
    const std::string& text,
    float duration,
    float fadeInTime,
    float fadeOutTime) {
    subtitle.speaker = speaker;
    subtitle.text = text;
    subtitle.duration = duration;
    subtitle.elapsed = 0.f;
    subtitle.alpha = 0.f;
    subtitle.active = true;
    // La logica de fade se maneja en renderSubtitle()
    // guardamos los tiempos de fade internamente
    // (simplificado: fade-in rapido, fade-out al final)
}

// ── frame ─────────────────────────────────────────────────────

void Engine::frame() {
    float nowMs = (float)glutGet(GLUT_ELAPSED_TIME);
    float dt = (nowMs - lastMs) / 1000.f;
    if (dt > 0.1f) dt = 0.1f;   // clamp para evitar saltos grandes
    lastMs = nowMs;
    globalTime += dt;

    // ── update ──
    timeline.update(dt);
    camera.update(dt);
    audio.updateFades(dt);
    audio.update();

    for (Sprite* sp : spriteList)
        sp->update(dt, globalTime);

    // Subtitle fade
    if (subtitle.active) {
        subtitle.elapsed += dt;
        float ratio = subtitle.elapsed / subtitle.duration;
        if (ratio < 0.1f)      subtitle.alpha = ratio / 0.1f;
        else if (ratio > 0.85f) subtitle.alpha = 1.f - (ratio - 0.85f) / 0.15f;
        else                    subtitle.alpha = 1.f;
        if (subtitle.elapsed >= subtitle.duration) {
            subtitle.active = false;
            subtitle.alpha = 0.f;
        }
    }

    if (onFrame) onFrame(dt);

    // ── render ──
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();

    camera.apply();
    for (Sprite* sp : spriteList) sp->draw();
    camera.revert();

    // onPostFrame: aqui van overlays, fondidos, HUD
    // Se llama DESPUES de sprites para que quede encima de todo
    if (onPostFrame) onPostFrame();

    renderSubtitle();

    glutSwapBuffers();
}

// ── render subtitulo ─────────────────────────────────────────

void Engine::renderSubtitle() {
    if (!subtitle.active || subtitle.alpha <= 0.f) return;

    float a = subtitle.alpha;

    // Fondo semitransparente de la caja de texto
    glDisable(GL_TEXTURE_2D);
    glColor4f(0.f, 0.f, 0.f, a * 0.65f);
    float bx = -W / 2.f + 20.f;
    float by = -H / 2.f + 10.f;
    float bw = W - 40.f;
    float bh = subtitle.speaker.empty() ? 50.f : 70.f;
    glBegin(GL_QUADS);
    glVertex2f(bx, by);
    glVertex2f(bx + bw, by);
    glVertex2f(bx + bw, by + bh);
    glVertex2f(bx, by + bh);
    glEnd();

    // Nombre del personaje (si hay)
    if (!subtitle.speaker.empty()) {
        glColor4f(1.f, 0.85f, 0.3f, a);   // amarillo dorado
        glRasterPos2f(bx + 10.f, by + bh - 18.f);
        for (char c : subtitle.speaker)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }

    // Texto del dialogo
    glColor4f(1.f, 1.f, 1.f, a);
    float textY = subtitle.speaker.empty() ? by + bh - 20.f : by + bh - 40.f;
    glRasterPos2f(bx + 10.f, textY);
    for (char c : subtitle.text)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}