#include <algorithm>
#include <cctype>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

std::string symbols = "~!@#$%^&*()`-=_+[]\\{}|:\";\'<>,./?";
std::vector<std::string> keywords = {
    "alignas", "alignof", "and", "and_eq", "asm", "auto",
    "bitand", "bitor", "bool", "break", "case", "catch",
    "char", "char8_t", "char16_t", "char32_t", "class", "compl",
    "concept", "const", "consteval", "constexpr", "constinit", "const_cast",
    "continue", "co_await", "co_return", "co_yield",
    "decltype", "default", "delete", "do", "double", "dynamic_cast",
    "else", "enum", "explicit", "export", "extern", "false",
    "float", "for", "friend", "goto", "if", "inline",
    "int", "long", "mutable", "namespace", "new", "noexcept",
    "not", "not_eq", "nullptr", "operator", "or", "or_eq",
    "private", "protected", "public", "register", "reinterpret_cast",
    "requires", "return", "short", "signed", "sizeof", "static",
    "static_assert", "static_cast", "struct", "switch", "template",
    "this", "thread_local", "throw", "true", "try", "typedef",
    "typeid", "typename", "union", "unsigned", "using", "virtual",
    "void", "volatile", "wchar_t", "while", "xor", "xor_eq"
};

enum class TokenType {
    TK_ERR,
    IDENTIFIER,
    KEYWORD,
    PUNCTUATOR,
    STRING,
    NUMBER,
    WHITESPACE,
    NEWLINE,
    TK_EOF
};

struct Token {
    TokenType ttype;
    std::string value;
    int64_t lineno;
    int64_t startindex;
    const std::string* line;
};

std::string tokenToString(const Token& tok) {
    std::string typeStr;
    switch (tok.ttype) {
        case TokenType::IDENTIFIER: typeStr = "IDENTIFIER"; break;
        case TokenType::WHITESPACE: typeStr = "WHITESPACE"; break;
        case TokenType::KEYWORD: typeStr = "KEYWORD"; break;
        case TokenType::PUNCTUATOR: typeStr = "PUNCTUATOR"; break;
        case TokenType::STRING: typeStr = "STRING"; break;
        case TokenType::NUMBER: typeStr = "NUMBER"; break;
        case TokenType::TK_EOF: typeStr = "EOF"; break;
        case TokenType::NEWLINE: typeStr = "NL"; break;
        default: typeStr = "UNKNOWN"; break;
    }
    return "Token(" + typeStr + ", value='" + tok.value + "')";
}
std::ostream& operator<<(std::ostream& out, Token t) {
    out << tokenToString(t);
    return out;
}
std::ofstream& operator<<(std::ofstream& out, Token t) {
    out << t.value;
    return out;
}
bool ispresentin(char c, const std::string& s) {
    return s.find(c) != std::string::npos;
}

class Lexer {
    std::vector<std::string> data;
    int64_t pos_line = 0;
    int64_t pos_char = 0;
    bool at_eof = false;
public:
    Lexer(const std::string& filename) {
        std::ifstream infile(filename);
        std::string line;
        while (std::getline(infile, line)) {
            data.push_back(line + '\n');
        }
        if (data.empty()) data.push_back("\n");
    }
    Token peektoken() {
        int64_t p_line = pos_line;
        int64_t p_char = pos_char;
        bool p_eof = at_eof;
        Token out = gettoken();
        pos_line = p_line;
        pos_char = p_char;
        at_eof = p_eof;
        return out;
    }
    Token gettoken() {
        Token out=_gettoken();
        if (out.value=="/") {
            bool b=true;
            if (_peektoken().value=="*") {
                _gettoken();
                do {
                    while (_gettoken().value!="*"&&_peektoken().ttype!=TokenType::TK_EOF);
                } while (_peektoken().value!="/"&&_peektoken().ttype!=TokenType::TK_EOF);
                _gettoken();
            } else if (_peektoken().value=="/") {
                while (_peektoken().value!="\n"&&_peektoken().ttype!=TokenType::TK_EOF) _gettoken();
            } else {
                b=false;
            }
            if (b) out=_gettoken();
        } else if (out.value=="\n") {
            while (_peektoken().value=="\n") _gettoken();
        }
        std::cout<<out<<std::endl;
        return out;
    }
private:
    Token _peektoken() {
        int64_t p_line = pos_line;
        int64_t p_char = pos_char;
        bool p_eof = at_eof;
        Token out = _gettoken();
        pos_line = p_line;
        pos_char = p_char;
        at_eof = p_eof;
        return out;
    }
    Token _gettoken() {
        while (pos_line < (int64_t)data.size()) {
            std::string& line_str = data[pos_line];
            int64_t line_len = (int64_t)line_str.size();
            if (pos_char >= line_len) {
                pos_line++;
                pos_char = 0;
                return Token{TokenType::NEWLINE, "\n", pos_line, 0, &line_str};
            }

            char c = line_str[pos_char];
            int64_t startindex = pos_char;
            if (c == ' ' || c == '\t' || c == '\r') {
                std::string value;
                while (pos_char < line_len &&
                        (line_str[pos_char] == ' ' ||
                         line_str[pos_char] == '\t' ||
                         line_str[pos_char] == '\r')) {
                    value += line_str[pos_char++];
                }
                return findtoken(value, pos_line + 1, startindex, &line_str);
            }
            if (c == '\n') {
                pos_char++;
                return Token{TokenType::NEWLINE, "\n", pos_line + 1, startindex, &line_str};
            }
            if (std::isalpha(c) || c == '_') {
                std::string value;
                while (pos_char < line_len &&
                       (std::isalnum(line_str[pos_char]) || line_str[pos_char] == '_')) {
                    value += line_str[pos_char++];
                }
                return findtoken(value, pos_line + 1, startindex, &line_str);
            }
            if (std::isdigit(c)) {
                std::string value;
                bool dot = false;
                while (pos_char < line_len &&
                       (std::isdigit(line_str[pos_char]) ||
                        (line_str[pos_char] == '.' && !dot))) {
                    if (line_str[pos_char] == '.') dot = true;
                    value += line_str[pos_char++];
                }
                if (pos_char < line_len && line_str[pos_char] == 'f') value += line_str[pos_char++];
                return findtoken(value, pos_line + 1, startindex, &line_str);
            }
            if (c == '"') {
                std::string value;
                value += line_str[pos_char++]; // opening quote
                bool escaped = false;
                while (pos_char < line_len) {
                    char ch = line_str[pos_char++];
                    value += ch;
                    if (ch == '"' && !escaped) break;
                    if (ch == '\\' && !escaped) escaped = true; else escaped = false;
                }
                return findtoken(value, pos_line + 1, startindex, &line_str);
            }
            if (ispresentin(c, symbols)) {
                std::string value(1, c);
                pos_char++;
                return findtoken(value, pos_line + 1, startindex, &line_str);
            }
            std::string value(1, c);
            pos_char++;
            return findtoken(value, pos_line + 1, startindex, &line_str);
        }

        if (!at_eof) {
            at_eof = true;
            return Token{TokenType::TK_EOF, "", pos_line + 1, pos_char, nullptr};
        }
        return Token{TokenType::TK_EOF, "", pos_line + 1, pos_char, nullptr};
    }
    Token findtoken(const std::string& s, int64_t line, int64_t index, const std::string* line_text) {
        TokenType ttype = TokenType::TK_ERR;
        if (s.empty()) return {TokenType::TK_EOF, "", line, index, line_text};
        if (std::isdigit(s[0])) {
            ttype = TokenType::NUMBER;
        } else if (std::isalpha(s[0]) || s[0] == '_') {
            if (std::find(keywords.begin(), keywords.end(), s) != keywords.end()) {
                ttype = TokenType::KEYWORD;
            } else {
                ttype = TokenType::IDENTIFIER;
            }
        } else if (s[0] == '"' && s.size() >= 2 && s.back() == '"') {
            ttype = TokenType::STRING;
        } else if (ispresentin(s[0], symbols) && s.size() == 1) {
            ttype = TokenType::PUNCTUATOR;
        } else if (s[0] == ' ' || s[0] == '\t' || s[0] == '\r') {
            ttype = TokenType::WHITESPACE;
        } else if (s == "\n") {
            ttype = TokenType::NEWLINE;
        } else {
            ttype = TokenType::TK_ERR;
        }
        return Token{ttype, s, line, index, line_text};
    }
};
