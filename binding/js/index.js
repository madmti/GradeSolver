"use strict";

const path = require("path");
const createSolverModule = require("./solver.js");

const hasDirname = typeof __dirname !== "undefined";
const locateFile = hasDirname
  ? (file) => (file.endsWith(".wasm") ? path.join(__dirname, "solver.wasm") : file)
  : null;

/**
 * Ejecuta el solver con un objeto de entrada o un JSON string.
 * @param {object|string} input
 * @returns {Promise<object>}
 */
async function solve(input) {
  const moduleInstance = await createSolverModule(
    locateFile ? { locateFile } : undefined
  );
  const inputJson = typeof input === "string" ? input : JSON.stringify(input);
  const outputJson = moduleInstance.ccall(
    "solve_process",
    "string",
    ["string"],
    [inputJson]
  );
  return JSON.parse(outputJson);
}

module.exports = solve;
module.exports.solve = solve;
module.exports.createSolverModule = createSolverModule;
module.exports.default = solve;
