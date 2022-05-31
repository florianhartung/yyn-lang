#pragma once

#include "list.h"

struct AST_STRUCT;

typedef struct FUNCTION_DEF_DATA {
    char* function_name;
    //TODO: function args
    struct AST_STRUCT* child_compound;
} function_def_data_t;

typedef struct AST_STRUCT {
   enum {
       AST_FUNCTION_DEF,
       AST_COMPOUND,
       AST_ROOT_COMPOUND,
       AST_HELLO
   } type;

   union {
       function_def_data_t* function_def_data;
       list_t* compound_children;
   };
} ast_t;

ast_t* init_ast(int type);

void ast_print(ast_t* ast);

void free_ast(ast_t* ast);