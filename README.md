TP2 — Construcción del Núcleo de un Sistema Operativo

Compilación y ejecución

  El proyecto se compila dentro de la imagen de la cátedra (agodio/itba-so-multiarch:3.1). Los scripts asumen un contenedor llamado tp-so con el proyecto montado en /root.

  Setup (una sola vez):

    docker pull agodio/itba-so-multiarch:3.1
    docker run -d -v "$(pwd)":/root --name tp-so agodio/itba-so-multiarch:3.1 tail -f /dev/null

  Compilar:

      ./compile.sh            build completo (memory manager por defecto: NAIVE)
      ./compile.sh MM=OWN     con el memory manager propio (free list explícita)
      ./compile.sh MM=BUDDY   con el buddy system

  compile.sh arranca el contenedor y corre make adentro. El selector de memory manager es la variable MM (NAIVE, OWN o BUDDY), resuelta en tiempo de compilación vía -DMM_*.

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
  - Syscalls: crear/finalizar con pasaje de parámetros, getpid, listar procesos, matar, cambiar prioridad, bloquear/desbloquear, yield, wait de hijos.

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
  - loop y test_sync sin semáforos usan busy-waiting a propósito (lo pide el TP); el resto del sistema está libre de busy-waiting.
  - Ctrl+C solo afecta al foreground. Un proceso lanzado con & (o los workers de mvar) se terminan con kill <pid>.


  -- Sincronización --

  Decisiones de diseño
  - Semáforos con nombre: procesos no relacionados los comparten acordando un ID (string) a priori (sem_open, sem_init, sem_close, sem_wait, sem_post).
  - Sin busy-waiting: al no haber recurso, el proceso se bloquea y se encola en una cola FIFO de waiters; sem_post despierta a uno.
  - Atomicidad garantizada con XCHG (TSL) más deshabilitar interrupciones (enter_region / leave_region) protegiendo la tabla de semáforos. Diseño libre de deadlock y race conditions.

  Ejemplos
      testsync 5 1            test_sync CON semáforos: resultado final 0 (determinista)
      testsync 5 0            test_sync SIN semáforos: varía por race conditions
      mvar 2 2               MVar: 2 escritores, 2 lectores (ver sección IPC)

  Consideraciones
  - sem_init sobre un semáforo existente lo re-inicializa (valor y cola de waiters): permite reutilizar un nombre fijo entre corridas aunque la anterior haya dejado el semáforo sucio (procesos matados a mitad de ciclo).
  - Limitación: matar un proceso no libera automáticamente los semáforos que tenía abiertos ni lo saca de las colas de waiters (no hay tracking por proceso). Por eso mvar resetea sus semáforos al arrancar.


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
      echo hola mundo | red       red imprime stdin en rojo (extra)
      cat | wc                    interactivo: escribir, Ctrl+D para EOF

  MVar (mvar <escritores> <lectores>): problema de múltiples lectores/escritores sobre una variable global, sincronizado con dos semáforos (empty/full). Cada lector imprime con un color identificador (estilo por proceso). El proceso principal termina apenas crea a los workers.

      mvar 2 2                ABABAB...  (2 colores de lector)
      mvar 3 2                ABCABC...
      mvar 2 2 ; kill <pid_B> al matar al escritor B la salida pasa a sólo A

  Consideraciones
  - La salida coloreada respeta la redirección: con |, las letras viajan por el pipe (el color sólo se aplica si el destino es la terminal, al estilo isatty()).
  - Los workers de mvar quedan en background (el principal sale enseguida): se frenan con kill <pid>, no con Ctrl+C.


  -- Drivers (teclado y video) --

  Decisiones de diseño
  - Driver de video en modo gráfico: la terminal mantiene un text_buffer (carácter + byte de estilo) que se renderiza al framebuffer, con scroll automático.
  - Driver de teclado con line discipline (modo cooked con echo / modo raw para juegos), manejo de Shift/Ctrl, y detección de Ctrl+C (SIGINT) y Ctrl+D (EOF).
  - Toda la salida a la terminal se serializa con un mutex (tty_out) para que procesos concurrentes no se entremezclen ni se pisen el cursor/estilo.
  - El color es estado por proceso (selectStyle): cada proceso tiene su estilo en el PCB, así dos procesos concurrentes no se corrompen el color.

  Ejemplos
      resize 32               cambia el tamaño de fuente
      clear                   limpia la pantalla


  -- Aplicaciones de User space (Shell) --

  Decisiones de diseño
  - sh: intérprete que lanza las aplicaciones; soporta background (&), pipe de 2 procesos (|), Ctrl+C (mata foreground) y Ctrl+D (EOF).
  - help lista todos los comandos e incluye el apartado de tests de la cátedra.
  - Comandos pedidos: mem, ps, loop, kill, nice, block, cat, wc, filter, mvar. Extra: echo, time, registers, show fps, speed, resize, red, tron, bounce, benchmarks (benchfloat/hw/mem/kbd) y los disparadores de excepciones (dividezero, invalidop).

  Ejemplos
      help                    listado de comandos + tests
      sh                      abre una shell anidada
      loop &                  proceso en background
      echo hola | wc          pipe entre dos procesos
      testmm 100000 &         test de la cátedra en background

  Consideraciones
  - Mapeo de nombres del enunciado al comando real: test_mm es testmm, test_proc es testprocesses, test_sync es testsync, test_prio es testprio.
  - No se soportan cadenas de 3+ pipes (p1 | p2 | p3), tal como permite el enunciado.


  Limitaciones y requerimientos parciales
  - Ctrl+C mata únicamente al proceso en foreground; no existe el concepto de grupo de procesos, por lo que los procesos en background se terminan con kill.
  - Matar un proceso no libera sus semáforos ni lo quita de las colas de waiters (mitigado en mvar reseteando los semáforos al arrancar).
  - El cambio de contexto no preserva el estado de la FPU/x87: el código de usuario evita punto flotante en caminos concurrentes (p. ej. el RNG es entero).
  - Pipes limitados a 2 procesos.


  Uso de IA

