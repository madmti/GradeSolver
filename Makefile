BUILD_DIR = build
EXEC = $(BUILD_DIR)/cli/solver_cli

.PHONY: all build run wasm test-wasm test-pack clean-wasm help

all: build

help:
	@echo "Comandos disponibles:"
	@echo "  make build      Compila el proyecto C++"
	@echo "  make run        Ejecuta el CLI"
	@echo "  make wasm       Compila el binding WASM"
	@echo "  make test-wasm  Ejecuta tests JS contra dist/js"
	@echo "  make test-pack  Ejecuta tests contra el paquete npm empaquetado"
	@echo "  make clean-wasm Limpia build_wasm y dist/js"
	@echo "  make release-js Publica el paquete en npm (usa dist/js)"

build: $(BUILD_DIR)/Makefile
	@echo "Compilando proyecto..."
	@cmake --build $(BUILD_DIR) -- -j$(shell nproc)

$(BUILD_DIR)/Makefile:
	@echo "Configurando CMake..."
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

run: build
	@echo "Ejecutando programa..."
	@$(EXEC)

wasm:
	@echo "Compilando binding WASM..."
	@bash scripts/build_wasm.sh

test-wasm:
	@echo "Ejecutando tests de JavaScript..."
	@cd tests/js && node test_runner.js

test-pack: wasm
	@echo "Ejecutando tests del paquete npm..."
	@bash scripts/test_pack.sh

clean-wasm:
	@echo "Limpiando archivos WASM..."
	@rm -rf build_wasm
	@rm -rf dist/js

release-js: wasm
	@echo "Recompilando para JavaScript..."
	@cd dist/js && npm publish
