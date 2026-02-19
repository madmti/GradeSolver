BUILD_DIR = build
EXEC = $(BUILD_DIR)/cli/solver_cli

.PHONY: all build run wasm test-wasm clean-wasm

all: build

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

clean-wasm:
	@echo "Limpiando archivos WASM..."
	@rm -rf build_wasm
	@rm -rf js
