#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DIST_DIR="$ROOT_DIR/dist/js"
BINDING_DIR="$ROOT_DIR/binding/js"
TESTS_DIR="$ROOT_DIR/tests"

echo "======================================"
echo "Probando paquete npm empaquetado"
echo "======================================"

if ! command -v npm >/dev/null 2>&1; then
  echo "Error: npm no está instalado o no está en el PATH"
  exit 1
fi

if ! command -v node >/dev/null 2>&1; then
  echo "Error: node no está instalado o no está en el PATH"
  exit 1
fi

if [ ! -f "$DIST_DIR/solver.js" ] || [ ! -f "$DIST_DIR/solver.wasm" ]; then
  echo "Error: no se encontraron artefactos en dist/js/"
  echo "Ejecuta primero: make wasm"
  exit 1
fi

if [ ! -f "$BINDING_DIR/package.json" ] || [ ! -f "$BINDING_DIR/solver.d.ts" ] || [ ! -f "$BINDING_DIR/index.js" ]; then
  echo "Error: falta binding/js/package.json, binding/js/solver.d.ts o binding/js/index.js"
  exit 1
fi

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "$TMP_DIR"' EXIT

PKG_DIR="$TMP_DIR/gradesolver-pkg"
APP_DIR="$TMP_DIR/test-app"

mkdir -p "$PKG_DIR"
cp "$BINDING_DIR/package.json" "$PKG_DIR/"
cp "$BINDING_DIR/solver.d.ts" "$PKG_DIR/"
cp "$BINDING_DIR/index.js" "$PKG_DIR/"
cp "$DIST_DIR/solver.js" "$PKG_DIR/"
cp "$DIST_DIR/solver.wasm" "$PKG_DIR/"

echo ""
echo "Empaquetando..."
PACK_JSON="$(cd "$PKG_DIR" && npm pack --json)"

TARBALL="$(node -e 'const fs=require("fs");const data=fs.readFileSync(0,"utf8");const json=JSON.parse(data);console.log(json[0].filename);' <<< "$PACK_JSON")"

echo "Tarball generado: $TARBALL"

mkdir -p "$APP_DIR"
cd "$APP_DIR"
npm init -y >/dev/null 2>&1
npm install "$PKG_DIR/$TARBALL" >/dev/null 2>&1

mkdir -p "$APP_DIR/tests"
cp -R "$TESTS_DIR/js" "$APP_DIR/tests/"
cp -R "$TESTS_DIR/cases" "$APP_DIR/tests/"

mkdir -p "$APP_DIR/dist/js"
cat > "$APP_DIR/dist/js/solver.js" <<'EOF'
module.exports = require("@madmti/gradesolver");
EOF

echo ""
echo "Ejecutando tests usando el paquete instalado..."
cd "$APP_DIR"
node tests/js/test_runner.js

echo ""
echo "======================================"
echo "Tests del paquete completados"
echo "======================================"
