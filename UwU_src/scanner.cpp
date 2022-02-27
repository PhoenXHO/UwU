#include "common.h"
#include "scanner.h"

typedef struct
{
    const char * start;
    const char * current;
    int line;
} Scanner;

Scanner scanner;

void init_scanner(const char * source)
{
    scanner.start = scanner.current = source;
    scanner.line = 1;
}

bool is_at_end()
{
    return *scanner.current == '\0';
}

char advance()
{
    scanner.current++;
    return scanner.current[-1];
}

static char peek()
{
    return *scanner.current;
}

static char peek_next()
{
    if (is_at_end()) return '\0';
    return scanner.current[1];
}

static bool match(char expected)
{
    if (is_at_end()) return false;
    if (*scanner.current != expected) return false;

    scanner.current++;
    return true;
}

Token make_token(Kind kind)
{
    Token token = Token(kind, scanner.start, (int)(scanner.current - scanner.start), scanner.line);

    return token;
}

static Token error_token(const char * message)
{
    Token token = Token(Kind::T_ERROR, message, (int)strlen(message), scanner.line);

    return token;
}

static void skip_whitespace()
{
    while (true)
    {
        char c = peek();
        switch (c)
        {
            case ' ': case '\r': case '\t': advance(); break;

            case '\n':
                advance();
                if (!is_at_end()) scanner.line++;
                break;

            case '{':
            {
                if (peek_next() == ':')
                {
                    while (!is_at_end())
                    {
                        if (match(':') && match('}')) break;
                        advance();
                    }
                }
                else
                {
                    return;
                }
                break;
            }

            default: return;
        }
    }
}

static Kind check_keyword(int start, int length, const char * rest, Kind kind, bool can_be_id)
{
    if (scanner.current - scanner.start == start + length &&
        memcmp(scanner.start + start, rest, length) == 0)
    {
        return kind;
    }

    if (can_be_id) return Kind::T_IDENTIFIER;
    return Kind::T_ERROR;
}

static Kind identifier_kind()
{
    switch (scanner.start[0])
    {
        case 'a': return check_keyword(1, 3, "wnd",   Kind::T_AND,   true);
        case 'i': return check_keyword(1, 2, "wi",    Kind::T_READ,  true);
        case '?': return check_keyword(1, 2, "w?",    Kind::T_IF,    false);
        case 'e': return check_keyword(1, 2, "we",    Kind::T_ELSE,  true);
        case 't': return check_keyword(1, 3, "wue",   Kind::T_TRUE,  true);

        case 'f':
            if (scanner.start[1])
            {
                switch (scanner.start[1])
                {
                    case 'a': return check_keyword(2, 3, "wse",  Kind::T_FALSE, true);
                    case 'w': return check_keyword(2, 2, "un",   Kind::T_FUN,   true);
                }
            }

        case 'u':
            if (scanner.start[1])
            {
                switch (scanner.start[1])
                {
                    case 'w': return check_keyword(2, 1, "u", Kind::T_VAR, true);
                    case 'n': return check_keyword(2, 3, "tiw", Kind::T_LOOP, true);
                }
            }
            break;

        case 'o':
            if (scanner.start[1])
            {
                switch (scanner.start[1])
                {
                    case 'w': return Kind::T_OR;
                    case 'u':
                        if (scanner.start[2])
                        {
                            switch (scanner.start[2])
                            {
                                case 'o': return Kind::T_PRINT;
                                case 't': return Kind::T_OUT;
                            }
                        }
                }
            }
            break;
    }

    return Kind::T_IDENTIFIER;
}

static Token _identifier()
{
    while(isalnum(peek()) || peek() == '?' || peek() == '.' || peek() == '^')
        advance();

    return make_token(identifier_kind());
}

static Token _number()
{
    while (isdigit(peek())) advance();

    if (peek() == '.' && isdigit(peek_next()))
    {
        advance();
        while (isdigit(peek())) advance();
    }

    return make_token(Kind::T_NUMBER);
}

static Token _string()
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n' && peek_next() != '\0') scanner.line++;
        advance();
    }

    if (is_at_end()) return error_token("untewminated stwing.");

    advance();

    return make_token(Kind::T_STRING);
}

static Token _char()
{
    if (is_at_end())   return error_token("untewminated c-chawactew towken.");
    if (peek() == '`') return error_token("empty c-chawactew constwant.");
    advance();
    while (true)
    {
        if (is_at_end()) return error_token("untewminated c-chawactew towken.");
        if (peek() == '`' )
        {
            if (scanner.current - scanner.start == 2) break;
            return error_token("invawid chawactew towken.");
        }
        advance();
    }
    advance();

    return make_token(Kind::T_CHAR);
}

Token scan_token()
{
    skip_whitespace();

    scanner.start = scanner.current;

    if (is_at_end()) return make_token(Kind::T_EOF);

    char c = advance();

    if (isalpha(c) || c == '.' || c == '^' || c == '?')
        return _identifier();
    if (isdigit(c))
        return _number();

    switch (c)
    {
        case '+': return make_token(Kind::T_PLUS);
        case '*': return make_token(Kind::T_STAR);
        case '/': return make_token(Kind::T_SLASH);
        case '=': return make_token(Kind::T_EQUAL);

        case '-':
            if (match('s'))
            {
                return make_token(Kind::T_READ_STRING);
            }
            else if (match('d'))
            {
                return make_token(Kind::T_READ_NUMBER);
            }
            else if (match('c'))
            {
                return make_token(Kind::T_READ_CHAR);
            }
            else
            {
                return make_token(Kind::T_MINUS);
            }

        case '(': return make_token(Kind::T_LEFT_PAR);
        case ')': return make_token(Kind::T_RIGHT_PAR);

        case ',': return make_token(Kind::T_COMMA);

        case '~':
            if (match('n')) return make_token(Kind::T_NEW_LINE);
            if (match('t')) return make_token(Kind::T_TAB);
            break;

        case '!':
            return make_token(match('=') ? Kind::T_NOT_EQUAL : Kind::T_NOT);
        case '<':
            return make_token(match('=') ? Kind::T_LESS_EQUAL : (match('<') ? Kind::T_READ_END : Kind::T_LESS));
        case '>':
            return make_token(match('=') ? Kind::T_GREATER_EQUAL : (match('>') ? Kind::T_OUT_END : Kind::T_GREATER));

        case '"': return _string();
        case '`': return _char();

        case '[':
            return make_token(match(':') ? Kind::T_BLOCK_START : Kind::T_LEFT_SQB);
            break;
        case ']':
            return make_token(Kind::T_RIGHT_SQB);

        case ':':
        {
            if (peek() == '=')
            {
                advance();
                return make_token(Kind::T_ASSIGN);
            }
            else if (peek() == ']')
            {
                advance();
                return make_token(Kind::T_BLOCK_END);
            }
        }
    }

    return error_token("unexpectwed chawactew.");
}
