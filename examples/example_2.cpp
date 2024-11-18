#include <iostream>
#include <lexer.hpp>

enum class TokenKind {
    EscapeSequence,
    Colon,
    SemiColon,
    EndCharacter,
    Number,
    Unknown,
    Eof
};

struct LexerConfig {
    using kind_t = TokenKind;

    static constexpr auto punctuations = dark::detail::Switch<
        dark::detail::Case(TokenKind::EscapeSequence, "\\x1b["),
        dark::detail::Case(TokenKind::EscapeSequence, "27["),
        dark::detail::Case(TokenKind::EscapeSequence, "\\033["),
        dark::detail::Case(TokenKind::Colon, ":"),
        dark::detail::Case(TokenKind::SemiColon, ";"),
        dark::detail::Case(TokenKind::EndCharacter, "m")
    >{};

    static constexpr auto is_digit(std::string_view s) noexcept -> bool {
        return dark::DefaultLexerConfig::is_digit(s);
    }
    static constexpr auto parse_number(std::string_view s) noexcept -> std::string_view {
        auto c = 0zu;
        while (c < s.size() && is_digit(s.substr(c))) ++c;

        return s.substr(0, c);
    }
}; 

constexpr std::string_view to_string(TokenKind kind) noexcept {
    switch (kind) {
        case TokenKind::EscapeSequence: return "EscapeSequence";
        case TokenKind::Colon: return "Colon";
        case TokenKind::SemiColon: return "SemiColon";
        case TokenKind::EndCharacter: return "EndCharacter";
        case TokenKind::Number: return "Number";
        case TokenKind::Unknown: return "Unknown";
        case TokenKind::Eof: return "Eof";
        }
}


int main() {
    using dark::to_string;
    std::string_view source = R"(\x1b[1;31m)";
    
    auto lexer = dark::Lexer<LexerConfig>(source);
    auto tokens = lexer.lex();

    for (auto t: tokens) {
        std::cout << to_string(t.kind) << " > '" << t.text << "', (" << t.line << ", " << t.col << ")\n"; 
    }
    // Output: 
    // EscapeSequence > '\x1b[', (0, 0)
    // Number > '1', (0, 5)
    // SemiColon > ';', (0, 6)
    // Number > '31', (0, 7)
    // EndCharacter > 'm', (0, 9)
    // Eof > '', (0, 10)

    return 0;
}
