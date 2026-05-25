// ============================================================
//  main.cpp  -  "Sin Gravedad"
//  ESTADO: Escena 1 completa.
// ============================================================

#include "Engine.h"

static float g_overlayAlpha = 1.f;
static bool  g_fadeAudio = false;
static float g_fadeAudioElapsed = 0.f;

static float g_transOverlay = 0.f;
static float g_transTarget = 0.f;
static float g_transSpeed = 2.0f;

// Estado del ciclo de fotogramas de Pac-Man
static bool  g_pacActive = false;
static float g_pacFrameTimer = 0.f;
static int   g_pacSeqIdx = 0;

// Estado de la secuencia de intentos (Escena 3)
static bool  g_intentoActive = false;
static float g_intentoTimer = 0.f;
static int   g_intentoIdx = 0;
static float g_intentoAudioTimer = 0.f;
static int   g_intentoAudioIdx = 0;

// ── Escena 4 ─────────────────────────────────────────────────
// Flash blanco (flashback)
static float g_flashAlpha = 0.f;
static float g_flashTarget = 0.f;
static float g_flashSpeed = 2.5f;

// Dialogo lip-sync generico (per1 y per2 comparten el ciclo, uno a la vez)
static bool  g_dia1Active = false;  // per1 hablando
static bool  g_dia2Active = false;  // per2 hablando
static float g_diaTimer = 0.f;
static int   g_diaIdx = 0;      // frame {0=1, 1=2, 2=3, 3=O}

// Secuencia de vuelo
static bool  g_flightActive = false;
static float g_vueloX = 0.f;
static float g_nubesVX = 0.f;   // nubes en paralaje (un poco más lentas que el fondo)
static float g_cableX1 = 1400.f;
static float g_cableX2 = 1400.f;
static bool  g_cable1Active = false;
static bool  g_cable2Active = false;

// Edificios (5 edificios, scroll de derecha a izquierda)
struct EdifInfo { float x = 1400.f; bool active = false; float speed = 80.f; };
static EdifInfo g_edif[5];

int main(int argc, char** argv) {

    Engine& e = Engine::get();
    e.init(argc, argv, 1280, 720, "Sin Gravedad");

    // =========================================================
    //  ASSETS
    // =========================================================

    // -- Backgrounds --
    e.assets.load("bg_tierra", "assets/backgrounds/bg_tierra.png");
    e.assets.load("bg_cielo", "assets/backgrounds/bg_cielo.png");
    e.assets.load("bg_casa_ext", "assets/backgrounds/bg_casa_exterior.png");
    e.assets.load("bg_cuarto", "assets/backgrounds/bg_cuarto.png");
    e.assets.load("bg_cocina", "assets/backgrounds/bg_cocina.png");
    e.assets.load("bg_cocina_sentado", "assets/backgrounds/bg_cocina_sentado.png");
    e.assets.load("bg_cocina_sentado2", "assets/backgrounds/bg_cocina_sentado_2.png");

    // -- Sprites --
    e.assets.load("nubes", "assets/sprites/nubes.png");
    e.assets.load("alarma", "assets/sprites/alarma.png");
    e.assets.load("lentes", "assets/sprites/lentes.png");
    e.assets.load("cobija_dormido", "assets/sprites/cobija_dormido.png");
    e.assets.load("cobija_golpe", "assets/sprites/cobija_golpe.png");
    e.assets.load("cobija", "assets/sprites/cobija.png");
    e.assets.load("cobija_techo", "assets/sprites/cobija_techo.png");
    e.assets.load("personaje_sentado", "assets/sprites/personaje_sentado_cama.png");
    e.assets.load("personaje_bosteza", "assets/sprites/personaje_bosteza.png");
    e.assets.load("personaje_lanza", "assets/sprites/personaje_lanza.png");
    e.assets.load("personaje_mira", "assets/sprites/personaje_mira_arriba.png");
    e.assets.load("personaje_habla1", "assets/sprites/personaje_habla1.png");
    e.assets.load("personaje_habla2", "assets/sprites/personaje_habla2.png");
    e.assets.load("personaje_habla3", "assets/sprites/personaje_habla3.png");
    e.assets.load("personaje_hablaO", "assets/sprites/personaje_hablaO.png");
    e.assets.load("personaje_viendo", "assets/sprites/personaje_viendo_tele.png");
    e.assets.load("cereal", "assets/sprites/cereal.png");
    e.assets.load("pacman1", "assets/sprites/pacman1.png");
    e.assets.load("pacman2", "assets/sprites/pacman2.png");
    e.assets.load("pacman3", "assets/sprites/pacman3.png");

    // -- Escena 3 --
    e.assets.load("bg_pasillo", "assets/backgrounds/bg_pasillo.png");
    e.assets.load("pasillo", "assets/sprites/pasillo.png");
    e.assets.load("intento1", "assets/sprites/intento1.png");
    e.assets.load("intento2", "assets/sprites/intento2.png");
    e.assets.load("intento3", "assets/sprites/intento3.png");
    e.assets.load("gato", "assets/sprites/gato.png");
    e.assets.load("sujetar_gato", "assets/sprites/sujetar_gato.png");

    // -- Audio --
    e.audio.load("amanecer_cancion", "assets/audio/amanecer_cancion.mp3", true);
    e.audio.load("pajaros_fondo", "assets/audio/pajaros_fondo.mp3", true);
    e.audio.load("alarma_sfx", "assets/audio/alarma.mp3", false);
    e.audio.load("golpe_despertador", "assets/audio/golpe_despertador.mp3", false);
    e.audio.load("bostezo", "assets/audio/bostezo.mp3", false);
    e.audio.load("lanza_cobija", "assets/audio/lanza_cobija.mp3", false);
    e.audio.load("mama_dialogo", "assets/audio/mama_dialogo.mp3", false);
    e.audio.load("PDpersonaje", "assets/audio/PDpersonaje.mp3", false);
    e.audio.load("tv_noticia", "assets/audio/tv_noticia.mp3", false);
    e.audio.load("dialogo_tele", "assets/audio/dialogo_tele.mp3", false);
    e.audio.load("dialogo_pacman", "assets/audio/dialogo_pacman.mp3", false);
    e.audio.load("estatica", "assets/audio/estatica.mp3", false);
    e.audio.load("musica_fondo2", "assets/audio/musica_fondo2.mp3", true);
    // Segunda instancia para sincronizar el final: empieza en t=80, dura 126s → termina t=206
    e.audio.load("musica_fondo2b", "assets/audio/musica_fondo2.mp3", false);
    e.audio.load("waka", "assets/audio/waka.mp3", true);
    e.audio.load("pacman_muerte", "assets/audio/pacman_muerte.mp3", false);
    e.audio.load("regano_comida", "assets/audio/regano_comida.mp3", false);

    // -- Escena 3 --
    e.audio.load("viento_incomodo", "assets/audio/viento_incomodo.mp3", false);
    e.audio.load("DAtorado", "assets/audio/DAtorado.mp3", false);
    e.audio.load("meow", "assets/audio/meow.mp3", false);
    e.audio.load("lanzar_gato", "assets/audio/lanzar_gato.mp3", false);
    e.audio.load("golpe_vidrio", "assets/audio/golpe_vidrio.mp3", false);
    e.audio.load("intento1_sfx", "assets/audio/intento1.mp3", false);
    e.audio.load("intento2_sfx", "assets/audio/intento2.mp3", false);
    e.audio.load("intento3_sfx", "assets/audio/intento3.mp3", false);

    // -- Escena 4 (final) --
    e.assets.load("bg_patio", "assets/backgrounds/bg_patio.png");
    e.assets.load("bg_mensaje", "assets/backgrounds/bg_mensaje.png");
    e.assets.load("bg_int_mensaje", "assets/backgrounds/bg_interior_mensaje.png");
    e.assets.load("bg_vuelo", "assets/backgrounds/bg_vuelo.png");
    e.assets.load("cables", "assets/sprites/cables_png.png");
    e.assets.load("mensaje1", "assets/sprites/Mensaje1.png");
    e.assets.load("mensaje2", "assets/sprites/Mensaje2.png");
    e.assets.load("carta", "assets/sprites/carta.png");
    e.assets.load("senora", "assets/sprites/senora.png");
    e.assets.load("edificio1", "assets/sprites/edificio1.png");
    e.assets.load("edificio2", "assets/sprites/edificio2.png");
    e.assets.load("edificio3", "assets/sprites/edificio3.png");
    e.assets.load("edificio4", "assets/sprites/edificio4.png");
    e.assets.load("edificio5", "assets/sprites/edificio5.png");
    e.assets.load("foco_idea", "assets/sprites/foco_idea.png");
    e.assets.load("newton", "assets/sprites/newton.png");
    e.assets.load("arbol", "assets/sprites/arbol.png");
    e.assets.load("manzana", "assets/sprites/manzana.png");
    e.assets.load("per1_vuelo", "assets/sprites/per1_vuelo.png");
    e.assets.load("per2_vuelo", "assets/sprites/per2_vuelo.png");
    e.assets.load("per1_d1", "assets/sprites/per1_dialogo1.png");
    e.assets.load("per1_d2", "assets/sprites/per1_dialogo2.png");
    e.assets.load("per1_d3", "assets/sprites/per1_dialogo3.png");
    e.assets.load("per1_dO", "assets/sprites/per1_dialogoO.png");
    e.assets.load("per2_d1", "assets/sprites/per2_dialogo1.png");
    e.assets.load("per2_d2", "assets/sprites/per2_dialogo2.png");
    e.assets.load("per2_d3", "assets/sprites/per2_dialogo3.png");
    e.assets.load("per2_dO", "assets/sprites/per2_dialogoO.png");
    e.assets.load("transitando1", "assets/sprites/transitando1.png");
    e.assets.load("transitando2", "assets/sprites/transitando2.png");

    e.audio.load("cereal_caja", "assets/audio/cereal_caja.mp3", false);
    e.audio.load("idea_sfx", "assets/audio/idea.mp3", false);
    e.audio.load("abre_puerta", "assets/audio/abre_puerta.mp3", false);
    e.audio.load("dialogo_mama_vuelo", "assets/audio/dialogo_mama_vuelo.mp3", false);
    e.audio.load("feldspar_golpe", "assets/audio/feldspar_golpe.mp3", false);
    e.audio.load("recuerdo", "assets/audio/recuerdo.mp3", false);
    e.audio.load("mensaje_envio", "assets/audio/mensaje_envio.mp3", false);
    e.audio.load("catapulta_sfx", "assets/audio/catapulta.mp3", false);
    e.audio.load("dialogo_final1", "assets/audio/dialogo_final1.mp3", false);
    e.audio.load("dialogo_final2", "assets/audio/dialogo_final2.mp3", false);
    e.audio.load("dialogo_final3", "assets/audio/dialogo_final3.mp3", false);
    e.audio.load("dialogo_final4", "assets/audio/dialogo_final4.mp3", false);
    e.audio.load("dialogo_final5", "assets/audio/dialogo_final5.mp3", false);
    e.audio.load("dialogo_final6", "assets/audio/dialogo_final6.mp3", false);

    // =========================================================
    //  SPRITES
    //
    //  Fondos apilados verticalmente (la camara viaja de Y=0 a Y=2100):
    //
    //    Y=   0  bg_tierra      <- camara empieza aqui
    //    Y= 720  bg_cielo_a     <- primer tramo de cielo
    //    Y=1440  bg_cielo_b     <- segundo tramo de cielo
    //    Y=2100  casa_ext       <- destino final
    //
    //  Las nubes estan dispersas entre Y=500 y Y=1600,
    //  cubriendo la pantalla con capas al pasar la camara.
    // =========================================================

    // -- Fondos --
    Sprite* bg_tierra = e.addSprite("bg_tierra", "bg_tierra",
        0, 0, 1280, 720, -100);
    Sprite* bg_cielo_a = e.addSprite("bg_cielo_a", "bg_cielo",
        0, 720, 1280, 720, -99);
    Sprite* bg_cielo_b = e.addSprite("bg_cielo_b", "bg_cielo",
        0, 1440, 1280, 720, -99);
    Sprite* casa_ext = e.addSprite("casa_ext", "bg_casa_ext",
        0, 2100, 1280, 720, -98);
    Sprite* bg_cuarto = e.addBackground("bg_cuarto", "bg_cuarto", -97);

    // -- Nubes de transicion --
    // Son 9 instancias del mismo nubes.png con distinta posicion,
    // escala y Z para dar profundidad y cubrir toda la pantalla.
    //
    //  Distribucion:
    //   Capa baja  (Y ~550- 750): primeras nubes que aparecen al subir
    //   Capa media (Y ~900-1100): cubren la pantalla por completo
    //   Capa alta  (Y ~1300-1500): ultimas nubes antes de ver la casa
    //
    //  X: variado de -640 a +640 para no quedar centradas
    //  scl: entre 0.9x y 2.0x para tamanios distintos
    //  Z:   -60 a -40 para que queden encima del cielo pero debajo de sprites

    Sprite* n[9];
    // Capa baja
    n[0] = e.addSprite("n0", "nubes", -320, 560, 1400, 380, -58);
    n[1] = e.addSprite("n1", "nubes", 380, 620, 1100, 300, -56);
    n[2] = e.addSprite("n2", "nubes", 0, 720, 1600, 440, -54);
    // Capa media (las que cubren la pantalla por completo)
    n[3] = e.addSprite("n3", "nubes", -480, 900, 1280, 400, -60);
    n[4] = e.addSprite("n4", "nubes", 400, 980, 1500, 420, -58);
    n[5] = e.addSprite("n5", "nubes", -100, 1080, 2000, 500, -55);  // muy grande, tapa todo
    // Capa alta
    n[6] = e.addSprite("n6", "nubes", 300, 1300, 1200, 350, -57);
    n[7] = e.addSprite("n7", "nubes", -400, 1380, 1450, 380, -55);
    n[8] = e.addSprite("n8", "nubes", 80, 1500, 1700, 460, -53);

    // Nubes de costura: cubren las lineas de division entre backgrounds.
    // Z=-52 = encima del cielo pero debajo de las nubes de viaje.
    // Seam 1: entre bg_tierra (tope Y=360) y bg_cielo_a (fondo Y=360)
    // Seam 2: entre bg_cielo_a (tope Y=1080) y bg_cielo_b (fondo Y=1080)
    Sprite* ns0 = e.addSprite("ns0", "nubes", 0, 360, 1280, 260, -52);
    Sprite* ns1 = e.addSprite("ns1", "nubes", 0, 1080, 1280, 260, -52);
    // Seam entre bg_cielo_b (top Y=1800) y bg_casa_ext (bottom Y=1740)
    Sprite* ns2 = e.addSprite("ns2", "nubes", 0, 1790, 1280, 280, -52);

    // -- Props de la habitacion --
    //
    //  Z-order dentro del cuarto:
    //  -97  bg_cuarto
    //   -1  cobija_techo   (esta en la cama del techo, area superior)
    //    1  personaje_*    (siempre debajo de la cobija)
    //    2  cobija_*       (siempre encima del personaje)
    //    3  alarma_sp      (prop flotante, encima de todo)

    // Sprites de modo dormido
    Sprite* cobija_dormido_sp = e.addSprite("cobija_dormido_sp", "cobija_dormido",
        0, 0, 1280, 720, 2);
    Sprite* cobija_golpe_sp = e.addSprite("cobija_golpe_sp", "cobija_golpe",
        0, 0, 1280, 720, 2);

    // Sprites de secuencia de levantarse
    Sprite* lentes_sp = e.addSprite("lentes_sp", "lentes",
        0, 0, 1280, 720, 3);
    Sprite* personaje_sentado_sp = e.addSprite("personaje_sentado_sp", "personaje_sentado",
        0, 0, 1280, 720, 1);
    Sprite* cobija_sp = e.addSprite("cobija_sp", "cobija",
        0, 0, 1280, 720, 2);
    Sprite* personaje_bosteza_sp = e.addSprite("personaje_bosteza_sp", "personaje_bosteza",
        0, 0, 1280, 720, 1);
    Sprite* personaje_lanza_sp = e.addSprite("personaje_lanza_sp", "personaje_lanza",
        0, 0, 1280, 720, 1);
    Sprite* cobija_techo_sp = e.addSprite("cobija_techo_sp", "cobija_techo",
        0, 0, 1280, 720, -1);

    // Sprites de dialogo
    Sprite* personaje_mira_sp = e.addSprite("personaje_mira_sp", "personaje_mira",
        0, 0, 1280, 720, 1);
    Sprite* habla1_sp = e.addSprite("habla1_sp", "personaje_habla1",
        0, 0, 1280, 720, 1);
    Sprite* habla2_sp = e.addSprite("habla2_sp", "personaje_habla2",
        0, 0, 1280, 720, 1);
    Sprite* habla3_sp = e.addSprite("habla3_sp", "personaje_habla3",
        0, 0, 1280, 720, 1);
    Sprite* hablaO_sp = e.addSprite("hablaO_sp", "personaje_hablaO",
        0, 0, 1280, 720, 1);

    // Prop flotante
    Sprite* alarma_sp = e.addSprite("alarma_sp", "alarma",
        0, 10, 80, 50, 4);

    // ── SPRITES ESCENA 3 ──────────────────────────────────────
    // bg_pasillo: fondo de pantalla completa
    Sprite* bg_pasillo_sp = e.addBackground("bg_pasillo_sp", "bg_pasillo", -96);

    // pasillo.png: personaje parado en el pasillo
    // Sprite portrait 1080x2340 → display 185x400
    Sprite* pasillo_sp = e.addSprite("pasillo_sp", "pasillo", 700, -30, 185, 400, 2);

    // intento1/2/3: mismas dimensiones portrait 1080x2340 → 185x400
    // Se posicionan donde quede pasillo_sp al varar (X=0)
    Sprite* intento1_sp = e.addSprite("intento1_sp", "intento1", 0, -30, 185, 400, 2);
    Sprite* intento2_sp = e.addSprite("intento2_sp", "intento2", 0, -30, 185, 400, 2);
    Sprite* intento3_sp = e.addSprite("intento3_sp", "intento3", 0, -30, 185, 400, 2);

    // gato: entra desde la derecha moviéndose a la izquierda
    Sprite* gato_sp = e.addSprite("gato_sp", "gato", 800, 30, 220, 160, 3);

    // sujetar_gato: X negativo = más a la izquierda para ver la mano
    Sprite* sujetar_sp = e.addSprite("sujetar_sp", "sujetar_gato", -100, -20, 640, 360, 10);

    // ── SPRITES ESCENA 4 ──────────────────────────────────────
    // Fondos
    Sprite* bg_patio_sp = e.addBackground("bg_patio_sp", "bg_patio", -96);
    Sprite* bg_mensaje_sp = e.addBackground("bg_mensaje_sp", "bg_mensaje", -95);
    Sprite* bg_intmsg_sp = e.addBackground("bg_intmsg_sp", "bg_int_mensaje", -95);
    // bg_vuelo: 2 copias para loop horizontal
    Sprite* bg_vuelo_a = e.addSprite("bg_vuelo_a", "bg_vuelo", 0, 0, 1280, 720, -96);
    Sprite* bg_vuelo_b = e.addSprite("bg_vuelo_b", "bg_vuelo", 1280, 0, 1280, 720, -96);

    // Personajes en patio — idle usa el primer frame de dialogo (boca cerrada/neutral)
    // per2 siempre scl.x=-1 para mirar hacia per1
    Sprite* per1_idle_sp = e.addSprite("per1_idle_sp", "per1_d1", 180, -80, 185, 400, 2);
    Sprite* per2_idle_sp = e.addSprite("per2_idle_sp", "per2_d1", -180, -80, 185, 400, 2);

    // Dialogo per1 (Z=3, encima del idle)
    Sprite* per1_d1_sp = e.addSprite("per1_d1_sp", "per1_d1", 180, -80, 185, 400, 3);
    Sprite* per1_d2_sp = e.addSprite("per1_d2_sp", "per1_d2", 180, -80, 185, 400, 3);
    Sprite* per1_d3_sp = e.addSprite("per1_d3_sp", "per1_d3", 180, -80, 185, 400, 3);
    Sprite* per1_dO_sp = e.addSprite("per1_dO_sp", "per1_dO", 180, -80, 185, 400, 3);

    // Dialogo per2 (espejado scl.x=-1)
    Sprite* per2_d1_sp = e.addSprite("per2_d1_sp", "per2_d1", -180, -80, 185, 400, 3);
    Sprite* per2_d2_sp = e.addSprite("per2_d2_sp", "per2_d2", -180, -80, 185, 400, 3);
    Sprite* per2_d3_sp = e.addSprite("per2_d3_sp", "per2_d3", -180, -80, 185, 400, 3);
    Sprite* per2_dO_sp = e.addSprite("per2_dO_sp", "per2_dO", -180, -80, 185, 400, 3);
    // Espejear todos los sprites de per2
    per2_idle_sp->scl = { -1.f,1.f };
    per2_d1_sp->scl = { -1.f,1.f }; per2_d2_sp->scl = { -1.f,1.f };
    per2_d3_sp->scl = { -1.f,1.f }; per2_dO_sp->scl = { -1.f,1.f };

    // Flashback
    Sprite* mensaje1_sp = e.addSprite("mensaje1_sp", "mensaje1", 0, 0, 600, 400, 5);
    Sprite* mensaje2_sp = e.addSprite("mensaje2_sp", "mensaje2", 0, 0, 600, 400, 5);
    Sprite* carta_sp = e.addSprite("carta_sp", "carta", 0, -200, 90, 58, 4);  // más pequeña
    Sprite* senora_sp = e.addSprite("senora_sp", "senora", 200, -500, 360, 460, 3); // más grande

    // Newton
    Sprite* newton_sp = e.addSprite("newton_sp", "newton", 100, -150, 200, 280, 2);
    Sprite* arbol_sp = e.addSprite("arbol_sp", "arbol", 60, 10, 480, 560, 1); // más grande
    Sprite* manzana_sp = e.addSprite("manzana_sp", "manzana", 220, 200, 45, 45, 3);
    Sprite* foco_sp2 = e.addSprite("foco_sp2", "foco_idea", 80, 120, 70, 90, 4);

    // Edificio que cae detrás de per1 en el cierre (Z=4, siempre detrás del personaje Z=5)
    Sprite* edif_fall = e.addSprite("edif_fall", "edificio2", 250, -20, 420, 800, 4);

    // Nubes de vuelo (paralaje, ligeramente más lentas que el fondo)
    Sprite* nubes_v1 = e.addSprite("nubes_v1", "nubes", 0, -60, 1280, 400, -50);
    Sprite* nubes_v2 = e.addSprite("nubes_v2", "nubes", 1280, -60, 1280, 400, -50);

    // Vuelo
    Sprite* per1_vuelo_sp = e.addSprite("per1_vuelo_sp", "per1_vuelo", -60, 20, 185, 400, 5);
    Sprite* per2_vuelo_sp = e.addSprite("per2_vuelo_sp", "per2_vuelo", 60, 20, 185, 400, 5);
    per2_vuelo_sp->scl = { -1.f, 1.f };

    // Edificios — tamaños grandes como edificios reales vistos desde el aire
    // edif[4] es el que pasa como "close-up" al final del vuelo
    Sprite* edif[5];
    edif[0] = e.addSprite("edif0", "edificio1", 1400, -50, 700, 900, 6);
    edif[1] = e.addSprite("edif1", "edificio2", 1400, -100, 500, 700, 5);
    edif[2] = e.addSprite("edif2", "edificio3", 1400, 0, 900, 1100, 7);
    edif[3] = e.addSprite("edif3", "edificio4", 1400, -80, 650, 850, 6);
    edif[4] = e.addSprite("edif4", "edificio5", 1400, 0, 2200, 2000, 8);  // gigante

    // Cables (sin transitando, solo el efecto visual de los cables)
    Sprite* cables1_sp = e.addSprite("cables1_sp", "cables", 1400, 0, 1280, 720, 9);
    Sprite* cables2_sp = e.addSprite("cables2_sp", "cables", 1400, 0, 1280, 720, 9);

    // ── SPRITES ESCENA 2 ──────────────────────────────────────
    // Backgrounds de la cocina (todos ocultos al inicio)
    Sprite* bg_cocina_sp = e.addBackground("bg_cocina_sp", "bg_cocina", -96);
    Sprite* bg_coc_sent_sp = e.addBackground("bg_coc_sent_sp", "bg_cocina_sentado", -95);
    Sprite* bg_coc_sent2_sp = e.addBackground("bg_coc_sent2_sp", "bg_cocina_sentado2", -94);

    // personaje_viendo_tele: si sigue aplastado, subir el primer valor (ancho)
    Sprite* viendo_sp = e.addSprite("viendo_sp", "personaje_viendo",
        -800, -50, 920, 540, 2);

    // 16 cereales pequeños (como puntos de pac-man) para volar y nivel
    Sprite* cer[16];
    for (int i = 0; i < 16; i++)
        cer[i] = e.addSprite("cer" + std::to_string(i), "cereal", 0, 0, 36, 36, 5);

    // Pac-Man: ancla invisible para movimiento + 3 fotogramas visuales
    // El ancla (pac) recibe todos los moveTo/rotateTo del timeline.
    // Los fotogramas (pac1/2/3) copian su posicion/rotacion en onFrame.
    Sprite* pac = e.addSprite("pac", "pacman1", -620, 80, 140, 140, 6);
    Sprite* pac1 = e.addSprite("pac1", "pacman1", -620, 80, 140, 140, 6);
    Sprite* pac2 = e.addSprite("pac2", "pacman2", -620, 80, 140, 140, 6);
    Sprite* pac3 = e.addSprite("pac3", "pacman3", -620, 80, 140, 140, 6);

    // =========================================================
    //  TIMELINE  -  Escena 1
    //
    //  Mapa de tiempos:
    //   t= 0.0  Fade de negro, fondos y audios arrancan
    //   t= 5.0  Camara sube hacia la casa (10 seg, EaseInOut)
    //   t=18.0  Zoom sobre casa_ext
    //   t=21.0  Corte al cuarto (vista completa)
    //   t=22.0  Zoom IN  hacia la cama (1.5 seg)
    //   t=25.5  Zoom OUT de regreso (1.5 seg)
    //   t=28.0  Fade de audios pre-alarma
    //   t=30.0  Suena la alarma
    //   t=33.0  Golpe: alarma se apaga
    //   t=33.4  Cobija vuelve a fotograma dormido
    //   t=34.5  Personaje se incorpora, lentes desaparecen
    //   t=43.0  Dialogo: mama pregunta
    //   t=48.0  Dialogo: respuesta
    //   t=53.0  FIN Escena 1
    // =========================================================

    auto& tl = e.timeline;

    alarma_sp->enableBob(5.f, 0.6f, 0.8f);

    // ── t=0 ──────────────────────────────────────────────────
    tl.at(0.0f, [&] {
        bg_tierra->show();
        bg_cielo_a->show();
        bg_cielo_b->show();
        casa_ext->show();
        for (int i = 0; i < 9; i++) n[i]->show();
        ns0->show(); ns1->show(); ns2->show();
        e.camera.pos = { 0.f, 0.f };
        e.audio.play("amanecer_cancion", 1.0f);
        e.audio.play("pajaros_fondo", 0.8f);
        }, "Inicio - fade de negro + audios");

    // ── t=5: Camara sube ─────────────────────────────────────
    tl.at(5.0f, [&] {
        e.camera.moveTo(0.f, 2100.f, 10.0f, Ease::EaseInOut);
        }, "Camara sube hacia la casa");

    // La camara pasa por la seam cielo-casa (Y≈1770) alrededor de t=14.
    // Ocultamos ns2 justo antes de llegar para que no se vea durante el zoom.
    tl.at(14.5f, [&] {
        ns2->hide();
        }, "Ocultar nube seam cielo-casa");

    // ── t=18: Zoom sobre la casa ─────────────────────────────
    tl.at(18.0f, [&] {
        casa_ext->scaleTo(2.5f, 2.5f, 3.0f, Ease::EaseInOut);
        }, "Zoom a la ventana");

    // ── t=21: Corte al cuarto — camara YA con zoom en la cama ──
    // No hay animacion de zoom: la camara llega directamente
    // enfocada en el area de la cama para que sea lo primero
    // que vea el espectador al entrar al cuarto.
    tl.at(21.0f, [&] {
        bg_tierra->hide();
        bg_cielo_a->hide();
        bg_cielo_b->hide();
        casa_ext->hide();
        for (int i = 0; i < 9; i++) n[i]->hide();
        ns0->hide(); ns1->hide(); ns2->hide();

        // Zoom instantaneo antes de mostrar el cuarto
        e.camera.pos = { 0.f, -120.f };
        e.camera.zoom = 1.75f;

        bg_cuarto->fadeIn(0.5f);
        cobija_dormido_sp->show();
        lentes_sp->show();   // lentes siempre visibles en el cuarto
        alarma_sp->show();

        alarma_sp->bobBase = 10.f;
        }, "Corte al cuarto - ya con zoom en la cama");

    // ── t=22: Fade de audios ──────────────────────────────────
    tl.at(22.0f, [&] {
        g_fadeAudio = true;
        g_fadeAudioElapsed = 0.f;
        }, "Pre-alarma: fade de audios");

    // ── t=24: Suena la alarma ────────────────────────────────
    // 3 segundos despues del corte al cuarto
    tl.at(24.0f, [&] {
        e.audio.play("alarma_sfx", 1.0f);
        }, "Alarma suena");

    // ── t=27: Golpe al despertador ───────────────────────────
    tl.at(27.0f, [&] {
        e.audio.stop("alarma_sfx");
        e.audio.play("golpe_despertador", 1.0f);

        cobija_dormido_sp->hide();
        cobija_golpe_sp->show();

        alarma_sp->bobEnabled = false;
        alarma_sp->moveTo(420, 260, 0.2f, Ease::EaseIn);
        alarma_sp->rotateTo(720.f, 0.25f);
        alarma_sp->fadeTo(0.f, 0.3f);
        }, "Golpe - cobija_golpe aparece");

    // ── t=27.4: Cobija vuelve a fotograma dormido ────────────
    tl.at(27.4f, [&] {
        cobija_golpe_sp->hide();
        cobija_dormido_sp->show();
        }, "Cobija vuelve a dormido");

    // ── t=28.4: Personaje sentado (primero personaje, luego cobija) ─
    tl.at(28.4f, [&] {
        personaje_sentado_sp->show();  // Z=1 primero
        cobija_dormido_sp->hide();  // sale cobija_dormido
        cobija_sp->show();  // Z=2 encima
        }, "Personaje sentado con cobija");

    // ── t=28.7: Lentes desaparecen (ya se los puso) ──────────
    tl.at(28.7f, [&] {
        lentes_sp->hide();
        }, "Lentes desaparecen - ya puestos");

    // ── t=29.4: Bostezo ──────────────────────────────────────
    tl.at(29.4f, [&] {
        personaje_sentado_sp->hide();
        personaje_bosteza_sp->show();
        // cobija permanece visible encima
        e.audio.play("bostezo", 1.0f);
        }, "Bostezo empieza");

    // ── t=32.4: Fin bostezo (3 seg) → lanza cobija ───────────
    tl.at(32.4f, [&] {
        personaje_bosteza_sp->hide();
        personaje_lanza_sp->show();
        cobija_sp->show(); // cobija aun visible antes de lanzar
        e.audio.play("lanza_cobija", 1.0f);
        }, "Personaje lanza la cobija");

    // La cobija sube y sale de pantalla (1/3 de segundo, lanzamiento)
    tl.at(32.5f, [&] {
        cobija_sp->moveTo(0, 500, 0.35f, Ease::EaseIn);
        }, "Cobija sube");

    // ── t=32.85: Cobija llego al techo ───────────────────────
    tl.at(32.85f, [&] {
        cobija_sp->hide();
        cobija_techo_sp->show();     // aparece la cobija en la cama del techo
        personaje_lanza_sp->hide();
        personaje_sentado_sp->show(); // vuelve a sentado
        }, "Cobija llega al techo - personaje vuelve a sentado");

    // ── t=35.5: Dialogo mama (3.2s → termina t=38.7) ────────
    tl.at(35.5f, [&] {
        e.audio.play("mama_dialogo", 1.0f);
        e.showSubtitle("Mama", "iHijo! iYa tendiste la cama?", 3.2f);
        }, "Dialogo - mama pregunta");

    // ── t=39.1: 0.4s despues que mama termina → hijo responde ─
    tl.at(39.1f, [&] {
        personaje_sentado_sp->hide();
        personaje_mira_sp->show();
        habla1_sp->show();
        e.audio.play("PDpersonaje", 1.0f);
        }, "Personaje empieza a hablar");

    // ── t=40.1: 1 segundo de dialogo cumplido → ZoomOut ──────
    tl.at(40.1f, [&] {
        e.camera.moveTo(0.f, 0.f, 1.8f, Ease::EaseInOut);
        e.camera.zoomTo(1.f, 1.8f, Ease::EaseInOut);
        }, "ZoomOut durante dialogo");

    // ── Ciclo de fotogramas de habla (lip sync) ───────────────
    tl.at(40.45f, [&] { habla1_sp->hide(); habla2_sp->show(); });
    tl.at(40.8f, [&] { habla2_sp->hide(); habla3_sp->show(); });
    tl.at(41.15f, [&] { habla3_sp->hide(); hablaO_sp->show(); });
    tl.at(41.5f, [&] { hablaO_sp->hide(); habla1_sp->show(); });
    tl.at(41.85f, [&] { habla1_sp->hide(); habla2_sp->show(); });
    tl.at(42.2f, [&] { habla2_sp->hide(); habla3_sp->show(); });
    tl.at(42.55f, [&] { habla3_sp->hide(); hablaO_sp->show(); });
    tl.at(42.9f, [&] { hablaO_sp->hide(); habla1_sp->show(); });
    tl.at(43.25f, [&] { habla1_sp->hide(); habla2_sp->show(); });
    tl.at(43.7f, [&] { habla2_sp->hide(); hablaO_sp->show(); }); // bug corregido

    // ── t=44.1: PDpersonaje termina → personaje en reposo ────
    tl.at(44.1f, [&] {
        habla1_sp->hide(); habla2_sp->hide();
        habla3_sp->hide(); hablaO_sp->hide();
        personaje_mira_sp->hide();
        personaje_sentado_sp->show();
        }, "Dialogo termina - personaje en reposo");

    // ── t=44.5: FIN ESCENA 1 → fade a negro ──────────────────
    tl.at(44.5f, [&] {
        personaje_sentado_sp->hide();
        cobija_techo_sp->hide();
        bg_cuarto->hide();
        alarma_sp->hide();
        e.audio.stop("amanecer_cancion");
        e.audio.stop("pajaros_fondo");
        g_transTarget = 1.0f;
        g_transSpeed = 2.5f;   // llega a negro en 0.4s
        }, "FIN ESCENA 1 - fade a negro");

    // ── t=45.3: Negro completo → mostrar bg_cocina ───────────
    // (44.5 + 0.4s de fade + 0.4s de negro puro = 45.3)
    tl.at(45.3f, [&] {
        bg_cocina_sp->show();
        e.camera.pos = { 0.f, 0.f };
        e.camera.zoom = 1.f;
        g_transTarget = 0.0f;   // abre desde negro
        e.audio.play("musica_fondo2", 0.8f);
        }, "Escena 2 - bg_cocina aparece");

    // ── t=46.3: Estática + personaje entra ───────────────────
    tl.at(46.3f, [&] {
        e.audio.play("estatica", 1.0f);
        e.showSubtitle("TV", "...retrasos en el cable de la Col. Centro...", 8.0f);
        viendo_sp->pos = { -800.f, -50.f };
        viendo_sp->alpha = 1.f;
        viendo_sp->show();
        viendo_sp->moveTo(-200, -50, 3.0f, Ease::EaseOut, [&] {
            viendo_sp->enableBob(8.f, 0.45f, 0.f);
            viendo_sp->bobBase = -50.f;
            });
        }, "Estatica + personaje entra");

    tl.at(47.3f, [&] {
        e.audio.play("dialogo_tele", 1.0f);
        }, "dialogo_tele empieza");

    // ── t=63.7: dialogo_tele termina (47.3+16.4) → estatica fade-out ─
    tl.at(63.7f, [&] {
        e.audio.play("estatica", 1.0f);
        e.audio.fadeOut("estatica", 1.0f);
        }, "Estatica fade-out");

    // ── t=64.3: Personaje sale ────────────────────────────────
    tl.at(64.3f, [&] {
        viendo_sp->bobEnabled = false;
        viendo_sp->fadeOut(0.5f);
        }, "Personaje sale");

    // ── t=65.3: Fade a negro → bg_cocina_sentado ─────────────
    tl.at(65.3f, [&] {
        g_transTarget = 1.0f;
        g_transSpeed = 2.5f;
        }, "Fade a negro pre-cocina_sentado");

    tl.at(66.0f, [&] {
        bg_cocina_sp->hide();
        bg_coc_sent_sp->show();
        g_transTarget = 0.0f;
        }, "bg_cocina_sentado visible");

    // ── t=68.0: bg_cocina_sentado_2 ──────────────────────────
    tl.at(68.0f, [&] {
        bg_coc_sent_sp->hide();
        bg_coc_sent2_sp->show();
        for (int i = 0; i < 16; i++) {
            cer[i]->pos = { -190.f, 0.f };
            cer[i]->alpha = 1.f;
            cer[i]->rot = 0.f;
        }
        }, "bg_cocina_sentado_2 listo");
    tl.at(68.1f, [&] {
        e.audio.play("cereal_caja", 1.0f);
        }, "cereal_caja sfx");

    // Cereales salen uno por uno, 0.15s entre cada uno
    tl.at(68.3f, [&] { cer[0]->show();  cer[0]->moveTo(-720, -400, 1.8f, Ease::EaseIn); cer[0]->rotateTo(360, 1.8f); });
    tl.at(68.45f, [&] { cer[1]->show();  cer[1]->moveTo(-480, -550, 1.8f, Ease::EaseIn); cer[1]->rotateTo(-360, 1.8f); });
    tl.at(68.60f, [&] { cer[2]->show();  cer[2]->moveTo(0, -600, 1.8f, Ease::EaseIn); cer[2]->rotateTo(360, 1.8f); });
    tl.at(68.75f, [&] { cer[3]->show();  cer[3]->moveTo(480, -550, 1.8f, Ease::EaseIn); cer[3]->rotateTo(-360, 1.8f); });
    tl.at(68.90f, [&] { cer[4]->show();  cer[4]->moveTo(720, -400, 1.8f, Ease::EaseIn); cer[4]->rotateTo(360, 1.8f); });
    tl.at(69.05f, [&] { cer[5]->show();  cer[5]->moveTo(-720, 0, 1.8f, Ease::EaseIn); cer[5]->rotateTo(-360, 1.8f); });
    tl.at(69.20f, [&] { cer[6]->show();  cer[6]->moveTo(-600, 420, 1.8f, Ease::EaseIn); cer[6]->rotateTo(360, 1.8f); });
    tl.at(69.35f, [&] { cer[7]->show();  cer[7]->moveTo(0, 600, 1.8f, Ease::EaseIn); cer[7]->rotateTo(-360, 1.8f); });
    tl.at(69.50f, [&] { cer[8]->show();  cer[8]->moveTo(600, 420, 1.8f, Ease::EaseIn); cer[8]->rotateTo(360, 1.8f); });
    tl.at(69.65f, [&] { cer[9]->show();  cer[9]->moveTo(720, 0, 1.8f, Ease::EaseIn); cer[9]->rotateTo(-360, 1.8f); });
    tl.at(69.80f, [&] { cer[10]->show(); cer[10]->moveTo(-500, -470, 1.8f, Ease::EaseIn); cer[10]->rotateTo(360, 1.8f); });
    tl.at(69.95f, [&] { cer[11]->show(); cer[11]->moveTo(500, -470, 1.8f, Ease::EaseIn); cer[11]->rotateTo(-360, 1.8f); });
    tl.at(70.10f, [&] { cer[12]->show(); cer[12]->moveTo(-650, 180, 1.8f, Ease::EaseIn); cer[12]->rotateTo(360, 1.8f); });
    tl.at(70.25f, [&] { cer[13]->show(); cer[13]->moveTo(650, 180, 1.8f, Ease::EaseIn); cer[13]->rotateTo(-360, 1.8f); });
    tl.at(70.40f, [&] { cer[14]->show(); cer[14]->moveTo(-300, 580, 1.8f, Ease::EaseIn); cer[14]->rotateTo(360, 1.8f); });
    tl.at(70.55f, [&] { cer[15]->show(); cer[15]->moveTo(300, 580, 1.8f, Ease::EaseIn); cer[15]->rotateTo(-360, 1.8f); });

    // ── t=72.5: bg_cocina + personaje + dialogo_pacman ───────
    tl.at(72.5f, [&] {
        bg_coc_sent2_sp->hide();
        bg_cocina_sp->show();
        const float row1Y = 80.f, row2Y = -80.f;
        const float xs[8] = { -490,-350,-210,-70, 70, 210, 350, 490 };
        for (int i = 0; i < 8; i++) { cer[i]->pos = { xs[i],row1Y };   cer[i]->rot = 0.f; cer[i]->alpha = 1.f; cer[i]->show(); }
        for (int i = 8; i < 16; i++) { cer[i]->pos = { xs[15 - i],row2Y }; cer[i]->rot = 0.f; cer[i]->alpha = 1.f; cer[i]->show(); }
        pac->pos = { -620.f,80.f }; pac->alpha = 1.f; pac->rot = 0.f;
        viendo_sp->bobEnabled = false;
        viendo_sp->pos = { -200.f,-50.f }; viendo_sp->alpha = 0.f; viendo_sp->show();
        viendo_sp->fadeTo(1.f, 0.6f, Ease::Linear, [&] {
            viendo_sp->enableBob(8.f, 0.45f, 0.f); viendo_sp->bobBase = -50.f;
            });
        e.audio.play("dialogo_pacman", 1.0f);
        }, "bg_cocina + dialogo_pacman");

    // ── t=74.7: termina dialogo_pacman → personaje sale izquierda ─
    tl.at(74.7f, [&] {
        viendo_sp->bobEnabled = false;
        viendo_sp->moveTo(-1000.f, -50.f, 0.8f, Ease::EaseIn, [&] { viendo_sp->hide(); });
        }, "Personaje sale a la izquierda");

    // ── t=75.8: Waka + Pac-Man (6.72s) ───────────────────────
    tl.at(75.8f, [&] {
        pac->rot = 0.f; g_pacActive = true; g_pacSeqIdx = 0; g_pacFrameTimer = 0.f;
        e.audio.play("waka", 1.0f);
        pac->moveTo(-490, 80, 0.42f, Ease::Linear);
        }, "Pac-Man empieza");

    tl.at(76.22f, [&] { cer[0]->hide();  pac->moveTo(-350, 80, 0.42f, Ease::Linear); });
    tl.at(76.64f, [&] { cer[1]->hide();  pac->moveTo(-210, 80, 0.42f, Ease::Linear); });
    tl.at(77.06f, [&] { cer[2]->hide();  pac->moveTo(-70, 80, 0.42f, Ease::Linear); });
    tl.at(77.48f, [&] { cer[3]->hide();  pac->moveTo(70, 80, 0.42f, Ease::Linear); });
    tl.at(77.90f, [&] { cer[4]->hide();  pac->moveTo(210, 80, 0.42f, Ease::Linear); });
    tl.at(78.32f, [&] { cer[5]->hide();  pac->moveTo(350, 80, 0.42f, Ease::Linear); });
    tl.at(78.74f, [&] { cer[6]->hide();  pac->moveTo(490, 80, 0.42f, Ease::Linear); });
    tl.at(79.16f, [&] { cer[7]->hide();  pac->moveTo(490, -80, 0.42f, Ease::Linear); pac->rot = -90.f; });
    tl.at(79.58f, [&] { cer[8]->hide();  pac->moveTo(350, -80, 0.42f, Ease::Linear); pac->rot = 180.f; });
    tl.at(80.00f, [&] { cer[9]->hide();  pac->moveTo(210, -80, 0.42f, Ease::Linear); });
    tl.at(80.42f, [&] { cer[10]->hide(); pac->moveTo(70, -80, 0.42f, Ease::Linear); });
    tl.at(80.84f, [&] { cer[11]->hide(); pac->moveTo(-70, -80, 0.42f, Ease::Linear); });
    tl.at(81.26f, [&] { cer[12]->hide(); pac->moveTo(-210, -80, 0.42f, Ease::Linear); });
    tl.at(81.68f, [&] { cer[13]->hide(); pac->moveTo(-350, -80, 0.42f, Ease::Linear); });
    tl.at(82.10f, [&] { cer[14]->hide(); pac->moveTo(-490, -80, 0.42f, Ease::Linear); });
    tl.at(82.52f, [&] { cer[15]->hide(); }); // → 75.8→82.52 = 6.72s ✓

    // ── t=73.3: musica_fondo2b arranca silenciosa ────────────
    // Cálculo: termina en t=199.3 (cuando per1 cae)
    // 199.3 - 126s = t=73.3
    tl.at(73.3f, [&] {
        e.audio.play("musica_fondo2b", 0.0f);
        }, "musica_fondo2b arranca silenciosa");

    // ── t=83.0: Regaño ───────────────────────────────────────
    tl.at(83.0f, [&] {
        e.audio.stop("waka");
        // musica_fondo2 BAJA a 0.1 durante el regano (no se detiene)
        e.audio.setVolume("musica_fondo2", 0.1f);
        e.audio.play("regano_comida", 1.0f);
        g_pacActive = false;
        pac1->visible = true; pac2->visible = false; pac3->visible = false;
        pac1->pos = pac->pos; pac1->rot = pac->rot;
        }, "Stop waka + regano_comida");

    // ── t=87.5: Muerte Pac-Man (83.0+4.5) ───────────────────
    tl.at(87.5f, [&] {
        g_pacActive = false; pac2->hide(); pac3->hide();
        pac1->pos = pac->pos; pac1->rot = 0.f; pac1->show();
        e.audio.play("pacman_muerte", 1.0f);
        // musica_fondo2 vuelve a volumen normal
        e.audio.setVolume("musica_fondo2", 0.7f);
        pac1->rotateTo(720.f, 1.5f, Ease::EaseIn);
        pac1->scaleTo(0.f, 0.f, 1.5f, Ease::EaseIn, [&] { pac1->hide(); });
        }, "Muerte Pac-Man");

    // ── t=91.0: FIN ESCENA 2 ─────────────────────────────────
    tl.at(91.0f, [&] {
        bg_cocina_sp->hide();
        // Swap: apagar primera instancia, subir la segunda que ya lleva 11s corriendo
        e.audio.stop("musica_fondo2");
        e.audio.setVolume("musica_fondo2b", 0.12f); // muy bajo para no tapar dialogos
        g_transTarget = 1.0f; g_transSpeed = 2.0f;
        }, "FIN ESCENA 2 - fade a negro");

    // ── t=91.7: bg_pasillo aparece ───────────────────────────
    tl.at(91.7f, [&] {
        bg_pasillo_sp->show();
        e.camera.pos = { 0.f,0.f }; e.camera.zoom = 1.f;
        g_transTarget = 0.0f;
        }, "Escena 3 - bg_pasillo");

    // ── t=92.5: Personaje entra ───────────────────────────────
    tl.at(92.5f, [&] {
        pasillo_sp->show();
        pasillo_sp->enableBob(6.f, 0.5f, 0.f);
        pasillo_sp->bobBase = -30.f;
        pasillo_sp->moveTo(0.f, -30.f, 4.0f, Ease::EaseInOut);
        }, "Personaje entra al pasillo");

    // ── t=94.7: Viento suena a mitad del camino ───────────────
    tl.at(94.7f, [&] {
        e.audio.play("viento_incomodo", 1.0f);
        }, "Viento antes de llegar");

    // ── t=97.7: Llega al centro — se para ─────────────────────
    tl.at(97.7f, [&] {
        pasillo_sp->clearTweens();
        pasillo_sp->bobEnabled = false;
        }, "Personaje varado al centro");

    // ── t=97.7: Voltea derecha/izquierda 2 veces (2s) ─────────
    tl.at(97.8f, [&] { pasillo_sp->scl = { -1.f,1.f }; });  // mira derecha
    tl.at(98.3f, [&] { pasillo_sp->scl = { 1.f,1.f }; });  // mira izquierda
    tl.at(98.8f, [&] { pasillo_sp->scl = { -1.f,1.f }; });  // mira derecha
    tl.at(99.3f, [&] { pasillo_sp->scl = { 1.f,1.f }; });  // mira izquierda

    // ── t=99.7: DAtorado.mp3 (2.7s) ──────────────────────────
    tl.at(99.7f, [&] {
        e.audio.play("DAtorado", 1.0f);
        e.showSubtitle("", "Uy, creo que... me quede atorado.", 3.0f);
        }, "Dialogo DAtorado");

    // ── t=102.4: DAtorado termina → secuencia de intentos ────
    tl.at(102.4f, [&] {
        pasillo_sp->hide();
        intento1_sp->show();
        intento1_sp->rot = 0.f; intento1_sp->scl = { 1.f,1.f };
        g_intentoActive = true;
        g_intentoIdx = 0;
        g_intentoTimer = 0.f;
        g_intentoAudioTimer = 0.f;
        g_intentoAudioIdx = 0;
        }, "Secuencia intentos empieza");

    // ── t=105.9: Meow — gato entra desde la IZQUIERDA ────────
    // scl.x=-1 → gato espejado, orientado hacia la derecha
    // moveTo de X=-800 a X=-320 (entra por izquierda, va a derecha)
    tl.at(105.9f, [&] {
        g_intentoActive = false;
        intento1_sp->hide(); intento2_sp->hide();
        intento3_sp->show();
        intento3_sp->rot = 0.f; intento3_sp->scl = { 1.f,1.f };
        e.audio.play("meow", 1.0f);
        gato_sp->pos = { -800.f, 30.f };
        gato_sp->scl = { -1.f, 1.f };  // espejado → mira a la derecha
        gato_sp->show();
        gato_sp->enableBob(5.f, 0.4f, 0.f);
        gato_sp->bobBase = 30.f;
        gato_sp->moveTo(-320.f, 30.f, 3.5f, Ease::EaseInOut);
        }, "Meow - gato entra desde izquierda");

    // ── t=109.4: Gato a 1/4 pantalla → zoom instantáneo ──────
    tl.at(109.4f, [&] {
        gato_sp->clearTweens();
        gato_sp->bobEnabled = false;
        gato_sp->hide();
        e.camera.zoom = 2.0f;
        e.camera.pos = { -150.f, 0.f };  // enfoca zona izquierda-centro
        // sujetar_gato espejado: mano sale desde la DERECHA
        sujetar_sp->scl = { -1.f, 1.f };
        sujetar_sp->show();
        intento3_sp->hide();
        }, "Zoom instantaneo + sujetar_gato espejado");

    // ── t=111.4: sujetar_gato sale disparado a la DERECHA ─────
    tl.at(111.4f, [&] {
        e.audio.play("lanzar_gato", 1.0f);
        sujetar_sp->moveTo(1400.f, -20.f, 0.4f, Ease::EaseIn);
        }, "Sujetar gato vuela a derecha");

    // ── t=111.9: Reset zoom ───────────────────────────────────
    tl.at(111.9f, [&] {
        e.camera.zoom = 1.f;
        e.camera.pos = { 0.f, 0.f };
        sujetar_sp->hide();
        // Personaje cruza hacia la izquierda desde X=0
        pasillo_sp->pos = { 0.f, -30.f };
        pasillo_sp->rot = 0.f;
        pasillo_sp->scl = { 1.f, 1.f };
        pasillo_sp->show();
        pasillo_sp->enableBob(6.f, 0.5f, 0.f);
        pasillo_sp->bobBase = -30.f;
        pasillo_sp->moveTo(-800.f, -30.f, 2.5f, Ease::EaseIn);
        // Gato sale disparado hacia la DERECHA con fuerza
        gato_sp->pos = { -320.f, 30.f };
        gato_sp->scl = { -1.f, 1.f };
        gato_sp->bobEnabled = false;
        gato_sp->show();
        gato_sp->moveTo(1000.f, 30.f, 0.6f, Ease::EaseIn);
        }, "Personaje cruza + gato vuela a derecha");

    // ── t=112.5: Gato sale de escena → golpe_vidrio ───────────
    tl.at(112.5f, [&] {
        gato_sp->hide();
        e.audio.play("golpe_vidrio", 1.0f);
        }, "Gato sale - golpe_vidrio");

    // ── t=114.4: Personaje sale de escena ────────────────────
    tl.at(114.4f, [&] {
        pasillo_sp->hide();
        }, "Personaje sale");

    // ── t=115.2: FIN ESCENA 3 → inicio Escena 4 ─────────────
    // abre_puerta.mp3 suena durante el negro de transicion
    tl.at(115.2f, [&] {
        bg_pasillo_sp->hide();
        e.audio.play("abre_puerta", 1.0f);
        g_transTarget = 1.0f; g_transSpeed = 2.5f;
        }, "FIN ESCENA 3 - fade a negro");

    // ── t=116.0: bg_patio aparece ────────────────────────────
    tl.at(116.0f, [&] {
        bg_patio_sp->show();
        e.camera.pos = { 0.f,0.f }; e.camera.zoom = 1.f;
        g_transTarget = 0.0f;
        }, "Escena 4 - bg_patio");

    // ── t=118.0: per1 entra, bob activo ──────────────────────
    tl.at(118.0f, [&] {
        per1_idle_sp->pos = { 700.f,-80.f }; per1_idle_sp->alpha = 1.f;
        per1_idle_sp->show();
        per1_idle_sp->enableBob(5.f, 0.45f, 0.f); per1_idle_sp->bobBase = -80.f;
        per1_idle_sp->moveTo(180.f, -80.f, 2.0f, Ease::EaseOut);
        }, "per1 entra al patio");

    // ── t=120.5: per2 llega desde la izquierda ───────────────
    tl.at(120.5f, [&] {
        per2_idle_sp->pos = { -700.f,-80.f }; per2_idle_sp->alpha = 1.f;
        per2_idle_sp->show();
        per2_idle_sp->enableBob(5.f, 0.45f, 0.3f); per2_idle_sp->bobBase = -80.f;
        per2_idle_sp->moveTo(-180.f, -80.f, 2.0f, Ease::EaseOut);
        }, "per2 llega al patio");

    // ── t=122.5: dialogo_final1 — per1 habla (2s) ────────────
    tl.at(122.5f, [&] {
        per1_idle_sp->hide();
        per1_d1_sp->show(); per1_d1_sp->scl = { 1.f,1.f };
        e.audio.play("dialogo_final1", 1.0f);
        e.showSubtitle("", "Eh, que rollo! Te mande mensaje y no me contestaste.", 3.0f);
        g_dia1Active = true; g_diaTimer = 0.f; g_diaIdx = 0;
        }, "dialogo_final1");

    // ── t=125.0: per1 termina (2s+0.5s) ─────────────────────
    tl.at(125.0f, [&] {
        g_dia1Active = false;
        per1_d1_sp->hide(); per1_d2_sp->hide();
        per1_d3_sp->hide(); per1_dO_sp->hide();
        per1_idle_sp->show();
        }, "per1 termina dialogo1");

    // ── t=125.5: dialogo_final2 — per2 habla (4s) ────────────
    tl.at(125.5f, [&] {
        per2_idle_sp->hide();
        per2_d1_sp->show(); per2_d1_sp->scl = { -1.f,1.f };
        e.audio.play("dialogo_final2", 1.0f);
        e.showSubtitle("", "Cual mensaje? Si a mi ni me llego nada... pase por ti para ver que onda.", 5.0f);
        g_dia2Active = true; g_diaTimer = 0.f; g_diaIdx = 0;
        }, "dialogo_final2");

    // ── t=130.0: per2 termina (4s+0.5s) ─────────────────────
    tl.at(130.0f, [&] {
        g_dia2Active = false;
        per2_d1_sp->hide(); per2_d2_sp->hide();
        per2_d3_sp->hide(); per2_dO_sp->hide();
        per2_idle_sp->show();
        }, "per2 termina dialogo2");

    // ── t=130.5: dialogo_final3 — per1 habla (1s) ────────────
    tl.at(130.5f, [&] {
        per1_idle_sp->hide();
        per1_d1_sp->show(); per1_d1_sp->scl = { 1.f,1.f };
        e.audio.play("dialogo_final3", 1.0f);
        e.showSubtitle("", "Y entonces, que paso con el mensaje?", 2.0f);
        g_dia1Active = true; g_diaTimer = 0.f; g_diaIdx = 0;
        }, "dialogo_final3");

    tl.at(132.0f, [&] {
        g_dia1Active = false;
        per1_d1_sp->hide(); per1_d2_sp->hide();
        per1_d3_sp->hide(); per1_dO_sp->hide();
        per1_idle_sp->show();
        }, "per1 termina dialogo3");

    // ── t=134.0: 2s mirándose → FLASH BLANCO (flashback) ─────
    tl.at(134.0f, [&] {
        e.audio.play("recuerdo", 1.0f);
        g_flashTarget = 1.0f; g_flashSpeed = 3.0f;
        }, "Flash blanco inicio flashback");

    // ── t=134.8: bg_interior_mensaje ─────────────────────────
    tl.at(134.8f, [&] {
        per1_idle_sp->hide(); per2_idle_sp->hide(); bg_patio_sp->hide();
        bg_intmsg_sp->show();
        g_flashTarget = 0.0f;  // abre desde blanco
        }, "bg_interior_mensaje");

    // ── t=137.0: zoom + Mensaje1 ─────────────────────────────
    tl.at(137.0f, [&] {
        e.camera.zoomTo(1.8f, 0.6f, Ease::EaseIn);
        }, "Zoom a mensaje");

    tl.at(137.7f, [&] {
        mensaje1_sp->fadeIn(0.4f);
        }, "Mensaje1 aparece");

    // ── t=139.2: Mensaje2 cambio brusco (sin fade) + mensaje_envio ─
    tl.at(139.2f, [&] {
        mensaje1_sp->hide();        // cambio instantaneo, sin fade
        mensaje2_sp->show();
        e.audio.play("mensaje_envio", 1.0f);
        }, "Mensaje2 + envio");

    // ── t=139.9: Catapulta sound
    tl.at(139.9f, [&] {
        e.audio.play("catapulta_sfx", 1.0f);
        }, "Catapulta despues de mensaje");

    // ── t=141.0: bg_cielo + carta + señora en la misma escena ───
    // La carta viene desde la izquierda hacia donde estará la señora
    tl.at(141.0f, [&] {
        mensaje2_sp->hide(); bg_intmsg_sp->hide();
        e.camera.zoom = 1.f; e.camera.pos = { 0.f,0.f };
        // Fondo de cielo directamente (no bg_mensaje)
        bg_cielo_a->show(); bg_cielo_a->pos = { 0.f,0.f };
        // Señora empieza a subir desde abajo
        senora_sp->pos = { 200.f,-500.f }; senora_sp->alpha = 1.f;
        senora_sp->show();
        senora_sp->enableBob(4.f, 0.4f, 0.f); senora_sp->bobBase = 0.f;
        senora_sp->moveTo(200.f, 0.f, 2.2f, Ease::EaseOut);
        // Carta sale desde fuera de la pantalla izquierda y vuela hacia la señora
        carta_sp->pos = { -750.f, 0.f }; carta_sp->alpha = 1.f; carta_sp->rot = 0.f;
        carta_sp->show();
        carta_sp->moveTo(200.f, 0.f, 2.0f, Ease::EaseIn); // vuela directo a la señora
        carta_sp->rotateTo(360.f, 2.0f);
        }, "Carta vuela hacia señora en bg_cielo");

    // ── t=142.9: carta LLEGA a la señora — impacto ────────────
    tl.at(142.9f, [&] {
        carta_sp->hide();
        e.audio.play("feldspar_golpe", 1.0f);
        // Señora sale volando arriba-derecha en el mismo instante
        senora_sp->bobEnabled = false;
        senora_sp->rotateTo(-540.f, 1.0f, Ease::EaseIn);
        senora_sp->moveTo(650.f, 650.f, 1.0f, Ease::EaseIn, [&] { senora_sp->hide(); });
        }, "Carta golpea - señora sale arriba-derecha");

    // ── t=145.5: señora sale → flash blanco de vuelta ────────
    tl.at(145.5f, [&] {
        bg_cielo_a->hide();
        e.audio.play("recuerdo", 1.0f);
        g_flashTarget = 1.0f; g_flashSpeed = 3.0f;
        }, "Flash blanco vuelta al presente");

    tl.at(146.3f, [&] {
        bg_patio_sp->show();
        per1_idle_sp->show(); per2_idle_sp->show();
        e.camera.zoom = 1.f; e.camera.pos = { 0.f,0.f };
        g_flashTarget = 0.0f;
        }, "Vuelta al patio post-flashback");

    // ── t=149.5: dialogo_final4 — per1 (1.8s) ────────────────
    tl.at(149.5f, [&] {
        per1_idle_sp->hide();
        per1_d1_sp->show(); per1_d1_sp->scl = { 1.f,1.f };
        e.audio.play("dialogo_final4", 1.0f);
        g_dia1Active = true; g_diaTimer = 0.f; g_diaIdx = 0;
        }, "dialogo_final4");

    tl.at(151.8f, [&] {
        g_dia1Active = false;
        per1_d1_sp->hide(); per1_d2_sp->hide();
        per1_d3_sp->hide(); per1_dO_sp->hide();
        per1_idle_sp->show();
        }, "per1 termina dialogo4");

    // ── t=152.5: dialogo_mama_vuelo (2.5s) ───────────────────
    tl.at(152.5f, [&] {
        e.audio.play("dialogo_mama_vuelo", 1.0f);
        e.showSubtitle("Mama", "Ya se van? Como se van, en catapulta o por los cables?", 3.5f);
        }, "dialogo_mama_vuelo");

    // ── t=155.5: dialogo_final5 — per2 (2.2s) ────────────────
    tl.at(155.5f, [&] {
        e.audio.setVolume("musica_fondo2b", 0.04f); // casi muda durante dialogos 5 y 6
        per2_idle_sp->hide();
        per2_d1_sp->show(); per2_d1_sp->scl = { -1.f,1.f };
        e.audio.play("dialogo_final5", 1.0f);
        g_dia2Active = true; g_diaTimer = 0.f; g_diaIdx = 0;
        }, "dialogo_final5");

    tl.at(158.2f, [&] {
        g_dia2Active = false;
        per2_d1_sp->hide(); per2_d2_sp->hide();
        per2_d3_sp->hide(); per2_dO_sp->hide();
        per2_idle_sp->show();
        }, "per2 termina dialogo5");

    // ── t=158.7: dialogo_final6 — per1 (1.4s) ────────────────
    tl.at(158.7f, [&] {
        per1_idle_sp->hide();
        per1_d1_sp->show(); per1_d1_sp->scl = { 1.f,1.f };
        e.audio.play("dialogo_final6", 1.0f);
        g_dia1Active = true; g_diaTimer = 0.f; g_diaIdx = 0;
        }, "dialogo_final6");

    tl.at(160.6f, [&] {
        g_dia1Active = false;
        per1_d1_sp->hide(); per1_d2_sp->hide();
        per1_d3_sp->hide(); per1_dO_sp->hide();
        per1_idle_sp->show();
        // Restaurar musica al volumen bajo normal
        e.audio.setVolume("musica_fondo2b", 0.12f);
        }, "per1 termina dialogo6");

    // ── t=161.5: fade a negro 3s + catapulta ─────────────────
    tl.at(161.5f, [&] {
        g_transTarget = 1.0f; g_transSpeed = 0.8f; // fade lento ~3s
        e.audio.play("catapulta_sfx", 1.0f);
        }, "Fade a negro catapulta");

    tl.at(162.5f, [&] {
        per1_idle_sp->hide(); per2_idle_sp->hide();
        bg_patio_sp->hide();
        }, "Ocultar patio en negro");

    // ── t=164.5: Vuelo comienza ───────────────────────────────
    tl.at(164.5f, [&] {
        bg_vuelo_a->show(); bg_vuelo_b->show();
        nubes_v1->show(); nubes_v2->show();  // nubes en paralaje
        per1_vuelo_sp->show();
        per2_vuelo_sp->show();
        per1_vuelo_sp->enableBob(8.f, 0.4f, 0.0f); per1_vuelo_sp->bobBase = 20.f;
        per2_vuelo_sp->enableBob(8.f, 0.4f, 0.5f); per2_vuelo_sp->bobBase = 20.f;
        g_flightActive = true;
        g_vueloX = 0.f; g_nubesVX = 0.f;
        g_transTarget = 0.0f;
        // Musica sube al empezar el vuelo
        e.audio.setVolume("musica_fondo2b", 0.65f);
        }, "Escena vuelo comienza");

    // Edificios — aparecen más seguido, velocidades altas para que se noten
    tl.at(165.5f, [&] { g_edif[0].active = true; g_edif[0].x = 1400.f; g_edif[0].speed = 130.f; edif[0]->show(); }, "edif0");
    tl.at(167.5f, [&] { g_edif[1].active = true; g_edif[1].x = 1400.f; g_edif[1].speed = 90.f; edif[1]->show(); }, "edif1");

    // Primera aparicion de cables (sin transitando)
    tl.at(170.0f, [&] {
        cables1_sp->pos = { 1400.f,0.f }; cables1_sp->show();
        g_cable1Active = true; g_cableX1 = 1400.f;
        }, "Cables 1 aparecen");

    tl.at(172.0f, [&] { g_edif[2].active = true; g_edif[2].x = 1400.f; g_edif[2].speed = 160.f; edif[2]->show(); }, "edif2");

    tl.at(174.5f, [&] {
        cables1_sp->hide(); g_cable1Active = false;
        }, "Cables 1 fuera");

    tl.at(176.0f, [&] { g_edif[3].active = true; g_edif[3].x = 1400.f; g_edif[3].speed = 110.f; edif[3]->show(); }, "edif3");

    // Segunda aparicion de cables
    tl.at(180.0f, [&] {
        cables2_sp->pos = { 1400.f,0.f }; cables2_sp->show();
        g_cable2Active = true; g_cableX2 = 1400.f;
        }, "Cables 2 aparecen");

    tl.at(183.0f, [&] {
        cables2_sp->hide(); g_cable2Active = false;
        }, "Cables 2 fuera");

    // ── t=186: Edificio 5 gigante pasa tapando casi toda la pantalla ─
    tl.at(186.0f, [&] {
        g_edif[4].active = true; g_edif[4].x = 1400.f; g_edif[4].speed = 500.f;
        edif[4]->show();
        }, "Edificio5 gigante pasa");

    // ── t=189.0: Corte a Newton ───────────────────────────────
    tl.at(189.0f, [&] {
        g_flightActive = false;
        bg_vuelo_a->hide(); bg_vuelo_b->hide();
        nubes_v1->hide(); nubes_v2->hide();
        for (int i = 0; i < 5; i++) { edif[i]->hide(); g_edif[i].active = false; }
        cables1_sp->hide(); cables2_sp->hide();
        per1_vuelo_sp->hide(); per2_vuelo_sp->hide();
        // bg_cielo_a NO se muestra aquí: taparía bg_tierra (cielo Z=-99 > tierra Z=-100)
        bg_cielo_a->hide();  // asegurar que esté oculto
        // Mostrar SOLO bg_tierra
        bg_tierra->pos = { 0.f,0.f };
        bg_tierra->show();
        arbol_sp->pos = { 80.f, 30.f };   arbol_sp->show();   // base del arbol en Y≈-200
        newton_sp->pos = { 120.f,-195.f }; newton_sp->show();
        manzana_sp->pos = { 220.f, 200.f }; manzana_sp->alpha = 1.f; manzana_sp->show();
        e.camera.pos = { 0.f,0.f }; e.camera.zoom = 1.f;
        g_transTarget = 0.0f;
        }, "Escena Newton - SOLO bg_tierra");

    // ── t=191.5: Manzana cae ─────────────────────────────────
    tl.at(191.5f, [&] {
        manzana_sp->moveTo(120.f, -195.f, 1.2f, Ease::EaseIn);
        }, "Manzana cae");

    // ── t=192.7: 1s pause tras impacto ───────────────────────
    tl.at(192.7f, [&] {
        manzana_sp->hide();
        }, "Manzana impacta");

    // ── t=193.7: foco + idea ─────────────────────────────────
    tl.at(193.7f, [&] {
        foco_sp2->pos = { 80.f,-115.f }; foco_sp2->alpha = 0.f; foco_sp2->show();
        foco_sp2->fadeIn(0.4f);
        e.audio.play("idea_sfx", 1.0f);
        e.showSubtitle("", "...!", 2.0f);
        }, "Foco de Newton");

    // ── t=195.0: CAOS — Newton desaparece, bg_vuelo vuelve ───
    // NO hay camera shake aquí todavía
    tl.at(195.0f, [&] {
        arbol_sp->hide();
        newton_sp->hide();
        foco_sp2->hide();
        manzana_sp->hide();
        bg_tierra->hide();
        bg_cielo_a->hide();
        // bg_vuelo aparece estático (g_flightActive=false → no scrollea)
        bg_vuelo_a->pos = { 0.f,0.f };   bg_vuelo_a->show();
        bg_vuelo_b->pos = { 1280.f,0.f }; bg_vuelo_b->show();
        nubes_v1->pos = { 0.f,-60.f };   nubes_v1->show();
        nubes_v2->pos = { 1280.f,-60.f }; nubes_v2->show();
        g_flightActive = false;  // SIN scroll — fondo estático para el cierre dramático
        // Edificio detrás del personaje, visible desde el inicio del caos
        edif_fall->pos = { 250.f,-20.f }; edif_fall->alpha = 1.f; edif_fall->show();
        // per1 aparece pequeño y escala
        per1_vuelo_sp->scl = { 0.04f,0.04f };
        per1_vuelo_sp->pos = { 0.f, 30.f };
        per1_vuelo_sp->alpha = 1.f;
        per1_vuelo_sp->bobEnabled = false;
        per1_vuelo_sp->show();
        per1_vuelo_sp->scaleTo(5.5f, 5.5f, 2.0f, Ease::EaseIn);
        }, "Caos - per1 escala, fondo estatico");

    // ── t=197.0: per1 enorme — 1.3s de pausa dramática ───────
    // NO shake todavía. El personaje llena la pantalla.

    // ── t=198.3: camera shake (1s) ───────────────────────────
    tl.at(198.3f, [&] {
        e.camera.shake(20.f, 1.0f);
        }, "Camera shake pre-caida");

    // ── t=199.3: per1 cae rápido + edificio cae detrás ───────
    // musica_fondo2b: 73.3 + 126 = 199.3 ✓
    tl.at(199.3f, [&] {
        g_flightActive = false;
        per1_vuelo_sp->bobEnabled = false;
        // per1 y edificio caen juntos a la misma velocidad
        per1_vuelo_sp->moveTo(0.f, -2000.f, 0.25f, Ease::EaseIn);
        edif_fall->moveTo(250.f, -2000.f, 0.25f, Ease::EaseIn, [&] { edif_fall->hide(); });
        g_transTarget = 1.0f; g_transSpeed = 0.5f;
        }, "per1 + edificio caen - musica termina");

    tl.at(202.5f, [&] {
        per1_vuelo_sp->hide(); bg_vuelo_a->hide(); bg_vuelo_b->hide();
        nubes_v1->hide(); nubes_v2->hide(); edif_fall->hide();
        }, "FIN");

    // =========================================================
    //  onFrame  -  logica cada fotograma (ANTES de dibujar)
    // =========================================================

    e.onFrame = [&](float dt) {

        // ── Fade negro de intro ───────────────────────────────
        if (g_overlayAlpha > 0.f && e.getTime() < 2.0f)
            g_overlayAlpha = std::max(0.f, g_overlayAlpha - dt * 0.5f);

        // ── Fade procedural de audios de intro ────────────────
        if (g_fadeAudio) {
            g_fadeAudioElapsed += dt;
            float t = std::min(g_fadeAudioElapsed / 2.0f, 1.0f);
            e.audio.setVolume("amanecer_cancion", 1.0f - 0.75f * t);
            e.audio.setVolume("pajaros_fondo", 0.8f - 0.80f * t);
            if (g_fadeAudioElapsed >= 2.0f) g_fadeAudio = false;
        }

        // ── Overlay negro entre escenas ───────────────────────
        if (g_transOverlay < g_transTarget)
            g_transOverlay = std::min(g_transTarget, g_transOverlay + g_transSpeed * dt);
        else if (g_transOverlay > g_transTarget)
            g_transOverlay = std::max(g_transTarget, g_transOverlay - g_transSpeed * dt);

        // ── Flash blanco (flashbacks) ─────────────────────────
        if (g_flashAlpha < g_flashTarget)
            g_flashAlpha = std::min(g_flashTarget, g_flashAlpha + g_flashSpeed * dt);
        else if (g_flashAlpha > g_flashTarget)
            g_flashAlpha = std::max(g_flashTarget, g_flashAlpha - g_flashSpeed * dt);

        // ── Ciclo de intentos (Escena 3) ──────────────────────
        if (g_intentoActive) {
            const int   frameSeq[8] = { 0,1,2,1,2,0,1,2 };
            const float rotSeq[8] = { 0.f,15.f,-10.f,20.f,-15.f,5.f,-20.f,10.f };
            const float scaleSeq[8] = { 1.f,-1.f,1.f,-1.f,1.f,-1.f,1.f,-1.f };
            g_intentoTimer += dt;
            if (g_intentoTimer >= 0.2f) {
                g_intentoTimer = 0.f;
                g_intentoIdx = (g_intentoIdx + 1) % 8;
                int fi = frameSeq[g_intentoIdx];
                float rot = rotSeq[g_intentoIdx];
                float scx = scaleSeq[g_intentoIdx];
                intento1_sp->visible = (fi == 0); intento2_sp->visible = (fi == 1); intento3_sp->visible = (fi == 2);
                if (fi == 0) { intento1_sp->rot = rot; intento1_sp->scl = { scx,1.f }; }
                if (fi == 1) { intento2_sp->rot = rot; intento2_sp->scl = { scx,1.f }; }
                if (fi == 2) { intento3_sp->rot = rot; intento3_sp->scl = { scx,1.f }; }
            }
            g_intentoAudioTimer += dt;
            if (g_intentoAudioTimer >= 0.6f) {
                g_intentoAudioTimer = 0.f;
                const char* sfx[3] = { "intento1_sfx","intento2_sfx","intento3_sfx" };
                e.audio.play(sfx[g_intentoAudioIdx % 3], 0.85f);
                g_intentoAudioIdx++;
            }
        }

        // ── Ciclo Pac-Man ─────────────────────────────────────
        if (g_pacActive) {
            pac1->pos = pac->pos; pac2->pos = pac->pos; pac3->pos = pac->pos;
            pac1->rot = pac->rot; pac2->rot = pac->rot; pac3->rot = pac->rot;
            g_pacFrameTimer += dt;
            if (g_pacFrameTimer >= 0.12f) {
                g_pacFrameTimer = 0.f;
                g_pacSeqIdx = (g_pacSeqIdx + 1) % 4;
            }
            const int seq[4] = { 0,1,2,1 };
            int frame = seq[g_pacSeqIdx];
            pac1->visible = (frame == 0); pac2->visible = (frame == 1); pac3->visible = (frame == 2);
        }

        // ── Ciclo dialogo lip-sync per1 ───────────────────────
        // Secuencia: {1,2,3,O,1,2,3,O...} = indices {0,1,2,3}
        if (g_dia1Active) {
            g_diaTimer += dt;
            if (g_diaTimer >= 0.3f) {
                g_diaTimer = 0.f;
                g_diaIdx = (g_diaIdx + 1) % 4;
            }
            per1_d1_sp->visible = (g_diaIdx == 0);
            per1_d2_sp->visible = (g_diaIdx == 1);
            per1_d3_sp->visible = (g_diaIdx == 2);
            per1_dO_sp->visible = (g_diaIdx == 3);
        }

        // ── Ciclo dialogo lip-sync per2 ───────────────────────
        if (g_dia2Active) {
            g_diaTimer += dt;
            if (g_diaTimer >= 0.3f) {
                g_diaTimer = 0.f;
                g_diaIdx = (g_diaIdx + 1) % 4;
            }
            per2_d1_sp->visible = (g_diaIdx == 0);
            per2_d2_sp->visible = (g_diaIdx == 1);
            per2_d3_sp->visible = (g_diaIdx == 2);
            per2_dO_sp->visible = (g_diaIdx == 3);
        }

        // ── Secuencia de vuelo ────────────────────────────────
        if (g_flightActive) {
            const float speed = 80.f; // px/s del fondo

            // Loop de bg_vuelo (2 copias lado a lado)
            g_vueloX -= speed * dt;
            if (g_vueloX <= -1280.f) g_vueloX += 1280.f;
            bg_vuelo_a->pos.x = g_vueloX;
            bg_vuelo_b->pos.x = g_vueloX + 1280.f;

            // Scroll de edificios
            for (int i = 0; i < 5; i++) {
                if (!g_edif[i].active) continue;
                g_edif[i].x -= g_edif[i].speed * dt;
                edif[i]->pos.x = g_edif[i].x;
                // Umbral ampliado para que el edificio salga completamente de pantalla
                if (g_edif[i].x < -2000.f) {
                    g_edif[i].active = false;
                    edif[i]->hide();
                }
            }

            // Nubes en paralaje (más lentas que el fondo, efecto de profundidad)
            g_nubesVX -= 55.f * dt;
            if (g_nubesVX <= -1280.f) g_nubesVX += 1280.f;
            nubes_v1->pos.x = g_nubesVX;
            nubes_v2->pos.x = g_nubesVX + 1280.f;

            // Cables (sin transitando)
            if (g_cable1Active) {
                g_cableX1 -= 80.f * dt;
                cables1_sp->pos.x = g_cableX1;
            }
            if (g_cable2Active) {
                g_cableX2 -= 80.f * dt;
                cables2_sp->pos.x = g_cableX2;
            }
        }
        };

    // ── onPostFrame: overlays encima de todo ──────────────────
    e.onPostFrame = [&]() {
        if (g_overlayAlpha > 0.f)
            e.drawColorOverlay(g_overlayAlpha);
        if (g_transOverlay > 0.f)
            e.drawColorOverlay(g_transOverlay);
        // Flash blanco (flashback)
        if (g_flashAlpha > 0.f)
            e.drawColorOverlay(g_flashAlpha, 1.f, 1.f, 1.f);
        };

    // =========================================================
    e.run();
    return 0;
}