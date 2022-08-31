#pragma once

#include "list.h"

struct AST_STRUCT;

typedef struct FUNCTION_DEF_DATA_STRUCT {
    char* identifier;
    //TODO: function args
    struct AST_STRUCT* child_compound;
} function_def_data_t;

typedef enum VARIABLE_TYPE_STRUCT {
    AST_TYPE_INT
} variable_type_t;

typedef struct VARIABLE_DATA_STRUCT {
    char* name;
    int value;
    int declaration_index;
    variable_type_t type;
    void* backend_variable;
} variable_t;

typedef struct VARIABLE_CONTEXT_STRUCT {
    list_t* variables;
    struct VARIABLE_CONTEXT_STRUCT* parent;
    list_t* children;
} variable_context_t;

typedef struct COMPOUND_DATA_STRUCT {
    list_t* children;
    variable_context_t* variable_context;
} compound_data_t;

typedef struct ROOT_COMPOUND_DATA_STRUCT {
    compound_data_t compound_data;
} root_compound_data_t;

struct AST_STRUCT;

typedef struct ASSIGNMENT_DATA_STRUCT {
    variable_t* variable;
    struct AST_STRUCT* expression;
} assignment_data_t;

typedef struct WHILE_LOOP_DATA_STRUCT {
    struct AST_STRUCT* expression_child;
    struct AST_STRUCT* child_compound;
} while_loop_data_t;

typedef enum BOOL_EXPRESSION_OPERAND_STRUCT {
    BOOL_LESS_THAN
} bool_expression_operand_t;

typedef struct BOOL_EXPRESSION_DATA_STRUCT {
    struct AST_STRUCT* left;
    bool_expression_operand_t operand;
    struct AST_STRUCT* right;
} bool_expression_data_t;

typedef struct AST_STRUCT {
    enum {
        AST_FUNCTION_DEF,
        AST_COMPOUND,
        AST_ROOT_COMPOUND,
        AST_HELLO,
        AST_EXIT,
        AST_PRINT_CHAR,
        AST_VARIABLE,
        AST_FUNCTION_CALL,
        AST_ASSIGNMENT_ADD,
        AST_ASSIGNMENT,
        AST_EXPRESSION,
        AST_INTEGER,
        AST_VARIABLE_ACCESS,
        AST_WHILE_LOOP,
        AST_BOOL_EXPRESSION
    } type;

    union {
        function_def_data_t* function_def_data;
        root_compound_data_t* root_compound_data;
        compound_data_t* compound_data;
        int exit_code;
        int character_code;
        variable_t* variable_data;
        char* function_identifier;
        assignment_data_t* assignment_data;
        struct AST_STRUCT* expression_child;
        int integer_value;
        while_loop_data_t* while_loop_data;
        bool_expression_data_t* bool_expression_data;
    };
} ast_t;

variable_context_t* init_variable_context(variable_context_t* parent_variable_context);

ast_t* init_ast(int type);

void ast_print(ast_t* ast);

void free_ast(ast_t* ast);

char* generate_function_identifier(char* function_name);