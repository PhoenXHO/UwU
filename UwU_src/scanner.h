#ifndef SCANNER_H_INCLUDED
#define SCANNER_H_INCLUDED

class Token {
    public:
        enum class Kind {
            //Single-character tokens
            T_PLUS, T_MINUS, T_STAR, T_SLASH, T_NOT,
            T_LEFT_PAR, T_RIGHT_PAR, T_COMMA,
            T_NEW_LINE, T_TAB,

            //One/Two-characters tokens
            T_ASSIGN, T_EQUAL, T_NOT_EQUAL,
            T_GREATER, T_GREATER_EQUAL,
            T_LESS, T_LESS_EQUAL,
            T_RIGHT_SQB, T_LEFT_SQB,

            //Literals
            T_IDENTIFIER,
            T_STRING, T_NUMBER, T_CHAR,

            //Keywords
            T_AND, T_OR,
            T_VAR,
            T_PRINT, T_PRINT_END,
            T_READ, T_READ_END,
            T_READ_STRING, T_READ_NUMBER, T_READ_CHAR,
            T_IF, T_ELSE, T_LOOP,
            T_TRUE, T_FALSE,

            T_ERROR, T_EOF
        };

        Token(Kind kind, const char * start, int length, int line) :
            _kind{kind}, _start(start), _length(length), _line(line) {}

        const char * start() { return _start;  }
        int length()         { return _length; }
        int line()           { return _line;   }
        Kind kind()          { return _kind;   }

    private:
        Kind _kind{};
        const char * _start;
        int _length;
        int _line;

};

typedef Token::Kind Kind;

void init_scanner(const char *);
Token scan_token();

#endif // SCANNER_H_INCLUDED
