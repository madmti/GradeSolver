const ffi = require("ffi-napi");
const path = require("path");
const fs = require("fs");

const MAGENTA = "\x1b[35m";
const RESET = "\x1b[0m";
const YELLOW = "\x1b[33m";

try {
    const libPath = path.resolve(
        __dirname,
        "../build/src/bindings/libgradesolver_api.so",
    );

    const solver = ffi.Library(libPath, {
        solve_process: ["void", ["string"]],
    });

    console.log(
        `\n${MAGENTA}=== Ejecutando Tests con Node.js (ffi-napi) ===${RESET}`,
    );

    const casesDir = path.join(__dirname, "cases");
    const files = fs
        .readdirSync(casesDir)
        .filter((f) => f.endsWith(".json"))
        .sort();

    files.forEach((file) => {
        console.log(`\nProcesando: ${YELLOW}${file}${RESET}`);
        const content = fs.readFileSync(path.join(casesDir, file), "utf8");

        solver.solve_process(content);
    });
} catch (e) {
    console.error("Error en Node runner:", e.message);
    if (e.message.includes("cannot open shared object")) {
        console.error("Consejo: Asegúrate de haber hecho 'make' antes.");
    }
    process.exit(1);
}
