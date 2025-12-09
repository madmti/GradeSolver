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
```

Se puede usar la opcion `--raw` para obtener una salida en JSON sin formato.
```bash
./build/src/cli/solver_cli <archivo_entrada.json> --raw
```

### Salida del CLI

El CLI puede mostrar la información de dos formas:

#### 1. Salida Formateada (por defecto)
Muestra una tabla ASCII con colores que incluye:
- **Encabezado:** Nombre de la asignatura y reglas activas
- **Tabla de evaluaciones:** Con nombres, pesos, notas actuales y proyectadas
- **Score global:** Calculado a partir de las reglas
- **Estado de reglas:** Estado individual de cada regla configurada
- **Resultado final:** Estado general y regla limitante

**Ejemplo:**
```
  ASIGNATURA: Química Lab
  Reglas Activas:
   - Global (Meta: 55)
   - Min Lab (Meta: 40)

  +---------------------------------------------------------------+
  | Evaluacion [Tags]                   |     Peso |         Nota |
  +---------------------------------------------------------------+
  | Teoria                              |     0.60 |         70.0 |
  | Lab 1 [labs]                        |     0.20 |        100.0 |
  | Lab 2 [labs]                        |     0.20 |   40.0 (Est) |
  +---------------------------------------------------------------+
  | SCORE GLOBAL                        |          |         62.0 |
  +---------------------------------------------------------------+

  ESTADO DE REGLAS:
   Promedio Global: guaranteed (62.0/55)
   Mínimo [labs]: possible (Min: 40)

  RESULTADO: possible
  Limitante principal: Min Lab
```

#### 2. Salida Raw JSON (`--raw`)
Devuelve el objeto `CalculationResult` completo en formato JSON para consumo programático:

```json
{
  "strategy_used": "Uniforme Multi-Objetivo",
  "status": "possible",
  "message": "Limitante principal: Min Lab",
  "limiting_rule_description": "Min Lab",
  "rule_statuses": [
    {
      "type": "global_average",
      "current_score": 62.0,
      "target": 55.0,
      "status": "guaranteed",
      "tag_filter": null
    },
    {
      "type": "min_grade_per_tag",
      "current_score": 0.0,
      "target": 40.0,
      "status": "possible",
      "tag_filter": "labs"
    }
  ],
  "proposed_grades": {
    "Lab 2": 40.0
  }
}
```

**Estados posibles:**
- `"guaranteed"`: La regla ya se cumple con las notas actuales
- `"impossible"`: Imposible de cumplir incluso con notas perfectas
- `"possible"`: Puede cumplirse con las notas propuestas

> **Nota:** El input del ejemplo se encuentra en `tests/cases/03-rules-possible.json`.
