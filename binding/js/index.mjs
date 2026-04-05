import createSolverModuleFactory from "./solver.web.mjs";

const wasmUrl = new URL("./solver.web.wasm", import.meta.url).toString();

function defaultLocateFile(path) {
  if (path.endsWith(".wasm")) {
    return wasmUrl;
  }
  return new URL(path, import.meta.url).toString();
}

export async function createSolverModule(options = {}) {
  const locateFile = options.locateFile ?? defaultLocateFile;
  return createSolverModuleFactory({ ...options, locateFile });
}

/**
 * Ejecuta el solver con un objeto de entrada o un JSON string.
 * @param {object|string} input
 * @returns {Promise<object>}
 */
export async function solve(input) {
  const moduleInstance = await createSolverModule();
  const inputJson = typeof input === "string" ? input : JSON.stringify(input);
  const outputJson = moduleInstance.ccall(
    "solve_process",
    "string",
    ["string"],
    [inputJson]
  );
  return JSON.parse(outputJson);
}

export default solve;
