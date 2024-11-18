#include "lexer/lexer.hpp"
#include <iostream>
#include <lexer.hpp>


int main() {
    std::string_view source = R"(
        int main() {
            int a = 3;
            int b = 2;
            int c = b * a + 4;
        }
    )";
    
    auto lexer = dark::Lexer<>(source);
    auto tokens = lexer.lex();

    for (auto t: tokens) {
        std::cout << dark::to_string(t.kind) << " > '" << t.text << "', (" << t.line << ", " << t.col << ")\n"; 
    }

    return 0;
}
