BUILD_DIR = build
EXEC = $(BUILD_DIR)/src/cli/solver_cli
TEST_FILE = tests/input.json

.PHONY: all build configure clean run help

all: build

build: $(BUILD_DIR)/Makefile
	@echo "Compilando proyecto..."
	@cmake --build $(BUILD_DIR) -- -j$(shell nproc)

$(BUILD_DIR)/Makefile:
	@echo "Configurando CMake..."
	@mkdir -p $(BUILD_DIR)
	@cmake -S . -B $(BUILD_DIR) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
	@ln -sf $(BUILD_DIR)/compile_commands.json .

configure:
	@rm -rf $(BUILD_DIR)/CMakeCache.txt
	$(MAKE) $(BUILD_DIR)/Makefile

run: build
	@echo "Ejecutando Solver..."
	@./$(EXEC) $(TEST_FILE)

clean:
	@echo "Limpiando build y artefactos..."
	@rm -rf $(BUILD_DIR)
	@rm -f compile_commands.json

help:
	@echo "Opciones disponibles:"
	@echo "  make        - Compila el proyecto (incremental)"
	@echo "  make run    - Compila y ejecuta con $(TEST_FILE)"
	@echo "  make clean  - Borra la carpeta build"
	@echo "  make configure - Fuerza la reconfiguración de CMake"
