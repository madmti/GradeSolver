#include <iostream>
#include "solver.hpp"

int main() {
    GradeSolver::Calculator calc;
    std::cout << "CLI Output: " << calc.calculate().dump(4) << std::endl;
    return 0;
}
