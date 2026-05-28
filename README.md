================================================================================
  SIN GRAVEDAD — Animación en OpenGL/C++
  Vega Hurtado Braulio Eduardo
  Graficación — Instituto Tecnológico de Ciudad Juárez
================================================================================

ARCHIVOS EN EL REPOSITORIO
────────────────────────────────────────────────────────────────────────────────
  El repositorio contiene únicamente el código fuente:

    Engine.h        Motor de animación (declaraciones)
    Engine.cpp      Motor de animación (implementación)
    Main.cpp        Guión completo de la animación
    stb_image.h     Biblioteca para carga de imágenes (ya incluida)

  Los assets (imágenes y audio) NO están incluidos en el repositorio
  por limitaciones de tamaño. Ver sección "Assets" más abajo.

  El instalador del FMOD Engine también está disponible en el repositorio
  como referencia, pero se recomienda descargarlo directamente desde el
  sitio oficial para obtener la versión correcta para tu sistema operativo.


REQUISITOS DEL SISTEMA
────────────────────────────────────────────────────────────────────────────────
  Windows:
    - Windows 10 / 11 (64-bit)
    - Visual Studio 2019 o superior
      (carga de trabajo "Desarrollo para escritorio con C++")
    - Tarjeta gráfica con soporte OpenGL 2.0 o superior

  Linux:
    - GCC 9 o superior
    - freeglut, OpenGL y GLU instalables vía apt/dnf (ver más abajo)
    - Tarjeta gráfica con soporte OpenGL 2.0 o superior


DEPENDENCIAS EXTERNAS
────────────────────────────────────────────────────────────────────────────────

1. FREEGLUT
   ─────────
   freeglut crea la ventana y gestiona el bucle principal de OpenGL.

   Windows — Descarga manual:
     https://www.transmissionzero.co.uk/software/freeglut-devel/
     → Descarga "freeglut 3.x.x MSVC Package"

     Instalación:
       a) Descomprime el ZIP.
       b) Copia los archivos a Visual Studio:

          freeglut\include\GL\*          →  C:\Program Files\Microsoft Visual
                                            Studio\<version>\Community\VC\Tools\
                                            MSVC\<ver>\include\GL\

          freeglut\lib\x86\freeglut.lib  →  ...\MSVC\<ver>\lib\x86\

          freeglut\bin\x86\freeglut.dll  →  Carpeta raíz del proyecto

   Windows — Alternativa con NuGet (más rápido):
     Visual Studio → Herramientas → Administrador de paquetes NuGet →
     Buscar "nupengl.core" e instalar.

   Linux (Ubuntu / Debian):
     sudo apt update
     sudo apt install freeglut3-dev libgl1-mesa-dev libglu1-mesa-dev

   Linux (Fedora / RHEL):
     sudo dnf install freeglut-devel mesa-libGL-devel mesa-libGLU-devel


2. FMOD ENGINE
   ─────────────
   FMOD es la API de audio utilizada para reproducir música y efectos
   de sonido. Es gratuita para proyectos no comerciales y educativos.

   Descarga (selecciona tu sistema operativo en el sitio):
     https://www.fmod.com/download
     → Crear una cuenta gratuita (requerida)
     → Sección "FMOD Engine" → elegir Windows o Linux → versión más reciente

   Licencia: https://www.fmod.com/licensing

   ── Windows ──
     a) Ejecuta el instalador. Ruta por defecto:
        C:\Program Files (x86)\FMOD SoftWare\FMOD Studio API Windows\

     b) Copia estos archivos a la carpeta raíz del proyecto:
          api\core\inc\fmod.hpp
          api\core\inc\fmod_common.h
          api\core\lib\x86\fmod_vc.lib
          api\core\lib\x86\fmod.dll    ← también junto al .exe al ejecutar

   ── Linux ──
     a) Descarga el paquete .tar.gz para Linux y extráelo:
        tar -xzf fmodstudioapi<version>linux.tar.gz

     b) Copia los archivos al proyecto:
          api/core/inc/fmod.hpp          →  carpeta del proyecto
          api/core/inc/fmod_common.h     →  carpeta del proyecto
          api/core/lib/x86_64/libfmod.so →  carpeta del proyecto
                                             (también a /usr/local/lib/)

     c) Actualiza el caché de bibliotecas:
        sudo ldconfig


3. STB_IMAGE
   ──────────
   Ya está incluida en el repositorio como stb_image.h.
   No requiere instalación adicional.


CONFIGURACIÓN DEL PROYECTO
────────────────────────────────────────────────────────────────────────────────

  ── Windows — Visual Studio ──

    1. Crea un nuevo proyecto vacío de C++ en Visual Studio.
    2. Agrega los archivos: Engine.h, Engine.cpp, Main.cpp, stb_image.h
       y los headers de FMOD (fmod.hpp, fmod_common.h).
    3. Clic derecho en el proyecto → Propiedades:

       C/C++ → General → Directorios de inclusión adicionales:
         $(ProjectDir)

       Vinculador → General → Directorios de bibliotecas adicionales:
         $(ProjectDir)

       Vinculador → Entrada → Dependencias adicionales:
         freeglut.lib
         fmod_vc.lib
         opengl32.lib
         glu32.lib

    4. La plataforma DEBE ser x86 (32 bits). En la barra superior cambia
       la plataforma a "x86". Usar x64 causará errores de enlazado.
       Usa los archivos .lib y .dll de la carpeta x86 de cada dependencia.
    5. Compila con F7 o Ctrl+Shift+B.

  ── Linux — g++ ──

    Compila con el siguiente comando desde la carpeta del proyecto:

      g++ -std=c++17 -o SinGravedad Engine.cpp Main.cpp \
          -lGL -lGLU -lfreeglut -lfmod -L. \
          -Wl,-rpath,.

    Luego ejecuta:
      ./SinGravedad


ASSETS REQUERIDOS
────────────────────────────────────────────────────────────────────────────────

  Los assets deben colocarse en la siguiente estructura de carpetas,
  en la misma ubicación que el ejecutable generado:

  assets/
  ├── backgrounds/      ← imágenes PNG 1280×720 px (sin canal alfa)
  │     bg_tierra.png
  │     bg_cielo.png
  │     bg_casa_exterior.png
  │     bg_cuarto.png
  │     bg_cocina.png
  │     bg_cocina_sentado.png
  │     bg_cocina_sentado_2.png
  │     bg_pasillo.png
  │     bg_patio.png
  │     bg_mensaje.png
  │     bg_interior_mensaje.png
  │     bg_vuelo.png
  │     bg_tierra_lejana.png
  │
  ├── sprites/          ← PNG con canal alfa (fondo transparente, PNG-32)
  │     nubes.png
  │     alarma.png
  │     lentes.png
  │     cobija_dormido.png
  │     cobija_golpe.png
  │     cobija.png
  │     cobija_techo.png
  │     personaje_sentado_cama.png
  │     personaje_bosteza.png
  │     personaje_lanza.png
  │     personaje_mira_arriba.png
  │     personaje_habla1.png
  │     personaje_habla2.png
  │     personaje_habla3.png
  │     personaje_hablaO.png
  │     personaje_viendo_tele.png
  │     cereal.png
  │     pacman1.png
  │     pacman2.png
  │     pacman3.png
  │     pasillo.png
  │     intento1.png
  │     intento2.png
  │     intento3.png
  │     gato.png
  │     sujetar_gato.png
  │     cables_png.png
  │     Mensaje1.png
  │     Mensaje2.png
  │     carta.png
  │     senora.png
  │     edificio1.png  →  edificio5.png
  │     foco_idea.png
  │     newton.png
  │     arbol.png
  │     manzana.png
  │     per1_dialogo1.png  →  per1_dialogo3.png
  │     per1_dialogoO.png
  │     per2_dialogo1.png  →  per2_dialogo3.png
  │     per2_dialogoO.png
  │     per1_vuelo.png
  │     per2_vuelo.png
  │
  └── audio/            ← archivos MP3 / WAV
        amanecer_cancion.mp3      pajaros_fondo.mp3
        alarma.mp3                golpe_despertador.mp3
        bostezo.mp3               lanza_cobija.mp3
        mama_dialogo.mp3          PDpersonaje.mp3
        tv_noticia.mp3            dialogo_tele.mp3
        dialogo_pacman.mp3        estatica.mp3
        cereal_caja.mp3           waka.mp3
        pacman_muerte.mp3         regano_comida.mp3
        musica_fondo2.mp3         viento_incomodo.mp3
        DAtorado.mp3              meow.mp3
        lanzar_gato.mp3           golpe_vidrio.mp3
        intento1.mp3              intento2.mp3
        intento3.mp3              abre_puerta.mp3
        recuerdo.mp3              mensaje_envio.mp3
        feldspar_golpe.mp3        catapulta.mp3
        dialogo_mama_vuelo.mp3    idea.mp3
        dialogo_final1.mp3  →  dialogo_final6.mp3


EJECUCIÓN
────────────────────────────────────────────────────────────────────────────────

  Windows:
    Ejecuta desde Visual Studio (F5) o haz doble clic en el .exe generado.
    Los archivos fmod.dll y freeglut.dll deben estar en la misma carpeta
    que el .exe (usualmente x64/Debug/ o x64/Release/).

  Linux:
    ./SinGravedad
    (libfmod.so debe estar en la misma carpeta o en /usr/local/lib/)

  Controles:
    ESC  →  Cierra la animación

  La animación completa dura aproximadamente 3 minutos y 20 segundos.
  Se recomienda ejecutarla con audio habilitado.


PROBLEMAS COMUNES
────────────────────────────────────────────────────────────────────────────────

  "fmod.dll / libfmod.so no encontrado"
  → Coloca el archivo junto al ejecutable o en /usr/local/lib/ (Linux).
    En Linux ejecuta: sudo ldconfig

  "freeglut.dll no encontrado" (Windows)
  → Copia freeglut.dll a la carpeta donde está el .exe.

  Pantalla negra al iniciar
  → Verifica que la carpeta assets/ esté en el mismo directorio que el
    ejecutable. El motor imprime en consola los archivos que no pudo cargar.

  No hay sonido
  → Verifica que los archivos de audio estén en assets/audio/ y que
    la biblioteca de FMOD esté correctamente instalada.

  Error de compilación en Linux: "fmod.hpp not found"
  → Asegúrate de haber copiado fmod.hpp y fmod_common.h a la carpeta
    del proyecto antes de compilar.

  Animación lenta (Windows)
  → Compila en modo Release en lugar de Debug.


NOTAS
────────────────────────────────────────────────────────────────────────────────

  - El motor está en Engine.h / Engine.cpp.
  - El guión de la animación está en Main.cpp. Cada evento lleva
    comentario con el segundo exacto en que ocurre.
  - Los sprites deben ser PNG-32 con canal alfa. Imágenes con fondo negro
    no se verán correctamente.


================================================================================
  Proyecto académico — Uso educativo
  ITCJ — 2026
================================================================================
