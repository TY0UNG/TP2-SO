// ---------------------------------------------------------------------------------------------------------------------
// Tron (Wireframe 3D Cube Renderer)
// Este código implementa un renderizador 3D de bajo nivel (solo wireframe) utilizando las primitivas de dibujo del kernel.
// Usa aproximaciones de la Serie de Taylor para seno y coseno para evitar errores de ABI/SSE.
// Ejecución en ESPACIO DE USUARIO (FRONTEND).
// ---------------------------------------------------------------------------------------------------------------------

#include <stdint.h>
#include <draw.h> // Incluye las primitivas de dibujo.
#include <stddef.h> // Para NULL

int tron(char ** argv, int argc) {
    enableGraphicsMode();
    drawFilledCircle(200, 200, 20, 0xFFFF00FF);
    return 0;
}
