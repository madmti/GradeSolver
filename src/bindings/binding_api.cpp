#include "solver.hpp"
#include <iostream>

extern "C" {
    void solve_json_api(const char* input_json_str) {
        try {
            auto j = nlohmann::json::parse(input_json_str);
            auto config = j.get<GradeSolver::CourseConfig>();

            GradeSolver::Calculator calc;
            auto result = calc.calculate(config);

            nlohmann::json output_json = result;

            std::cout << output_json.dump() << std::endl;
        } catch (...) {
            std::cout << "{\"error\": \"Invalid Input\"}" << std::endl;
        }
    }
}
