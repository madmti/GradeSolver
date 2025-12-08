#include "solver.hpp"
#include <iostream>
#include <string>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* solve_process(const char* input_json_raw) {
        static std::string output_buffer;

        output_buffer.clear();

        try {
            if (input_json_raw == nullptr) {
                return "{}";
            }

            std::string input_json(input_json_raw);
            auto j_in = nlohmann::json::parse(input_json);

            auto config = j_in.get<GradeSolver::CourseConfig>();

            GradeSolver::Calculator calc;
            auto result = calc.calculate(config);

            nlohmann::json j_out = result;

            output_buffer = j_out.dump();

        } catch (const std::exception& e) {
            nlohmann::json err;
            err["status"] = "error";
            err["message"] = e.what();

            output_buffer = err.dump();

            std::cerr << "[WASM Error] " << e.what() << std::endl;
        }

        return output_buffer.c_str();
    }
}
