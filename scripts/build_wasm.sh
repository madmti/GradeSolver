#!/usr/bin/env bash

set -e

echo "======================================"
echo "Building GradeSolver WASM Binding"
echo "======================================"

# Verificar que emscripten esté disponible
if ! command -v emcc &> /dev/null; then
    echo "Error: emscripten no está instalado o no está en el PATH"
    echo "Instala emscripten desde: https://emscripten.org/docs/getting_started/downloads.html"
    exit 1
fi

echo "Emscripten version:"
emcc --version

# Crear directorio de build para WASM si no existe
BUILD_DIR="build_wasm"
if [ -d "$BUILD_DIR" ]; then
    echo "Limpiando build anterior..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "Configurando con CMake..."
emcmake cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_STANDARD=20

echo ""
echo "Compilando..."
emmake make solver_wasm -j4

# Crear directorio js si no existe
cd ..
mkdir -p tests/js

# Copiar archivos generados a la carpeta js
echo ""
echo "Copiando archivos a js/..."
cp "$BUILD_DIR"/binding/solver.js tests/js/
cp "$BUILD_DIR"/binding/solver.wasm tests/js/

echo ""
echo "======================================"
echo "Build completado exitosamente!"
echo "======================================"
echo ""
echo "Archivos generados:"
echo "  - js/solver.js"
echo "  - js/solver.wasm"
echo ""
