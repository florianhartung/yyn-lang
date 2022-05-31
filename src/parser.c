#include "include/parser.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

typedef struct PARSER_STATE_STRUCT {
    list_t* tokens;
    size_t current_token_index;
    token_t* current_token;
} parser_state_t;

void parser_advance(parser_state_t* parser) {
    parser->current_token_index += 1;
    if (parser->current_token_index == parser->tokens->size) {
        fprintf(stderr, "Parser cannot read another token because the last one was already read.");
        exit(1);
    }
    parser->current_token = list_get(parser->tokens, parser->current_token_index);
}

parser_state_t* init_parser(list_t* tokens) {
    parser_state_t* parser = malloc(sizeof(parser_state_t));
    parser->tokens = tokens;
    parser->current_token_index = 0;
    parser->current_token = list_get(parser->tokens, parser->current_token_index);
    return parser;
}

void free_parser(parser_state_t* parser) {
    free_list(parser->tokens);
    free(parser);
}

void expect_token(parser_state_t* parser, int type) {
    if (parser->current_token->type != type) {
        fprintf(stderr, "Unexpected token %s, expected %s", token_to_string(parser->current_token),
                token_type_to_string(type));
        exit(1);
    }
}

void expect_keyword(parser_state_t* parser, int keyword) {
    expect_token(parser, TOKEN_KEYWORD);
    if (parser->current_token->token_data->keyword != keyword) {
        fprintf(stderr, "Unexpected token %s, expected keyword '%s'", token_to_string(parser->current_token),
                keywords[parser->current_token->token_data->keyword]);
        exit(1);
    }
}

void parser_advance_with_token(parser_state_t* parser, int type) {
    expect_token(parser, type);
    parser_advance(parser);
}

void parser_advance_with_keyword(parser_state_t* parser, int keyword) {
    expect_keyword(parser, keyword);
    parser_advance(parser);
}

ast_t* parse_statement(parser_state_t* parser) {

    ast_t* statement;
    switch (parser->current_token->type) {
        case TOKEN_KEYWORD: {
            parser_advance_with_keyword(parser, KEYWORD_HELLO);
            statement = init_ast(AST_HELLO);
            break;
        }
        default:
            fprintf(stderr, "Unknown token %s, expected a statement", token_to_string(parser->current_token));
            exit(1);
    }

    parser_advance_with_token(parser, TOKEN_NEWLINE);
    return statement;
}

ast_t* parse_compound(parser_state_t* parser) {
    parser_advance_with_token(parser, TOKEN_LBRACE);

    ast_t* compound = init_ast(AST_COMPOUND);

    while (parser->current_token->type != TOKEN_RBRACE) {
        if(parser->current_token->type == TOKEN_NEWLINE) {
            parser_advance(parser);
            continue;
        }
        ast_t* statement = parse_statement(parser);
        list_append(compound->compound_children, statement);
    }

    parser_advance_with_token(parser, TOKEN_RBRACE);

    return compound;
}

ast_t* parse_function_definition(parser_state_t* parser) {
    parser_advance_with_keyword(parser, KEYWORD_FUNCTION);

    ast_t* function_definition = init_ast(AST_FUNCTION_DEF);

    expect_token(parser, TOKEN_IDENTIFIER);
    function_definition->function_def_data->function_name = parser->current_token->token_data->identifier_name;
    parser_advance(parser);

    // TODO: parse function arguments
    parser_advance_with_token(parser, TOKEN_LPAREN);
    parser_advance_with_token(parser, TOKEN_RPAREN);

    ast_t* function_child_compound = parse_compound(parser);
    function_definition->function_def_data->child_compound = function_child_compound;
    return function_definition;
}

ast_t* parse_root_compound(parser_state_t* parser) {
    ast_t* root_compound = init_ast(AST_ROOT_COMPOUND);

    while (parser->current_token->type != TOKEN_EOF) {
        if(parser->current_token->type == TOKEN_NEWLINE) {
            parser_advance(parser);
            continue;
        }
        list_append(root_compound->compound_children, parse_function_definition(parser));
    }

    return root_compound;
}

ast_t* parser_example_ast() {
    ast_t* root = init_ast(AST_ROOT_COMPOUND);

    ast_t* fun_def_main = init_ast(AST_FUNCTION_DEF);
    fun_def_main->function_def_data->function_name = "main";

    ast_t* hello_node = init_ast(AST_HELLO);
    list_append(root->compound_children, fun_def_main);
    list_append(fun_def_main->function_def_data->child_compound->compound_children, hello_node);

    return root;
}

ast_t* parser_ast_from_tokens(list_t* tokens) {
    parser_state_t* parser = init_parser(tokens);
    ast_t* root = parse_root_compound(parser);
    free_parser(parser);

    return root;
}
