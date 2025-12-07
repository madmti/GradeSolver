#include "solver.hpp"
#include <iostream>
#include <string>

extern "C" {
    void solve_process(const char* input_json_raw) {
        try {
            if (input_json_raw == nullptr) return;
            std::string input_json(input_json_raw);

            auto j_in = nlohmann::json::parse(input_json);

            auto config = j_in.get<GradeSolver::CourseConfig>();

            GradeSolver::Calculator calc;
            auto result = calc.calculate(config);

            nlohmann::json j_out = result;
            std::cout << j_out.dump() << std::endl;

        } catch (const std::exception& e) {
            nlohmann::json err;
            err["status"] = "error";
            err["message"] = e.what();
            std::cout << err.dump() << std::endl;
        }
    }
}
