#include <iostream>
#include <vector>
#include <string>
#include <cctype>
#include <map>
#include <fstream>

using namespace std;

enum TokenType {
    T_INT, T_FLOAT, T_DOUBLE, T_STRING, T_BOOL, T_CHAR, T_ID, T_NUM, T_IF, T_ELSE, T_RETURN, T_WHILE, T_FOR, T_ASSIGN,
    T_PLUS, T_MINUS, T_MUL, T_DIV, T_LPAREN, T_RPAREN, T_LBRACE, T_RBRACE, T_SEMICOLON, T_GT, T_LT, T_TRUE, T_FALSE, 
    T_EOF
};

struct Token {
    TokenType type;
    string value;
    int line;
};

class Lexer {
private:
    string src;
    size_t pos;
    int line;

public:
    Lexer(const string &src) {
        this->src = src;
        this->pos = 0;
        this->line = 1;
    }

    string consumeNumber() {
        size_t start = pos;
        while (pos < src.size() && (isdigit(src[pos]) || src[pos] == '.')) pos++;
        return src.substr(start, pos - start);
    }

    string consumeWord() {
        size_t start = pos;
        while (pos < src.size() && isalnum(src[pos])) pos++;
        return src.substr(start, pos - start);
    }

    string consumeString() {
        pos++; 
        size_t start = pos;
        while (pos < src.size() && src[pos] != '"') {
            pos++;
        }
        if (pos >= src.size()) {
            cout << "Unterminated string literal at line " << line << endl;
            exit(1);
        }
        string str = src.substr(start, pos - start);
        pos++;  
        return str;
    }

    char consumeChar() {
        pos++; 
        if (pos + 1 >= src.size() || src[pos + 1] != '\'') {
            cout << "Invalid character literal at line " << line << endl;
            exit(1);
        }
        char ch = src[pos];
        pos += 2;
        return ch;
    }

    vector<Token> tokenizer() {
        vector<Token> tokens;
        while (pos < src.size()) {
            char current = src[pos];
            if (isspace(current)) {
                if (current == '\n') line++;
                pos++;
                continue;
            }
            if (isdigit(current)) {
                tokens.push_back(Token{T_NUM, consumeNumber(), line});
                continue;
            }
            if (isalpha(current)) {
                string word = consumeWord();
                if (word == "int") tokens.push_back(Token{T_INT, word, line});
                else if (word == "float") tokens.push_back(Token{T_FLOAT, word, line});
                else if (word == "double") tokens.push_back(Token{T_DOUBLE, word, line});
                else if (word == "string") tokens.push_back(Token{T_STRING, word, line});
                else if (word == "bool") tokens.push_back(Token{T_BOOL, word, line});
                else if (word == "char") tokens.push_back(Token{T_CHAR, word, line});
                else if (word == "agar") tokens.push_back(Token{T_IF, word, line});
                else if (word == "else") tokens.push_back(Token{T_ELSE, word, line});
                else if (word == "return") tokens.push_back(Token{T_RETURN, word, line});
                else if (word == "while") tokens.push_back(Token{T_WHILE, word, line});
                else if (word == "for") tokens.push_back(Token{T_FOR, word, line});
                else if (word == "true") tokens.push_back(Token{T_TRUE, word, line});
                else if (word == "false") tokens.push_back(Token{T_FALSE, word, line});
                else tokens.push_back(Token{T_ID, word, line});
                continue;
            }
            if (current == '"') {
                tokens.push_back(Token{T_STRING, consumeString(), line});
                continue;
            }
            if (current == '\'') {
                tokens.push_back(Token{T_CHAR, string(1, consumeChar()), line});
                continue;
            }
            switch (current) {
                case '=': tokens.push_back(Token{T_ASSIGN, "=", line}); break;
                case '+': tokens.push_back(Token{T_PLUS, "+", line}); break;
                case '-': tokens.push_back(Token{T_MINUS, "-", line}); break;
                case '*': tokens.push_back(Token{T_MUL, "*", line}); break;
                case '/': tokens.push_back(Token{T_DIV, "/", line}); break;
                case '(': tokens.push_back(Token{T_LPAREN, "(", line}); break;
                case ')': tokens.push_back(Token{T_RPAREN, ")", line}); break;
                case '{': tokens.push_back(Token{T_LBRACE, "{", line}); break;
                case '}': tokens.push_back(Token{T_RBRACE, "}", line}); break;
                case ';': tokens.push_back(Token{T_SEMICOLON, ";", line}); break;
                case '>': tokens.push_back(Token{T_GT, ">", line}); break;
                case '<': tokens.push_back(Token{T_LT, "<", line}); break;
                default:
                    cout << "Unexpected character: " << current << " at line " << line << endl;
                    exit(1);
            }
            pos++;
        }
        tokens.push_back(Token{T_EOF, "", line});
        return tokens;
    }
};

class Parser {
private:
    vector<Token> tokens;
    size_t pos;

public:
    Parser(const vector<Token> &tokens) {
        this->tokens = tokens;
        this->pos = 0;
    }

    void parseProgram() {
        while (tokens[pos].type != T_EOF) {
            parseStatement();
        }
        cout << "Parsing completed successfully! No syntax errors." << endl;
    }

    void parseStatement() {
        if (tokens[pos].type == T_INT || tokens[pos].type == T_FLOAT || tokens[pos].type == T_DOUBLE ||
            tokens[pos].type == T_STRING || tokens[pos].type == T_BOOL || tokens[pos].type == T_CHAR) {
            parseDeclaration();
        } else if (tokens[pos].type == T_ID) {
            parseAssignment();
        } else if (tokens[pos].type == T_IF) {
            parseIfStatement();
        } else if (tokens[pos].type == T_WHILE) {
            parseWhileStatement();
        } else if (tokens[pos].type == T_FOR) {
            parseForStatement();
        } else if (tokens[pos].type == T_RETURN) {
            parseReturnStatement();
        } else if (tokens[pos].type == T_LBRACE) {
            parseBlock();
        } else {
            reportError("Unexpected token");
        }
    }

    void parseBlock() {
        expect(T_LBRACE);
        while (tokens[pos].type != T_RBRACE && tokens[pos].type != T_EOF) {
            parseStatement();
        }
        expect(T_RBRACE);
    }

    void parseDeclaration() {
        pos++;  // Skip the type (int, float, double, etc.)
        expect(T_ID);
        expect(T_SEMICOLON);
    }

    void parseAssignment() {
        expect(T_ID);
        expect(T_ASSIGN);
        parseExpression();
        expect(T_SEMICOLON);
    }
    void parseAssignment_FOR() {
        expect(T_ID);
        expect(T_ASSIGN);
        parseExpression();
    }
    void parseIfStatement() {
        expect(T_IF);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
        if (tokens[pos].type == T_ELSE) {
            expect(T_ELSE);
            parseStatement();
        }
    }

    void parseWhileStatement() {
        expect(T_WHILE);
        expect(T_LPAREN);
        parseExpression();
        expect(T_RPAREN);
        parseStatement();
    }

    void parseForStatement() {
        expect(T_FOR);
        expect(T_LPAREN);
        parseAssignment();
        parseExpression();
        expect(T_SEMICOLON);
        parseAssignment_FOR();
        expect(T_RPAREN);
        parseStatement();
    }

    void parseReturnStatement() {
        expect(T_RETURN);
        parseExpression();
        expect(T_SEMICOLON);
    }

    void parseExpression() {
        parseTerm();
        while (tokens[pos].type == T_PLUS || tokens[pos].type == T_MINUS || tokens[pos].type == T_GT || tokens[pos].type == T_LT) {
            pos++;
            parseTerm();
        }
    }

    void parseTerm() {
        parseFactor();
        while (tokens[pos].type == T_MUL || tokens[pos].type == T_DIV) {
            pos++;
            parseFactor();
        }
    }

    void parseFactor() {
        if (tokens[pos].type == T_NUM || tokens[pos].type == T_ID || tokens[pos].type == T_STRING || tokens[pos].type == T_CHAR) {
            pos++;
        } else if (tokens[pos].type == T_LPAREN) {
            expect(T_LPAREN);
            parseExpression();
            expect(T_RPAREN);
        } else if (tokens[pos].type == T_TRUE || tokens[pos].type == T_FALSE) {
            pos++;  // Boolean literals
        } else {
            reportError("Unexpected token in expression");
        }
    }

    void expect(TokenType type) {
        if (tokens[pos].type != type) {
            reportError("Expected token type " + to_string(type));
        }
        pos++;
    }

    void reportError(const string &message) {
        cout << "Syntax error: " << message << " at line " << tokens[pos].line << endl;
        exit(1);
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename>\n";
        return 1;
    }

    std::ifstream file(argv[1]);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << "\n";
        return 1;
    }

    std::string source_code((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    Lexer lexer(source_code);
    std::vector<Token> tokens = lexer.tokenizer();

    for (const auto& token : tokens)
    {
        std::cout << "Token: " << token.value << " Type: " << token.type << std::endl;
    }

    Parser parser(tokens);
    parser.parseProgram();

    return 0;
}




