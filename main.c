#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "dynamic_array.h"
#include "util.h"

typedef enum {
    ADD, SUB, MUL, DIV, POW,
    LPAREN, RPAREN, NUM, WHITESPACE,
} TokenType;

typedef struct {
    size_t pos;
    TokenType type;
    int value;
} Token;

static void tokenize(void);
static void print_tokens(void);
static void parse_tokens(void);
static void print_tokens_in_rpn(void);
static int evaluate_tokens_in_rpn(void);

static inline bool is_digit(char c) { return BETWEEN(c, '0', '9'); }

const char *tokentypenames[] = {
    "+", "-", "*", "/", "**",
    "(", ")", "NUM", "WHITESPACE",
};

const uint operator_precedence[] = {
    [ADD] = 0,
    [SUB] = 0,
    [MUL] = 1,
    [DIV] = 1,
    [POW] = 2,
    [LPAREN] = 3,
    [RPAREN] = 3,
};

Token *tokens;
size_t *parsed_token_indices;
size_t parsed_token_indices_len;
char *input;

int
main(int argc, char *argv[])
{
    if (argc != 2)
        die("usage: %s <expr>", argv[0]);

    input = argv[1];

    da_create(tokens, sizeof(Token), 16);
    tokenize();

    print_tokens();

    parsed_token_indices = ecalloc(da_len(tokens), sizeof(*parsed_token_indices));

    parse_tokens();

    print_tokens_in_rpn();

    int res;
    res = evaluate_tokens_in_rpn();
    printf("result: %i\n", res);

    free(parsed_token_indices);
    da_destroy(tokens);
    return 0;
}

void
tokenize(void)
{
    char numstr[32], *str;
    str = input;

    for (size_t i = 0; i < strlen(str); i++) {
        Token t;
        size_t numstr_i;

        t.pos = i;
        numstr_i = 0;

        switch (str[i]) {
        case '+': t.type = ADD; break;
        case '-': t.type = SUB; break;
        case '*':
            if ('*' == str[i+1]) { t.type = POW; i++; }
            else { t.type = MUL; }
            break;
        case '/': t.type = DIV; break;
        case '(': t.type = LPAREN; break;
        case ')': t.type = RPAREN; break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            while (is_digit(str[i]) && numstr_i < sizeof(numstr))
                numstr[numstr_i++] = str[i++];
            if (numstr_i == sizeof(numstr))
                die("E: Why do you have a number longer than 32 characters?");
            numstr[numstr_i] = '\0';
            t.type = NUM;
            t.value = atoi(numstr);
            i--;
            break;
        case ' ': t.type = WHITESPACE; break;
        default:
            die("E: unknown character `%c`?", str[i]);
            break;
        }
        if (WHITESPACE != t.type)
            da_append(tokens, &t);
    }
}

void
print_tokens(void)
{
    size_t len;
    len = da_len(tokens);

    printf("tokens: ");
    for (size_t i = 0; i < len; i++) {
        Token *t;
        t = &tokens[i];
        if (t->type == NUM) printf("%i ", t->value);
        else if (t->type == WHITESPACE) /* nop */;
        else printf("%s ", tokentypenames[t->type]);
    }
    fputc('\n', stdout);
}

void
parse_tokens(void)
{
    size_t *stack, *output;
    size_t si, oi, len;

    len = da_len(tokens);
    stack = ecalloc(len, sizeof(*stack));
    output = parsed_token_indices;

    si = oi = 0;

    for (size_t i = 0; i < da_len(tokens); i++) {
        TokenType type, stack_head_type;
        size_t o1, o2;
        uint op1, op2;

        type = tokens[i].type;

        switch (type) {
        // number
        case NUM:
            output[oi++] = i;
            break;
        // operator
        case ADD:
        case SUB:
        case MUL:
        case DIV:
        case POW:
            op1 = operator_precedence[type];
            while ((si > 0) // operator on stack
                    && (tokens[stack[si-1]].type != LPAREN) // stack head not left parenthesis
                    && (operator_precedence[tokens[stack[si-1]].type] >= op1)) // stack head precedence greater or eq than op1
                output[oi++] = stack[--si];
            stack[si++] = i;
            break;
        // left parenthesis
        case LPAREN:
            stack[si++] = i;
            break;
        // right parenthesis
        case RPAREN:
            while ((si > 0) && (tokens[stack[si-1]].type != LPAREN))
                output[oi++] = stack[--si];
            if ((si > 0) && (tokens[stack[si-1]].type == LPAREN))
                si--;
            if ((si > 0) && BETWEEN(tokens[stack[si-1]].type, ADD, POW))
                output[oi++] = stack[--si];
            break;
        default:
            die("E: unknown token `%u`", type);
            break;
        }
    }

    // pop remaining items
    while (si)
        output[oi++] = stack[--si];

    parsed_token_indices_len = oi;

    // cleanup
    free(stack);
}

void
print_tokens_in_rpn(void)
{
    printf("rpn: ");
    for (size_t i = 0; i < parsed_token_indices_len; i++) {
        Token *t;
        t = &tokens[parsed_token_indices[i]];
        if (t->type == NUM) printf("%i ", t->value);
        else if (t->type == WHITESPACE) /* nop */;
        else printf("%s ", tokentypenames[t->type]);
    }
    fputc('\n', stdout);
}

int
evaluate_tokens_in_rpn(void)
{
    int *stack, out;
    size_t si;

    stack = ecalloc(parsed_token_indices_len, sizeof(*stack));

    si = 0;
    for (int i = 0; i < parsed_token_indices_len; i++) {
        Token *t;
        t = &tokens[parsed_token_indices[i]];
        if (t->type == ADD || t->type == SUB || t->type == MUL || t->type == DIV || t->type == POW) {
            int a, b;
            a = stack[--si];
            b = stack[--si];
            if (t->type == ADD)
                stack[si++] = b + a;
            else if (t->type == SUB)
                stack[si++] = b - a;
            else if (t->type == MUL)
                stack[si++] = b * a;
            else if (t->type == DIV)
                stack[si++] = b / a;
            else if (t->type == POW)
                stack[si++] = pow(b, a);
        }
        else stack[si++] = t->value;
    }

    if (si > 1)
        die("E: Too many items in stack after RPN evaluation algorithm");

    out = stack[0];

    free(stack);

    return out;
}
