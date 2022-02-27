#include "common.h"
#include "compiler.h"
#include "scanner.h"

int DEBUG_PRINT_CODE = 0;
int DEBUG_TRACE_EXECUTION = 0;

typedef struct
{
    Token current = Token(Kind::T_EOF, NULL, 0, 0);
    Token previous = Token(Kind::T_EOF, NULL, 0, 0);
    bool had_error;
    bool panic_mode;
} Parser;

typedef enum
{
    P_NONE,
    P_ASSIGNMENT, // :=
    P_OR,         // ow
    P_AND,        // awnd
    P_EQUALITY,   // =
    P_COMPARISON, // < <= > >=
    P_TERM,       // + -
    P_FACTOR,     // * /
    P_UNARY,      // ! -
    P_CALL,       // ()
    P_PRIMARY,
} Precedence;

typedef void (* ParseFn)(bool);

typedef struct
{
    ParseFn prefix;
    ParseFn infix;
    Precedence precedence;
} ParseRule;

typedef struct
{
    Token name = Token(Kind::T_EOF, NULL, 0, 0);
    int depth;
} Local;

typedef enum
{
    TYPE_SCRIPT,
    TYPE_FUNCTION,
} FunType;

class Compiler
{
    private:
        Compiler * _enclosing;
        int _local_count;
        int _scope_depth;
        Local _locals[UINT8_COUNT];
        Function * _function;
        FunType _type;

    public:
        Compiler * & enlosing()   { return _enclosing;   }

        int & local_count()       { return _local_count; }
        int & scope_depth()       { return _scope_depth; }
        Local * locals()          { return _locals;      }
        Function * & __function() { return _function;    }
        FunType & ftype()         { return _type;        }
};

Parser parser;
Compiler * current = NULL;

static Chunk * current_chunk()
{
    return &current->__function()->chunk();
}

static void error_at(Token * token, const char * message)
{
    if (parser.panic_mode) return;
    parser.panic_mode = true;

    fprintf(stderr, "[line %d] Ewwow", token->line());

    if (token->kind() == Kind::T_EOF)
    {
        fprintf(stderr, " at end");
    }
    else if (token->kind() == Kind::T_ERROR)
    {

    }
    else
    {
        fprintf(stderr, " at '%.*s'", token->length(), token->start());
    }

    fprintf(stderr, ": ");
    fprintf(stderr, message, parser.current.length(), parser.current.start());
    fprintf(stderr, "\n");

    parser.had_error = true;
}

static void error(const char * message)
{
    error_at(&parser.previous, message);
}

static void error_at_current(const char * message)
{
    error_at(&parser.current, message);
}

static void advance()
{
    parser.previous = parser.current;

    while (true)
    {
        parser.current = scan_token();

        if (parser.current.kind() != Kind::T_ERROR) break;

        error_at_current(parser.current.start());
    }
}

static void consume(Kind kind, const char * message)
{
    if (parser.current.kind() == kind)
    {
        advance();
        return;
    }

    error_at_current(message);
}

static bool check(Kind kind)
{
    return parser.current.kind() == kind;
}

static bool match(Kind kind)
{
    if (!check(kind)) return false;
    advance();
    return true;
}

static void emit_byte(uint8_t byte)
{
    current_chunk()->write(byte, parser.previous.line());
}

static void emit_bytes(uint8_t byte1, uint8_t byte2)
{
    emit_byte(byte1);
    emit_byte(byte2);
}

static void emit_loop(int loop_start)
{
    emit_byte(OP_LOOP);

    int offset = current_chunk()->ccount() - loop_start + 2;
    if (offset > UINT16_MAX) error("w-woop body is t-too wawge.");

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

static int emit_jump(uint8_t instruction)
{
    emit_byte(instruction);
    emit_byte(0xff);
    emit_byte(0xff);
    return current_chunk()->ccount() - 2;
}

static void emit_return()
{
    emit_byte(OP_NULL);
    emit_byte(OP_OUT);
}

static void init_compiler(Compiler * compiler, FunType type)
{
    compiler->enlosing() = current;

    compiler->__function() = NULL;
    compiler->ftype() = type;
    compiler->local_count() = 0;
    compiler->scope_depth() = 0;
    compiler->__function() = new_function();

    current = compiler;

    if (type != TYPE_SCRIPT)
    {
        current->__function()->name() = copy_string(parser.previous.start(), parser.previous.length());
    }

    Local * local = &current->locals()[current->local_count()++];
    local->depth = 0;
    local->name.start() = "";
    local->name.length() = 0;
}

static Function * end_compiler()
{
    emit_return();
    Function * _function = current->__function();

    if (DEBUG_PRINT_CODE)
    {
        if (!parser.had_error)
        {
            current_chunk()->disassemble(_function->name() != NULL ? _function->name()->chars() : "<script>");
        }
    }

    current = current->enlosing();
    return _function;
}

static uint8_t make_constant(Value value)
{
    int constant = current_chunk()->add_constant(value);
    if (constant >UINT8_MAX)
    {
        error("too many constwants in one chwnk.");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Value value)
{
    emit_bytes(OP_CONSTANT, make_constant(value));
}

static void patch_jump(int offset)
{
    int jump = current_chunk()->ccount() - offset - 2;

    if (jump > UINT16_MAX)
    {
        error("too much c-code to jwump ovew.");
    }

    current_chunk()->ccode()[offset] = (jump >> 8) & 0xff;
    current_chunk()->ccode()[offset + 1] = jump & 0xff;
}

static void begin_scope()
{
    current->scope_depth()++;
}

static void end_scope()
{
    current->scope_depth()--;

    while (current->local_count() > 0 && current->locals()[current->local_count() - 1].depth > current->scope_depth())
    {
        emit_byte(OP_POP);
        current->local_count()--;
    }
}

static void expression();
static void statement();
static void declaration();
static ParseRule * get_rule(Kind);
static void parse_precedence(Precedence precedence);

static void _binary(bool)
{
    Kind operator_kind = parser.previous.kind();

    ParseRule * rule = get_rule(operator_kind);
    parse_precedence((Precedence)(rule->precedence + 1));

    switch (operator_kind)
    {
        case Kind::T_PLUS:  emit_byte(OP_ADD);      break;
        case Kind::T_MINUS: emit_byte(OP_SUBTRACT); break;
        case Kind::T_STAR:  emit_byte(OP_MULTIPLY); break;
        case Kind::T_SLASH: emit_byte(OP_DIVIDE);   break;

        case Kind::T_EQUAL:         emit_byte(OP_EQUAL);         break;
        case Kind::T_NOT_EQUAL:     emit_byte(OP_NOT_EQUAL);     break;
        case Kind::T_GREATER:       emit_byte(OP_GREATER);       break;
        case Kind::T_GREATER_EQUAL: emit_byte(OP_GREATER_EQUAL); break;
        case Kind::T_LESS:          emit_byte(OP_LESS);          break;
        case Kind::T_LESS_EQUAL:    emit_byte(OP_LESS_EQUAL);    break;

        default: return;
    }
}

static void _literal(bool)
{
    switch (parser.previous.kind())
    {
        case Kind::T_TRUE:  emit_byte(OP_TRUE);  break;
        case Kind::T_FALSE: emit_byte(OP_FALSE); break;

        default: return;
    }
}

static void _grouping(bool)
{
    expression();
    consume(Kind::T_RIGHT_PAR, "')' expected aftew expwession.");
}

static void _number(bool)
{
    double value = strtod(parser.previous.start(), NULL);
    emit_constant(NUMBER_VAL(value));
}

static void _and(bool)
{
    int end_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    parse_precedence(P_AND);

    patch_jump(end_jump);
}

static void _or(bool)
{
    int end_jump = emit_jump(OP_JUMP_IF_TRUE);
    emit_byte(OP_POP);
    parse_precedence(P_OR);

    patch_jump(end_jump);
}

static void _string(bool)
{
    emit_constant(
                  OBJECT_VAL(
                             copy_string(
                                         parser.previous.start() + 1,
                                         parser.previous.length() - 2
                                         )
                             )
                  );
}

static void _char(bool)
{
    Value c = CHAR_VAL(*(parser.previous.start() + 1));
    emit_constant(c);
}

static void _newline(bool)
{
    Value newline = CHAR_VAL('\n');
    emit_constant(newline);
}

static void _tab(bool)
{
    Value tab = CHAR_VAL('\t');
    emit_constant(tab);
}

static uint8_t identifier_constant(Token * name)
{
    return make_constant(
                         OBJECT_VAL(
                                    copy_string(
                                                name->start(),
                                                name->length()
                                                )
                                    )
                         );
}

static bool identifiers_equal(Token * a, Token * b)
{
    if (a->length() != b->length()) return false;
    return memcmp(a->start(), b->start(), a->length()) == 0;
}

static int resolve_local(Compiler * compiler, Token * name)
{
    for (int i = compiler->local_count() - 1; i >= 0; i--)
    {
        Local * local = &compiler->locals()[i];
        if (identifiers_equal(name, &local->name))
        {
            if (local->depth == -1)
            {
                error("can't wead wocaw vawiable fwom its o-own initiawizew.");
            }

            return i;
        }
    }

    return -1;
}

static void add_local(Token name)
{
    if (current->local_count() == UINT8_COUNT)
    {
        error("too m-many wocaw v-vawiabwes in scowpe.");
        return;
    }

    Local * local = &current->locals()[current->local_count()++];
    local->name = name;
    local->depth = -1;
}

static void declare_variable()
{
    if (current->scope_depth() == 0) return;

    Token * name = &parser.previous;
    for (int i = current->local_count() - 1; i >= 0; i--)
    {
        Local * local = &current->locals()[i];
        if (local->depth != -1 && local->depth < current->scope_depth())
        {
            break;
        }

        if (identifiers_equal(name, &local->name))
        {
            error("muwtiple definitions of t-the same vawiabwe in the same s-scope.");
        }
    }

    add_local(*name);
}

static void named_variable(Token name, bool can_assign, bool read)
{
    int get_op, set_op;
    int arg = resolve_local(current, &name);

    if (arg != -1)
    {
        get_op = OP_GET_LOCAL;
        set_op = OP_SET_LOCAL;
    }
    else
    {
        arg = identifier_constant(&name);
        get_op = OP_GET_GLOBAL;
        set_op = OP_SET_GLOBAL;
    }

    if (can_assign && (match(Kind::T_ASSIGN) || read))
    {
        if (!read) expression();
        emit_bytes(set_op, (uint8_t)arg);
    }
    else
    {
        emit_bytes(get_op, (uint8_t)arg);
    }
}

static void _variable(bool can_assign)
{
    named_variable(parser.previous, can_assign, false);
}

static void _unary(bool)
{
    Kind operator_kind = parser.previous.kind();

    parse_precedence(P_UNARY);

    switch (operator_kind)
    {
        case Kind::T_MINUS: emit_byte(OP_NEGATE); break;
        case Kind::T_NOT:   emit_byte(OP_NOT);    break;

        default: return;
    }
}

static uint8_t arg_list()
{
    uint8_t arg_count = 0;
    if (!check(Kind::T_RIGHT_PAR))
    {
        do
        {
            expression();

            if (arg_count == 255)
            {
                error("can't have mowe than 255 awguments.");
            }

            arg_count++;
        } while (match(Kind::T_COMMA));
    }

    consume(Kind::T_RIGHT_PAR, "')' expected aftew fwunction awguments.");
    return arg_count;
}

static void _call(bool)
{
    uint8_t arg_count = arg_list();
    emit_bytes(OP_CALL, arg_count);
}

ParseRule rules[] =
{
    /*[Kind::T_PLUS]          =*/ {NULL,    _binary, P_TERM},
    /*[Kind::T_MINUS]         =*/ {_unary,  _binary, P_TERM},
    /*[Kind::T_STAR]          =*/ {NULL,    _binary, P_FACTOR},
    /*[Kind::T_SLASH]         =*/ {NULL,    _binary, P_FACTOR},

    /*[Kind::T_NOT]           =*/ {_unary,     NULL, P_NONE},

    /*[Kind::T_LEFT_PAR]      =*/ {_grouping, _call, P_CALL},
    /*[Kind::T_RIGHT_PAR]     =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_COMMA]         =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_NEW_LINE]      =*/ {_newline,   NULL, P_NONE},
    /*[Kind::T_TAB]           =*/ {_tab,       NULL, P_NONE},

    /*[Kind::T_ASSIGN]        =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_EQUAL]         =*/ {NULL,    _binary, P_EQUALITY},
    /*[Kind::T_NOT_EQUAL]     =*/ {NULL,    _binary, P_EQUALITY},

    /*[Kind::T_GREATER]       =*/ {NULL,    _binary, P_COMPARISON},
    /*[Kind::T_GREATER_EQUAL] =*/ {NULL,    _binary, P_COMPARISON},
    /*[Kind::T_LESS]          =*/ {NULL,    _binary, P_COMPARISON},
    /*[Kind::T_LESS_EQUAL]    =*/ {NULL,    _binary, P_COMPARISON},

    /*[Kind::T_LEFT_SQB]      =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_RIGHT_SQB]     =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_BLOCK_START]   =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_BLOCK_END]     =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_IDENTIFIER]    =*/ {_variable,  NULL, P_NONE},
    /*[Kind::T_STRING]        =*/ {_string,    NULL, P_NONE},
    /*[Kind::T_NUMBER]        =*/ {_number,    NULL, P_NONE},
    /*[Kind::T_CHAR]          =*/ {_char,      NULL, P_NONE},

    /*[Kind::T_AND]           =*/ {NULL,       _and, P_AND},
    /*[Kind::T_OR]            =*/ {NULL,        _or, P_OR},

    /*[Kind::T_VAR]           =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_FUN]           =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_PRINT]         =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_READ]          =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_READ_END]      =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_READ_STRING]   =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_READ_NUMBER]   =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_READ_CHAR]     =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_IF]            =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_ELSE]          =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_LOOP]          =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_TRUE]          =*/ {_literal,   NULL, P_NONE},
    /*[Kind::T_FALSE]         =*/ {_literal,   NULL, P_NONE},

    /*[Kind::T_OUT]           =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_OUT_END]       =*/ {NULL,       NULL, P_NONE},

    /*[Kind::T_ERROR]         =*/ {NULL,       NULL, P_NONE},
    /*[Kind::T_EOF]           =*/ {NULL,       NULL, P_NONE},
};

static void parse_precedence(Precedence precedence)
{
    advance();
    ParseFn prefix_rule = get_rule(parser.previous.kind())->prefix;
    if (prefix_rule == NULL)
    {
        error("expwession expected.");
        return;
    }

    bool can_assign = precedence <= P_ASSIGNMENT;
    prefix_rule(can_assign);

    while (precedence <= get_rule(parser.current.kind())->precedence)
    {
        advance();
        ParseFn infix_rule = get_rule(parser.previous.kind())->infix;
        infix_rule(can_assign);
    }

    if (can_assign && match(Kind::T_ASSIGN))
    {
        error("invawid assignment tawget.");
    }
}

static uint8_t parse_variable(const char * error_message)
{
    consume(Kind::T_IDENTIFIER, error_message);

    declare_variable();
    if (current->scope_depth() > 0) return 0;

    return identifier_constant(&parser.previous);
}

static void mark_initialized()
{
    if (current->scope_depth() == 0) return;
    current->locals()[current->local_count() - 1].depth = current->scope_depth();
}

static void define_variable(uint8_t global)
{
    if (current->scope_depth() > 0)
    {
        mark_initialized();
        return;
    }

    emit_bytes(OP_DEFINE_GLOBAL, global);
}

static ParseRule * get_rule(Kind kind)
{
    return &rules[(int)kind];
}

static void expression()
{
    parse_precedence(P_ASSIGNMENT);
}

static void block()
{
    while (!check(Kind::T_BLOCK_END) && !check(Kind::T_EOF))
    {
        declaration();
    }

    consume(Kind::T_BLOCK_END, "':]' expected aftew bwock.");
}

static void _function(FunType type)
{
    Compiler compiler;
    init_compiler(&compiler, type);
    begin_scope();

    consume(Kind::T_LEFT_PAR, "'(' expected aftew fwunction name.");
    if (!check(Kind::T_RIGHT_PAR))
    {
        do
        {
            current->__function()->arity()++;
            if (current->__function()->arity() > 255)
            {
                error_at_current("can't have mowe than 255 pawametews.");
            }

            uint8_t param_constant = parse_variable("pawamtew name expected.");
            define_variable(param_constant);
        } while (match(Kind::T_COMMA));
    }
    consume(Kind::T_RIGHT_PAR, "')' expected aftew fwunction pawametews.");

    consume(Kind::T_BLOCK_START, "'[:' expected befowe fwunction body.");
    block();

    Function * _function = end_compiler();
    emit_bytes(OP_CONSTANT, make_constant(OBJECT_VAL(_function)));
}

static void function_declaration()
{
    uint8_t global = parse_variable("fwunction name e-expected.");
    mark_initialized();
    _function(TYPE_FUNCTION);
    define_variable(global);
}

static void variable_declaration()
{
    uint8_t global = parse_variable("vawiabwe n-name expected.");

    if (match(Kind::T_ASSIGN))
    {
        expression();
    }
    else
    {
        emit_byte(OP_NULL);
    }

    define_variable(global);
}

static void expression_statement()
{
    expression();
    emit_byte(OP_POP);
}

static void if_statement()
{
    expression();

    int then_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);

    consume(Kind::T_BLOCK_START, "'[:' expected aftew condition.");
    begin_scope();
    block();
    end_scope();

    int else_jump = emit_jump(OP_JUMP);

    patch_jump(then_jump);
    emit_byte(OP_POP);

    if (match(Kind::T_ELSE))
    {
        consume(Kind::T_BLOCK_START, "'[:' expected aftew condition.");
        begin_scope();
        block();
        end_scope();
    }

    patch_jump(else_jump);
}

static void loop_statement()
{
    int loop_start = current_chunk()->ccount();

    expression();

    int exit_jump = emit_jump(OP_JUMP_IF_TRUE);
    emit_byte(OP_POP);

    consume(Kind::T_BLOCK_START, "'[:' expected aftew condition.");
    begin_scope();
    block();
    end_scope();

    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(OP_POP);
}

static void out_statement()
{
    if (current->ftype() == TYPE_SCRIPT)
    {
        error("iwwgaw out statement.");
    }

    if (check(Kind::T_OUT_END))
    {
        emit_return();
    }
    else
    {
        expression();
        consume(Kind::T_OUT_END, "'>>' expected aftew out.");
        emit_byte(OP_OUT);
    }
}

static void print_statement()
{
    expression();
    emit_byte(OP_PRINT);

    if (match(Kind::T_COMMA))
    {
        print_statement();
        return;
    }

    consume(Kind::T_OUT_END, "'>>' expected aftew expwession.");
}

static void read_statement()
{
    switch (parser.current.kind())
    {
        case Kind::T_READ_STRING: emit_byte(OP_READ_STRING); break;
        case Kind::T_READ_NUMBER: emit_byte(OP_READ_NUMBER); break;
        case Kind::T_READ_CHAR:   emit_byte(OP_READ_CHAR);   break;

        default: error("inpwt twype expected a-aftew wead statwement."); break;
    }

    advance();
    consume(Kind::T_IDENTIFIER, "vawiabwe n-name e-expected aftew wead s-statement.");
    named_variable(parser.previous, true, true);

    consume(Kind::T_READ_END, "'<<' expected aftew expwession.");
}

static void synchronize()
{
    parser.panic_mode = false;

    while (parser.current.kind() != Kind::T_EOF)
    {
        switch (parser.current.kind())
        {
            case Kind::T_VAR: case Kind::T_IF: case Kind::T_LOOP: case Kind::T_PRINT: case Kind::T_READ:
                return;

            default:;
        }

        advance();
    }
}

static void declaration()
{
    if (match(Kind::T_FUN))
    {
        function_declaration();
    }
    else if (match(Kind::T_VAR))
    {
        variable_declaration();
    }
    else
    {
        statement();
    }

    if (parser.panic_mode) synchronize();
}

static void statement()
{
    if (match(Kind::T_PRINT))
    {
        print_statement();
    }
    else if (match(Kind::T_READ))
    {
        read_statement();
    }
    else if (match(Kind::T_IF))
    {
        if_statement();
    }
    else if (match(Kind::T_LOOP))
    {
        loop_statement();
    }
    else if (match(Kind::T_OUT))
    {
        out_statement();
    }
    else if (match(Kind::T_LEFT_SQB))
    {
        begin_scope();
        block();
        end_scope();
    }
    else
    {
        expression_statement();
    }
}

Function * compile(const char * source)
{
    init_scanner(source);
    Compiler compiler;
    init_compiler(&compiler, TYPE_SCRIPT);

    parser.had_error = false;
    parser.panic_mode = false;

    advance();

    while (!match(Kind::T_EOF))
    {
        declaration();
    }

    Function * _function = end_compiler();
    return parser.had_error ? NULL : _function;
}
