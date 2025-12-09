#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include "solver.hpp"

#define RST  "\033[0m"
#define RED  "\033[31m"
#define GRN  "\033[32m"
#define YEL  "\033[33m"
#define MAG  "\033[35m"
#define CYN  "\033[36m"
#define BOLD "\033[1m"
#define DIM  "\033[2m"

size_t get_visible_length(const std::string& s) {
    size_t len = 0;
    bool in_escape = false;
    for (char c : s) {
        if (c == '\033') { in_escape = true; }
        if (!in_escape) { len++; }
        if (in_escape && c == 'm') { in_escape = false; }
    }
    return len;
}

void print_cell(const std::string& content, int width, bool align_right = false) {
    size_t vis_len = get_visible_length(content);
    int padding = std::max(0, width - (int)vis_len);

    if (align_right) {
        for(int i=0; i<padding; ++i) std::cout << " ";
        std::cout << content;
    } else {
        std::cout << content;
        for(int i=0; i<padding; ++i) std::cout << " ";
    }
}

void print_separator(int width) {
    std::cout << "  +" << std::string(width - 2, '-') << "+" << std::endl;
}

void print_header(std::string c1, std::string c2, std::string c3, int w1, int w2, int w3) {
    std::cout << "  | ";
    print_cell(BOLD + c1 + RST, w1);
    std::cout << " | ";
    print_cell(BOLD + c2 + RST, w2, true);
    std::cout << " | ";
    print_cell(BOLD + c3 + RST, w3, true);
    std::cout << " |" << std::endl;
}

void print_formatted_output(const GradeSolver::CourseConfig& config, const GradeSolver::CalculationResult& result) {
    std::cout << std::endl;
    std::cout << "  " << BOLD << CYN << "ASIGNATURA: " << config.name << RST << std::endl;

    std::cout << "  " << DIM << "Reglas Activas:" << RST << std::endl;
    for(const auto& r : config.rules) {
        std::cout << "   - " << r.description << " (Meta: " << r.target << ")" << std::endl;
    }
    std::cout << std::endl;

    int w_name = 35;
    int w_weight = 8;
    int w_grade = 12;
    int total_width = w_name + w_weight + w_grade + 10;

    print_separator(total_width);
    print_header("Evaluacion [Tags]", "Peso", "Nota", w_name, w_weight, w_grade);
    print_separator(total_width);

    for (const auto& a : config.assessments) {
        std::string name_display = a.name;
        if (!a.tags.empty()) {
            name_display += " " + std::string(DIM) + "[";
            for(size_t i=0; i<a.tags.size(); i++) {
                name_display += a.tags[i] + (i < a.tags.size()-1 ? "," : "");
            }
            name_display += "]" + std::string(RST);
        }

        std::string weight_str = std::to_string(a.weight).substr(0, 4);

        std::string val_str;
        if (a.grade.has_value()) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << a.grade.value();
            val_str = ss.str();
        } else {
            double projected = 0.0;
            if (result.proposed_grades.count(a.name)) projected = result.proposed_grades.at(a.name);

            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << projected;

            std::string color = GRN;
            if (projected > 100) color = RED;
            else if (projected > 60) color = YEL;
            else if (projected == 0) color = DIM;

            val_str = color + ss.str() + " (Est)" + RST;
        }

        std::cout << "  | ";
        print_cell(name_display, w_name);
        std::cout << " | ";
        print_cell(weight_str, w_weight, true);
        std::cout << " | ";
        print_cell(val_str, w_grade, true);
        std::cout << " |" << std::endl;
    }

    print_separator(total_width);

    // Mostrar score global calculado desde reglas
    for (const auto& rule_status : result.rule_statuses) {
        if (rule_status.type == GradeSolver::RuleType::GLOBAL_AVERAGE) {
            std::cout << "  | ";
            print_cell("SCORE GLOBAL", w_name);
            std::cout << " | ";
            print_cell(" ", w_weight);
            std::cout << " | ";

            std::stringstream ss_glob;
            ss_glob << std::fixed << std::setprecision(1) << rule_status.current_score;
            print_cell(ss_glob.str(), w_grade, true);
            std::cout << " |" << std::endl;
            break;
        }
    }

    print_separator(total_width);

    // Mostrar estado de las reglas
    std::cout << std::endl;
    std::cout << "  " << BOLD << "ESTADO DE REGLAS:" << RST << std::endl;
    for (const auto& rule_status : result.rule_statuses) {
        std::string rule_name;
        if (rule_status.type == GradeSolver::RuleType::GLOBAL_AVERAGE) {
            rule_name = "Promedio Global";
        } else if (rule_status.type == GradeSolver::RuleType::TAG_AVERAGE) {
            rule_name = "Promedio [" + rule_status.tag_filter.value_or("") + "]";
        } else if (rule_status.type == GradeSolver::RuleType::MIN_GRADE_PER_TAG) {
            rule_name = "Mínimo [" + rule_status.tag_filter.value_or("") + "]";
        }

        std::string status_color = rule_status.status == "impossible" ? RED : 
                                 (rule_status.status == "guaranteed" ? GRN : YEL);

        std::cout << "   " << rule_name << ": " << BOLD << status_color << rule_status.status << RST;
        
        if (rule_status.type != GradeSolver::RuleType::MIN_GRADE_PER_TAG) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(1) << rule_status.current_score;
            std::cout << " (" << ss.str() << "/" << rule_status.target << ")";
        } else {
            std::cout << " (Min: " << rule_status.target << ")";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
    std::string status_color = result.status == "impossible" ? RED : (result.status == "guaranteed" ? GRN : YEL);

    std::cout << "  RESULTADO: " << BOLD << status_color << result.status << RST << std::endl;
    std::cout << "  " << result.message << std::endl;
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) return 1;

    bool raw_output = false;
    std::string filename;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--raw") {
            raw_output = true;
        } else {
            filename = arg;
        }
    }

    if (filename.empty()) {
        std::cerr << RED << "Error: No se especificó archivo." << RST << std::endl;
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << RED << "Error abriendo archivo." << RST << std::endl;
        return 1;
    }

    nlohmann::json j;
    file >> j;
    auto config = j.get<GradeSolver::CourseConfig>();

    GradeSolver::Calculator calc;
    auto result = calc.calculate(config);

    if (raw_output) {
        // Convertir CalculationResult a JSON usando la serialización automática
        nlohmann::json output = result;
        std::cout << output.dump() << std::endl;
    } else {
        print_formatted_output(config, result);
    }

    return 0;
}
