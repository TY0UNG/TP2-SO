TP2 — Construcción del Núcleo de un Sistema Operativo

Grupo 2. Integrantes:
  - Tobias Young
  - Antonio Valentuzzi
  - Facundo López Llopis

Compilación y ejecución

  El proyecto se compila dentro de la imagen de la cátedra (agodio/itba-so-multiarch:3.1). Los scripts asumen un contenedor llamado tp-so con el proyecto montado en /root.

  Setup (una sola vez, en el proyecto):

    docker pull agodio/itba-so-multiarch:3.1
    docker run -d -v "$(pwd)":/root --name tp-so agodio/itba-so-multiarch:3.1 tail -f /dev/null

  Compilar:

      ./compile.sh            build completo (memory manager por defecto: NAIVE)
      ./compile.sh MM=OWN     con el memory manager propio (free list explícita)
      ./compile.sh MM=BUDDY   con el buddy system

  compile.sh arranca el contenedor y corre make adentro. El selector de memory manager es la variable MM (NAIVE, OWN o BUDDY), resuelta en tiempo de compilación vía -DMM_*.

  Modificamos los make para que todo el output (.o, .map, etc) se guarden en /build, en las carpetas respectivas tanto para userland como para kernel.

  Ejecutar:

      sudo ./run.sh           levanta QEMU con la imagen generada

  run.sh intenta varios backends de audio (CoreAudio / SDL+Pulse / Pulse) y cae a ejecución sin audio si ninguno está disponible (hecho en el TP de Arqui).

Caracteres especiales y atajos de teclado en la shell
    |            Conecta dos comandos por un pipe: p1 | p2
    &            Ejecuta el comando en background (la shell no cede el foreground)
    Ctrl + C     Mata al proceso en foreground
    Ctrl + D     Envía EOF al proceso en foreground

Requerimientos — decisiones de diseño, ejemplos y consideraciones

  -- Memory Manager --

    Decisiones de diseño
      - Tres administradores intercambiables tras una interfaz común (memory.h)
      - La selección es en tiempo de compilación con MM=NAIVE, OWN o BUDDY (mediante #ifdef). Nunca conviven dos managers; el resto del kernel y los procesos usan la misma API.

    Ejemplos
        mem                     estado de la memoria (total y usada)
        testmm 1000000          stress test de malloc/free (foreground)
        testmm 1000000 &        mismo test en background

  -- Procesos, Context Switching y Scheduling --

    Decisiones de diseño
      - Multitasking preemptivo disparado por el timer tick; el scheduler es Round Robin con prioridades (5 niveles, 0 = más alta .. 4 = más baja), con una priority-list por nivel.
      - Cada comando de la shell se ejecuta como un proceso aparte (no built-in): la shell crea el proceso, le pasa argv/argc y opcionalmente le cede el foreground.
      - Los procesos tienen un flag killable (la shell se marca false para que Ctrl+C cancele la línea pero no mate la shell, como en bash).
      - Para evitar inanicion por baja prioridad, lo que hicimos fue poner todos los procesos en un unico round robin, pero la mayor prioridad determina mayores quantums a usar.

    Ejemplos
        ps                      lista procesos: nombre, PID, prioridad, RSP/RBP, foreground, estado
        loop                    imprime su PID cada ~1s (espera activa, no bloqueante)
        loop 500 &              en background, cada 500ms
        nice <pid> 0            sube la prioridad del proceso a la más alta
        block <pid>             alterna bloqueado/listo
        kill <pid>              mata el proceso
        testprocesses 10        test_proc: crea/bloquea/desbloquea/mata procesos dummy
        testprio 1000000        test_prio: mismas vs distintas prioridades

    Consideraciones
      - Ctrl+C solo afecta al foreground. Un proceso lanzado con & (o los workers de mvar) se terminan con kill <pid>.

  -- Sincronización --

    Decisiones de diseño
      - Semáforos con nombre: procesos no relacionados los comparten acordando un ID (string) a priori.
      - XCHG como primitiva de sincro (enter_region / leave_region) protegiendo la tabla de semáforos.

    Ejemplos
        testsync 5 1            test_sync CON semáforos: resultado final 0 (determinista)
        testsync 5 0            test_sync SIN semáforos: varía por race conditions
        mvar 2 2               MVar: 2 escritores, 2 lectores (ver sección IPC)

    Consideraciones
      - sem_init sobre un semáforo existente lo re-inicializa (valor y cola de waiters): permite reutilizar un nombre fijo entre corridas aunque la anterior haya dejado el semáforo sucio (procesos matados a mitad de ciclo).
      - Los prints de mvar no se ven exactamente como el enunciado, debido a las decisiones de diseño que tomamos en el scheduling. Por ejemplo, al aumentar prioridades, se da mayor quantum pero sigue siendo round robin global, por lo que el resultado no cambia.

    Limitaciones
      - Matar un proceso no libera automáticamente los semáforos que tenía abiertos ni lo saca de las colas de waiters (no hay tracking por proceso). Por eso init reinicializa.

  -- Inter Process Communication (Pipes) --

    Decisiones de diseño
      - Pipes unidireccionales con lectura/escritura bloqueantes.
      - Transparencia terminal/pipe vía una abstracción tipo VFS: cada descriptor es un file_t con ops->read / ops->write. Un proceso escribe/lee igual sin importar si detrás hay una terminal o un pipe; su código no cambia.
      - Por defecto fds[0/1/2] (stdin/stdout/stderr) apuntan a la terminal. La shell, al ver |, reemplaza el fds[1] del comando izquierdo y el fds[0] del derecho por los extremos de un pipe.
      - Pipes con nombre/ID compartibles entre procesos no relacionados.

    Ejemplos
        echo hola mundo | cat       cat reimprime stdin
        echo hola mundo | wc        wc cuenta líneas
        echo hola mundo | filter    filter quita las vocales
        echo hola mundo | red       red imprime stdin en rojo
        cat | wc                    interactivo: escribir, Ctrl+D para EOF

  -- Drivers (teclado y video) --

    Decisiones de diseño
      - Hicimos una clara separacion de lo que es driver de video, terminal y shell.
        - Driver de video: Está en kernel y tiene las funciones para dibujar en la pantalla (Figuras, texto en posiciones absolutas).
        - Terminal: Está en kernel y maneja el buffer de texto, dibuja el texto con el driver de video, maneja el proceso en foreground.
        - Shell: Está en userland, es un proceso, contiene la logica de parseo de comandos, dispatch de procesos.
      - Debido a la ausencia de kernel threads, el driver de teclado tiene el line discipline (modo con echo / modo raw para juegos), manejo de Shift/Ctrl, y detección de Ctrl+C (KILL) y Ctrl+D (EOF). Esto es debido a que la terminal no puede consumir el driver de teclado periodicamente pues no es un proceso ni tiene kernel threads.
      - Toda la salida a la terminal se serializa con un mutex (tty_out) para que procesos concurrentes no se entremezclen ni se pisen el cursor/estilo.
      - El color es estado por proceso (selectStyle): cada proceso tiene su estilo en el PCB, así dos procesos concurrentes no se corrompen el color.

    Ejemplos
        resize 32               cambia el tamaño de fuente
        clear                   limpia la pantalla
        tron                    juego para probar el driver de video

  -- Aplicaciones de User space (Shell) --

    Decisiones de diseño
      - sh: intérprete que lanza las aplicaciones; soporta background (&), pipe de 2 procesos (|), Ctrl+C (mata foreground) y Ctrl+D (EOF).
      - help lista todos los comandos e incluye el apartado de tests de la cátedra.
      - mem, ps, loop, kill, nice, block, cat, wc, filter, mvar. Extra/Viejos: echo, time, registers, show fps, speed, resize, red, tron, bounce, benchmarks (benchfloat/hw/mem/kbd) y los disparadores de excepciones (dividezero, invalidop).

    Ejemplos
        help                    listado de comandos + tests
        sh                      abre una shell anidada
        loop &                  proceso en background
        echo hola | wc          pipe entre dos procesos
        testmm 100000 &         test de la cátedra en background

    Limitaciones y requerimientos parciales
      - Ctrl+C mata únicamente al proceso en foreground; no existe el concepto de grupo de procesos, por lo que los procesos en background se terminan con kill.
      - No se soportan cadenas de 2+ pipes (p1 | p2 | p3).

  Uso de IA
    Utilizamos IA para el debug de algunos problemas y modificaciones de algunas features. Se usó principalmente Claude.