#ifndef DARK_LEXER_LEXER_HPP
#define DARK_LEXER_LEXER_HPP

#include "lexer/switch.hpp"
#include <concepts>
#include <string_view>
#include <type_traits>
#include <utility>
namespace dark {

    enum class DefaultTokenKind {
        // Punctuations
        Comma,
        Colon,
        Dot,
        OpenParen,
        CloseParen,
        OpenSquare,
        CloseSqaure,
        OpenCurly,
        CloseCurly,
        Equal,

        // Operator
        Plus,
        Minus,
        ForwardSlash,
        Star,
        LessThanLessThan,
        LessTham,
        GreaterThan,  
        GreaterThanGreaterThan,  
        ThinArrow,
        Tilde,
        And,
        AndAnd,
        Or,
        OrOr,
        Caret,
        Percentage,
        ExclamationMark,
        EqualEqual,
        Not,
        NotEqual,
        QuestionMark,

        // Keyword
        

        Identifier,
        Number,

        
        // Whitespace
        Whitespace,

        Unknown,
        Eof
    };

    struct DefaultLexerConfig {
        using kind_t = DefaultTokenKind;

        static constexpr auto punctuations = detail::Switch<
            detail::Case(DefaultTokenKind::Comma, ","), 
            detail::Case(DefaultTokenKind::Colon, ";"), 
            detail::Case(DefaultTokenKind::Dot, "."), 
            detail::Case(DefaultTokenKind::OpenParen, "("),
            detail::Case(DefaultTokenKind::CloseParen, ")"),
            detail::Case(DefaultTokenKind::OpenSquare, "["),
            detail::Case(DefaultTokenKind::CloseSqaure, "]"),
            detail::Case(DefaultTokenKind::OpenCurly, "{"),
            detail::Case(DefaultTokenKind::CloseCurly, "}"),
            detail::Case(DefaultTokenKind::Equal, "=")>{};

        static constexpr auto operators = detail::Switch<
            detail::Case(DefaultTokenKind::Plus, "+"),
            detail::Case(DefaultTokenKind::Minus, "-"),
            detail::Case(DefaultTokenKind::ForwardSlash, "/"),
            detail::Case(DefaultTokenKind::Star, "*"),
            detail::Case(DefaultTokenKind::GreaterThanGreaterThan, ">>"),
            detail::Case(DefaultTokenKind::GreaterThan, ">"),
            detail::Case(DefaultTokenKind::LessTham, "<"),
            detail::Case(DefaultTokenKind::LessThanLessThan, "<<"),
            detail::Case(DefaultTokenKind::ThinArrow, "->"),
            detail::Case(DefaultTokenKind::Tilde, "~"),
            detail::Case(DefaultTokenKind::And, "&"),
            detail::Case(DefaultTokenKind::AndAnd, "&&"),
            detail::Case(DefaultTokenKind::Or, "|"),
            detail::Case(DefaultTokenKind::OrOr, "||"),
            detail::Case(DefaultTokenKind::Caret, "^"),
            detail::Case(DefaultTokenKind::Percentage, "%"),
            detail::Case(DefaultTokenKind::ExclamationMark, "!"), 
            detail::Case(DefaultTokenKind::EqualEqual, "=="),
            detail::Case(DefaultTokenKind::NotEqual, "!="),
            detail::Case(DefaultTokenKind::QuestionMark, "?")>{};
        static constexpr auto whitespace = detail::Switch<
            detail::Case(DefaultTokenKind::Whitespace, " "),
            detail::Case(DefaultTokenKind::Whitespace, "\n"),
            detail::Case(DefaultTokenKind::Whitespace, "\r")
        >{};
        static constexpr auto is_valid_identifier_start(std::string_view s) noexcept -> bool {
            auto const f = s[0];
            return f == '_' || f == '$' || (f >= 'a' && f <= 'z') || (f >= 'A' && f <= 'Z');
        } 

        static constexpr auto is_valid_identifier(std::string_view s) noexcept -> bool {
            if (is_valid_identifier_start(s)) return true;
            return s[0] >= 0 && s[0] <= 9;
        }
        
        static constexpr auto is_digit(std::string_view s) noexcept -> bool {
            return s[0] >= '0' && s[0] <= '9';
        }

        static constexpr auto parse_number(std::string_view s) noexcept -> std::string_view {
            auto cursor = 0zu;
            while (cursor < s.size() && (is_digit(s.substr(cursor)) || s[cursor] == '.')) {
                ++cursor;
            }
            return s.substr(0, cursor);
        }
    };

    namespace detail {
        template <typename T>
        concept LexerConfig = requires {
            requires std::is_enum_v<typename T::kind_t>;
        };

        template <typename T>
        concept has_whitespace = requires {
            T::whitespace;
        };

        template <typename T>
        concept has_punctuations = requires {
            T::punctuations;
        };
        
        template <typename T>
        concept has_operators = requires {
            T::operators;
        };
        
        template <typename T>
        concept has_identifier = requires {
            { T::is_valid_identifier_start(std::declval<std::string_view>()) } -> std::same_as<bool>;
            { T::is_valid_identifier(std::declval<std::string_view>()) } -> std::same_as<bool>;
        };

        template <typename T>
        concept has_numbers = requires {
            { T::is_digit(std::declval<std::string_view>()) } -> std::same_as<bool>;
            { T::parse_number(std::declval<std::string_view>()) } -> std::same_as<std::string_view>;
        };

    } // namespace detail

    static_assert(detail::LexerConfig<DefaultLexerConfig>, "Lexer config not satisfied");

    template <typename Kind>
        requires std::is_enum_v<Kind>
    struct Token {
        Kind kind;
        std::string_view text;
        unsigned start;
        unsigned line;
        unsigned col;
    };

    template <detail::LexerConfig Config = DefaultLexerConfig>
    struct Lexer {
        constexpr Lexer(std::string_view source) noexcept
            : m_source(source)
        {}
        constexpr Lexer(Lexer const&) noexcept = default;
        constexpr Lexer& operator=(Lexer const&) noexcept = default;
        constexpr Lexer(Lexer &&) noexcept = default;
        constexpr Lexer& operator=(Lexer &&) noexcept = default;
        constexpr ~Lexer() noexcept = default;
    

        auto lex() -> std::vector<Token<typename Config::kind_t>> {
            using kind_t = typename Config::kind_t;
            std::vector<Token<kind_t>> tokens{};
            auto line = 0u;
            auto line_start_pos = 0u;

            while (m_cursor < m_source.size()) {
                auto source = m_source.substr(m_cursor);
                if (source[0] == '\n') {
                    ++line;
                    line_start_pos = m_cursor;
                }    
                if constexpr (detail::has_whitespace<Config>) {
                    auto id = Config::whitespace.match(source);
                    if (id != Config::whitespace.npos) {
                        auto text = Config::whitespace.str_from_index(id);
                        auto kind = Config::whitespace.token_from_index(id);
                        auto size = static_cast<unsigned>(text.size());
                        tokens.push_back({
                            .kind = kind,
                            .text = text,
                            .start = m_cursor,
                            .line = line,
                            .col = m_cursor - line_start_pos
                        });
                        m_cursor += size;
                        continue;
                    }
                }
                if constexpr (detail::has_punctuations<Config>) {
                    auto id = Config::punctuations.match(source);
                    if (id != Config::punctuations.npos) {
                        auto text = Config::punctuations.str_from_index(id);
                        auto kind = Config::punctuations.token_from_index(id);
                        auto size = static_cast<unsigned>(text.size());
                        tokens.push_back({
                            .kind = kind,
                            .text = text,
                            .start = m_cursor,
                            .line = line,
                            .col = m_cursor - line_start_pos
                        });
                        m_cursor += size;
                        continue;
                    }
                }
               if constexpr (detail::has_operators<Config>) {
                    auto id = Config::operators.match(source);
                    if (id != Config::operators.npos) {
                        auto text = Config::operators.str_from_index(id);
                        auto kind = Config::operators.token_from_index(id);
                        auto size = static_cast<unsigned>(text.size());
                        tokens.push_back({
                            .kind = kind,
                            .text = text,
                            .start = m_cursor,
                            .line = line,
                            .col = m_cursor - line_start_pos
                        });
                        m_cursor += size;
                        continue;
                    }
                }
                if constexpr (detail::has_identifier<Config>) {
                    if (Config::is_valid_identifier_start(source)) {
                        auto end = 0zu;
                        while ((end < source.size()) && (Config::is_valid_identifier(source.substr(end)))) {
                            ++end;
                        }

                        auto text = source.substr(0, end);
                        tokens.push_back({
                            .kind = kind_t::Identifier,
                            .text = text,
                            .start = m_cursor,
                            .line = line,
                            .col = m_cursor - line_start_pos
                        });

                        m_cursor += static_cast<unsigned>(end);
                        continue;
                    }
                }
                if constexpr (detail::has_numbers<Config>) {
                    if (Config::is_digit(source)) {
                        auto text = Config::parse_number(source);
                        
                        tokens.push_back({
                            .kind = kind_t::Number,
                            .text = text,
                            .start = m_cursor,
                            .line = line,
                            .col = m_cursor - line_start_pos
                        });

                        m_cursor += static_cast<unsigned>(text.size());
                        continue;
                    } 
                }
                
                tokens.push_back({
                    .kind = kind_t::Unknown,
                    .text = source.substr(0, 1),
                    .start = m_cursor,
                    .line = line,
                    .col = m_cursor - line_start_pos
                });
                m_cursor += 1u;
                
            }

            tokens.push_back({
                .kind = kind_t::Eof,
                .text = "",
                .start = m_cursor,
                .line = line,
                .col = m_cursor - line_start_pos
            });

            return tokens;
        }

    private:
        unsigned m_cursor{0};
        std::string_view m_source;
    };

    constexpr std::string_view to_string(DefaultTokenKind kind) noexcept {
        switch (kind) {
            case DefaultTokenKind::Comma:  return "Comma"; 
            case DefaultTokenKind::Colon:  return "Colon"; 
            case DefaultTokenKind::Dot:  return "Dot"; 
            case DefaultTokenKind::OpenParen:  return "OpenParen"; 
            case DefaultTokenKind::CloseParen:  return "CloseParen"; 
            case DefaultTokenKind::OpenSquare:  return "OpenSquare"; 
            case DefaultTokenKind::CloseSqaure:  return "CloseSqaure"; 
            case DefaultTokenKind::OpenCurly:  return "OpenCurly"; 
            case DefaultTokenKind::CloseCurly:  return "CloseCurly"; 
            case DefaultTokenKind::Equal:  return "Equal"; 
            case DefaultTokenKind::Plus:  return "Plus"; 
            case DefaultTokenKind::Minus:  return "Minus"; 
            case DefaultTokenKind::ForwardSlash:  return "ForwardSlash"; 
            case DefaultTokenKind::Star:  return "Star"; 
            case DefaultTokenKind::LessThanLessThan:  return "LessThanLessThan"; 
            case DefaultTokenKind::LessTham:  return "LessTham"; 
            case DefaultTokenKind::GreaterThan:    return ":"; 
            case DefaultTokenKind::GreaterThanGreaterThan:    return ":"; 
            case DefaultTokenKind::ThinArrow:  return "ThinArrow"; 
            case DefaultTokenKind::Tilde:  return "Tilde"; 
            case DefaultTokenKind::And:  return "And"; 
            case DefaultTokenKind::AndAnd:  return "AndAnd"; 
            case DefaultTokenKind::Or:  return "Or"; 
            case DefaultTokenKind::OrOr:  return "OrOr"; 
            case DefaultTokenKind::Caret:  return "Caret"; 
            case DefaultTokenKind::Percentage:  return "Percentage"; 
            case DefaultTokenKind::ExclamationMark:  return "ExclamationMark"; 
            case DefaultTokenKind::EqualEqual:  return "EqualEqual"; 
            case DefaultTokenKind::Not:  return "Not"; 
            case DefaultTokenKind::NotEqual:  return "NotEqual"; 
            case DefaultTokenKind::QuestionMark:  return "QuestionMark"; 
            case DefaultTokenKind::Identifier:  return "Identifier"; 
            case DefaultTokenKind::Number:  return "Number"; 
            case DefaultTokenKind::Whitespace:  return "Whitespace"; 
            case DefaultTokenKind::Unknown:  return "Unknown"; 
            case DefaultTokenKind::Eof:  return "Eof"; 
        }
    }


} // namespace dark

#endif // DARK_LEXER_LEXER_HPP
