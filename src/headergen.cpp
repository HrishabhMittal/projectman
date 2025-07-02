#include "lexer.cpp"
#include <cstdio>
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>
class HeaderGen {
private:
    Lexer l;
    std::ofstream header;
    std::string headername;
    bool matchnext(TokenType type, const std::string& val = "") {
        Token x=l.peektoken();
        return x.ttype == type && (val.empty() || x.value == val);
    }
    bool matchnext(const std::string& val = "") {
        Token x=l.peektoken();
        return (val.empty() || x.value == val);
    }
    bool match(Token currentToken,TokenType type, const std::string& val = "") {
        return currentToken.ttype == type && (val.empty() || currentToken.value == val);
    }
    void genheader() {
        while (genstatement());
    }
    bool genstatement() {
        if (matchnext(TokenType::TK_EOF)) return 0;
        while (l.peektoken().ttype==TokenType::NEWLINE||l.peektoken().ttype==TokenType::WHITESPACE) {header<<l.gettoken();}
        if (matchnext(TokenType::PUNCTUATOR,"#")) {
            std::cout<<"# rn"<<std::endl;
            header<<l.gettoken();
            bool booll=true;
            while (booll) {
                Token tok=l.gettoken();
                while (!matchnext(TokenType::NEWLINE)) {
                    if (matchnext(TokenType::TK_EOF)) return 0;
                    header<<tok;
                    tok=l.gettoken();
                }
                header<<tok;
                if (!match(tok,TokenType::PUNCTUATOR,"\\")) {
                    booll=false;
                }
                tok=l.gettoken();
                header<<tok;
            }
            std::cout<<"#end rn"<<std::endl;
        } else if (matchnext("namespace")||matchnext("class")) {
            std::cout<<"namespace/class rn"<<std::endl;
            header<<l.gettoken();
            while (!matchnext(";")&&!matchnext("{")) {
                if (matchnext(TokenType::TK_EOF)) return 0;
                header<<l.gettoken();
            }
            if (matchnext(";")) {
                header<<l.gettoken();
            } else if (matchnext("{")) {
                header<<l.gettoken();
                while (l.peektoken().ttype==TokenType::NEWLINE||l.peektoken().ttype==TokenType::WHITESPACE) {
                    if (matchnext(TokenType::TK_EOF)) return 0;
                    header<<l.gettoken();
                }
                while (!matchnext("}")) {
                    if (matchnext(TokenType::TK_EOF)) return 0;
                    genstatement();    
                    while (l.peektoken().ttype==TokenType::NEWLINE||l.peektoken().ttype==TokenType::WHITESPACE) {
                        if (matchnext(TokenType::TK_EOF)) return 0;
                        header<<l.gettoken();
                    }
                }
                header<<l.gettoken();
            }
        } else if (matchnext("template")) {
            header<<l.gettoken();
            while (!matchnext(";")&&!matchnext("{")) {
                if (matchnext(TokenType::TK_EOF)) return 0;
                header<<l.gettoken();
            }
            if (matchnext(";")) {
                header<<l.gettoken();
            } else if (matchnext("{")) {
                int scope=1;
                bool run=true;
                header<<l.gettoken();
                while (run) {
                    if (matchnext(TokenType::TK_EOF)) return 0;
                    if (matchnext("}")) scope--;
                    else if (matchnext("{")) scope++;
                    if (scope==0) run=false;
                    header<<l.gettoken();
                }
            }
        } else if (matchnext("enum")) {
            std::cout<<"enum rn"<<std::endl;
            while (!matchnext(";")) {
                if (matchnext(TokenType::TK_EOF)) return 0;
                header<<l.gettoken();
            }
            header<<l.gettoken();
        } else if (matchnext(TokenType::KEYWORD)) {
            std::cout<<"trying decalaration of something rn"<<std::endl;
            header<<l.gettoken();
            while (!matchnext(";")&&!matchnext("{")) {
                if (matchnext(TokenType::TK_EOF)) return 0;
                header<<l.gettoken();
            }
            if (matchnext(";")) {
                header<<l.gettoken();
            } else if (matchnext("{")) {
                int scope=1;
                bool run=true;
                while (run) {
                    if (matchnext(TokenType::TK_EOF)) return 0;
                    if (matchnext("}")) scope--;
                    else if (matchnext("{")) scope++;
                    if (scope==0) run=false;
                    l.gettoken();
                }
                header<<";";
            }
        } else if (matchnext(TokenType::TK_EOF)) {
            std::cout<<"eof rn"<<std::endl;
            return 0;
        } else {
            std::cout<<"unknown rn"<<std::endl;
            while (!matchnext(TokenType::NEWLINE)) {
                if (matchnext(TokenType::TK_EOF)) return 0;
                header<<l.gettoken();
            }
            header<<l.gettoken();
        }
        return 1;
    }
public:
    HeaderGen(const std::string& filename): l(filename) {
        if (filename.size()>4&&filename.substr(filename.size()-4,4)!=".cpp") {
            std::cout<<"filename doesn't have .cpp extension, quitting..."<<std::endl;
            return;
        }
        headername=filename;
        headername[headername.size()-3]='h';
        header=std::ofstream(headername);
        genheader();
        std::cout<<"header generated for "<<filename<<" in "<<headername<<std::endl;
        header.close();
    }
};
