import ctypes
import glob
import os
import sys

GREEN = "\033[92m"
RESET = "\033[0m"
CYAN = "\033[96m"


def run_tests():
    lib_path = os.path.abspath("build/src/bindings/libgradesolver_api.so")

    if not os.path.exists(lib_path):
        print(f"Error: No se encuentra {lib_path}. Ejecuta 'make' primero.")
        sys.exit(1)

    try:
        solver = ctypes.CDLL(lib_path)
        solver.solve_process.argtypes = [ctypes.c_char_p]
        solver.solve_process.restype = None

        cases = sorted(glob.glob("tests/cases/*.json"))

        print(f"\n{CYAN}=== Ejecutando Tests con Python (ctypes) ==={RESET}")

        for case in cases:
            print(f"\nProcesando: {GREEN}{os.path.basename(case)}{RESET}")
            with open(case, "rb") as f:
                json_bytes = f.read()
                solver.solve_process(json_bytes)

    except Exception as e:
        print(f"Error fatal en Python runner: {e}")
        sys.exit(1)


if __name__ == "__main__":
    run_tests()
