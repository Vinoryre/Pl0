#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Lexer.h"
#include "Token.h"
#include "Parser.h"

int main(int argc, char* argv[]) {
    system("chcp 65001 > nul");

    // ================= 参数检查 =================
    if (argc != 2) {
        std::cout << "Usage: main.exe <source-file>\n";
        return 1;
    }

    std::string filename = argv[1];

    try {
        // ================= Lexer =================
        Lexer lexer(filename);
        std::vector<Token> tokens = lexer.tokenize();

        // ================= 输出Token流 =================
        for (const auto& t : tokens) {
            std::cout << "("
                      << t.line << " "
                      << lexer.tokenToString(t)
                      << ")\n";
        }

        // ================= Parser =================
        Parser parser(tokens);
        parser.parseProgram();

        // ================= 输出语法错误 =================
        const auto& errors = parser.getErrors();

        if (!errors.empty()) {
            std::cout << "\nSyntax Errors:\n";
            for (const auto& e : errors) {
                std::cout << "Line " << e.line << ": "
                          << e.message << "\n";
            }
        } else {
            std::cout << "\nSyntax OK\n";
        }

    } catch (const std::exception& e) {
        // 只保留：文件/词法错误
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}