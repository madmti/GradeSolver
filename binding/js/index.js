"use strict";

const createSolverModule = require("./solver.js");

/**
 * Ejecuta el solver con un objeto de entrada o un JSON string.
 * @param {object|string} input
 * @returns {Promise<object>}
 */
async function solve(input) {
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

module.exports = solve;
module.exports.solve = solve;
module.exports.createSolverModule = createSolverModule;
module.exports.default = solve;