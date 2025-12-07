# GradeSolver

**GradeSolver** es un motor de cálculo de calificaciones de alto rendimiento escrito en **C++20**.
Permite proyectar notas faltantes basándose en reglas complejas (promedios ponderados, notas mínimas por etiqueta, cuellos de botella) y estrategias de resolución (distribución uniforme o enfoque en la evaluación de mayor peso).

El proyecto incluye:
1.  **Core Library:** Lógica de negocio pura.
2.  **CLI:** Interfaz de línea de comandos con visualización de tablas ASCII y colores.
3.  **Bindings:** Librería compartida (`.so`) exportable a Python, Node.js, etc.

---

## Requisitos Previos

### Para Compilar y Ejecutar el CLI (C++)
Necesitas un entorno de desarrollo C++ moderno:

* **Compilador C++20:** GCC 10+ o Clang 10+.
* **CMake:** Versión 3.15 o superior.
* **Make:** Para utilizar el wrapper de automatización.

> **Nota:** La dependencia `nlohmann/json` se descarga y configura automáticamente mediante CMake (`FetchContent`), no necesitas instalarla manualmente.

### Para Correr los Tests de Integración (Bindings)
Si deseas verificar que la librería funciona desde otros lenguajes:

* **Python 3:** (Viene preinstalado en casi todas las distros Linux).
* **Node.js & npm:** Requerido para probar el binding de Node. (node>=22, pnpm>=10)
* **Dependencias de Node:** Librerías `ffi-napi` y `ref-napi` (se instalan localmente).

---

## Compilación

El proyecto incluye un `Makefile` para simplificar el flujo de trabajo.

1.  **Compilar todo (Lib, CLI y Bindings):**
    ```bash
    make
    ```
    *Esto generará la carpeta `build/`, configurará CMake y compilará los binarios.*

2.  **Limpiar la compilación (Borrar build):**
    ```bash
    make clean
    ```

---

## Uso del CLI

Una vez compilado, puedes usar el ejecutable directamente con un archivo JSON de configuración.

**Comando:**
```bash
./build/src/cli/solver_cli <archivo_entrada.json>
