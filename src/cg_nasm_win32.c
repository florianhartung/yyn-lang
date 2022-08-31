#pragma clang diagnostic push
#pragma ide diagnostic ignored "misc-no-recursion"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include "include/cg_nasm_win32.h"
#include "include/string_builder.h"
#include "include/util.h"

#define NUMARGS(...)  (sizeof((char*[]){__VA_ARGS__})/sizeof(char*))
#define I(instruction_list, ...) add_instruction(instruction_list, NUMARGS(__VA_ARGS__), __VA_ARGS__)

#define STR(x) #x
#define WIN_FUNCTION(name, argc) "_" STR(name) "@" STR(argc)

typedef struct NASM_FUNCTION_STRUCT {
    char* name;
    list_t* instructions; // char* list
} nasm_function_t;

typedef enum NASM_VARIABLE_TYPE_STRUCT {
    NASM_TYPE_INT,
    NASM_TYPE_STRING,
    NASM_TYPE_POINTER,
    NASM_TYPE_CHAR
} nasm_variable_type_t;

typedef struct NASM_GLOABL_VARIABLE_STRUCT {
    char* name;
    nasm_variable_type_t type;
    char* value;
    size_t size;
} nasm_global_variable_t;

typedef struct NASM_STACK_VARIABLE_STRUCT {
    nasm_variable_type_t type;
    int stack_offset;
    size_t size;
} nasm_stack_variable_t;

static const int TYPE_SIZES[] = {
        [NASM_TYPE_INT] = 4,
        [NASM_TYPE_POINTER] = 4,
        [NASM_TYPE_CHAR] = 1,
        [NASM_TYPE_STRING] = 0
};

static const int TYPE_AST2NASM[] = {
        [AST_TYPE_INT] = NASM_TYPE_INT
};

static const char* SIZES_STRINGS[] = {
        [0] = "wtf",
        [1] = "byte",
        [2] = "word",
        [3] = "wtf",
        [4] = "dword"
};


typedef struct DEFAULT_NASM_VARIABLES_STRUCT {
    nasm_global_variable_t std_out;
} default_nasm_variables_t;

static const default_nasm_variables_t DEFAULT_VARIABLES = {
        .std_out = {
                .name = "stdOut",
                .size = 4
        }
};

typedef struct COMPILED_DATA {
    list_t* functions; // nasm_function_t* list
    list_t* windows_api_functions; // char* list
    list_t* variables;
    list_t* global_initializations;
} compiled_t;

const struct WIN_FUNCTIONS_STRUCT {
    char* write_file;
    char* exit_process;
    char* get_std_handle;
} WIN_API = {
        WIN_FUNCTION(WriteFile, 20),
        WIN_FUNCTION(ExitProcess, 4),
        WIN_FUNCTION(GetStdHandle, 4)
};

static nasm_function_t* init_nasm_function(char* name) {
    nasm_function_t* nasm_function = malloc(sizeof(nasm_function_t));

    nasm_function->name = name;
    nasm_function->instructions = init_list(sizeof(char*));

    return nasm_function;
}

static void free_nasm_function(nasm_function_t* nasm_function) {
    free_list(nasm_function->instructions);
    free(nasm_function);
}


static compiled_t* init_compiled() {
    compiled_t* compiled = malloc(sizeof(compiled_t));

    compiled->functions = init_list(sizeof(nasm_function_t*));
    compiled->functions->element_free_function = (free_function_t) free_nasm_function;
    compiled->windows_api_functions = init_list(sizeof(char*));
    compiled->variables = init_list(sizeof(nasm_global_variable_t*));
    compiled->global_initializations = init_list(sizeof(char*));

    return compiled;
}


static nasm_global_variable_t* init_global_variable(int variable_type) {
    static int nasm_variable_counter = 0;

    nasm_global_variable_t* variable = calloc(1, sizeof(nasm_global_variable_t));
    variable->name = iasprintf("var%d", ++nasm_variable_counter);
    variable->size = TYPE_SIZES[variable_type];
    variable->value = 0;
    variable->type = variable_type;

    return variable;
}

static nasm_stack_variable_t* init_stack_variable(int variable_type) {
    nasm_stack_variable_t* variable = calloc(1, sizeof(nasm_stack_variable_t));
    variable->type = variable_type;
    variable->size = TYPE_SIZES[variable_type];

    return variable;
}


static void include_windows_api_function(compiled_t* compiled, char* function) {
    LIST_FOREACH(compiled->windows_api_functions, char* existing_function, {
        if (strcmp(existing_function, function) == 0) {
            return;
        }
    })
    list_append(compiled->windows_api_functions, function);
}

static inline void register_variable(compiled_t* compiled, nasm_global_variable_t* variable) {
    list_append(compiled->variables, variable);
}

static char* build_compiled(compiled_t* compiled) {
    string_builder_t* result = init_string_builder();

    string_builder_append(result, "\tglobal _main\n");
    LIST_FOREACH(compiled->windows_api_functions, char* win_api_function, {
        string_builder_append(result, "\textern ");
        string_builder_append(result, win_api_function);
        string_builder_append(result, "\n");
    })
    string_builder_append(result, "\n");

    string_builder_append(result, "section .data\n");
    LIST_FOREACH(compiled->variables, nasm_global_variable_t* variable, {
        char* variable_value = "0";
        if (variable->value != 0) {
            variable_value = variable->value;
        }

        char* variable_str;
        asprintf(&variable_str, "\t%s: times %zu db %s\n", variable->name, variable->size, variable_value);
        string_builder_append(result, variable_str);
        free(variable_str);
    })
    string_builder_append(result, "\n");


    string_builder_append(result, "section .text\n");

    LIST_FOREACH(compiled->functions, nasm_function_t* nasm_function, {
        string_builder_append(result, nasm_function->name);
        string_builder_append(result, ":\n");
        if (strcmp(nasm_function->name, "_main") == 0) {
            LIST_FOREACH(compiled->global_initializations,
            char* global_initialization,
            {
                string_builder_append(result, global_initialization);
                string_builder_append(result, "\n");
            })
        }

        LIST_FOREACH(nasm_function->instructions,
        char* instruction,
        {
            string_builder_append(result, instruction);
            string_builder_append(result, "\n");
        })
    })

    free_list(compiled->functions);
    free(compiled);

    char* result_string = string_builder_build(result);

    return result_string;
}

void add_instruction(list_t* instruction_list, int count, ...) {
    string_builder_t* final_instruction = init_string_builder();
    string_builder_append(final_instruction, "\t");

    va_list ap;
    va_start(ap, count);

    bool first = true;
    while (count-- > 0) {
        if (first) {
            first = false;
        } else {
            string_builder_append(final_instruction, "\t");
        }
        char* s = iasprintf("%-6s", va_arg(ap, char*));
        string_builder_append(final_instruction, s);
        free(s);
    }
    va_end(ap);

    list_append(instruction_list, string_builder_build(final_instruction));
}

char* predeclare_label(char* label_name) {
    static int label_counter = 0;

    return iasprintf("l%s%d", label_name, label_counter++);
}

void add_predeclared_label(list_t* instruction_list, char* predeclared_label) {
    list_append(instruction_list, iasprintf("%s:", predeclared_label));
}

char* add_label(list_t* instruction_list, char* label_name) {
    char* label_identifier = predeclare_label(label_name);
    add_predeclared_label(instruction_list, label_identifier);
    return label_identifier;
}

void code_generation_prolog(list_t* instructions) {
    I(instructions, "push", "ebp");
    I(instructions, "mov", "ebp, esp");
}

void code_generation_epilog(list_t* instructions) {
    I(instructions, "mov", "esp, ebp");
    I(instructions, "pop" , "ebp");
}

void code_generation_ast_exit(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_exit) {
    include_windows_api_function(compiled, WIN_API.exit_process);

    I(parent_instructions, "push", iasprintf("%d", ast_exit->exit_code));
    I(parent_instructions, "call", WIN_API.exit_process);
}

void code_generation_ast_hello(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_hello) {
    const char* hello_world_str = "Hello World!";

    nasm_global_variable_t* hello_world_constant = init_global_variable(NASM_TYPE_STRING);
    hello_world_constant->size = strlen(hello_world_str) + 2ULL;
    hello_world_constant->value = iasprintf("\"%s\",10,0", hello_world_str);

    list_append(compiled->variables, hello_world_constant);

    include_windows_api_function(compiled, WIN_API.write_file);

    I(parent_instructions, "push", "0");
    I(parent_instructions, "push", "0");
    I(parent_instructions, "push", iasprintf("%d", hello_world_constant->size));
    I(parent_instructions, "push", hello_world_constant->name);
    I(parent_instructions, "push",
      iasprintf("%s [%s]", SIZES_STRINGS[DEFAULT_VARIABLES.std_out.size], DEFAULT_VARIABLES.std_out.name));
    I(parent_instructions, "call", WIN_API.write_file);
}

static void code_generation_ast_expression(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_expression, char* result_register) {
    ast_t* child = ast_expression->expression_child;
    switch(child->type) {
        case AST_INTEGER: {
            I(parent_instructions, "mov", iasprintf("%s, %d", result_register, child->integer_value));
            break;
        }
        case AST_VARIABLE_ACCESS: {
            nasm_stack_variable_t* var = child->variable_data->backend_variable;
            I(parent_instructions, "mov", iasprintf("%s, [ebp-%d]", result_register, var->stack_offset));
            break;
        }
        default:
            fprintf(stderr, "Unknown expression_child ast int type %d", child->type);
            exit(1);
    }
}

void code_generation_ast_print_char(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_print_char) {
    include_windows_api_function(compiled, WIN_API.write_file);

    code_generation_ast_expression(compiled, parent_instructions, ast_print_char->expression_child, "eax");

    // prolog & allocate 1 byte buffer
    code_generation_prolog(parent_instructions);

    I(parent_instructions, "sub", "esp, 4");

    I(parent_instructions, "mov", "[ebp-4], al");
    I(parent_instructions, "push", "0");
    I(parent_instructions, "push", "0");
    I(parent_instructions, "push", "1");
    I(parent_instructions, "lea ", "eax, [ebp-4]");
    I(parent_instructions, "push", "eax");
    I(parent_instructions, "push", iasprintf("%s [%s]", SIZES_STRINGS[DEFAULT_VARIABLES.std_out.size], DEFAULT_VARIABLES.std_out.name));
    I(parent_instructions, "call", WIN_API.write_file);

    // epilog
    code_generation_epilog(parent_instructions);
}


void code_generation_ast_variable(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_variable) {
    nasm_stack_variable_t* nasm_variable = ast_variable->variable_data->backend_variable;
    if (!nasm_variable) {
        fprintf(stderr, "Could not link frontend and backend variable %s", ast_variable->variable_data->name);
        exit(1);
    }

    I(parent_instructions, "mov ", iasprintf("%s [ebp-%d], %d", SIZES_STRINGS[nasm_variable->size], nasm_variable->stack_offset,
                                             ast_variable->variable_data->value));
}

static void code_generation_ast_assignment(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_assignment) {
    nasm_stack_variable_t* variable = ast_assignment->assignment_data->variable->backend_variable;
    switch (ast_assignment->type) {
        case AST_ASSIGNMENT_ADD: {
            code_generation_ast_expression(compiled, parent_instructions, ast_assignment->assignment_data->expression, "eax");
            I(parent_instructions, "add", iasprintf("eax, [ebp-%d]", variable->stack_offset));
            I(parent_instructions, "mov", iasprintf("[ebp-%d], eax", variable->stack_offset));
            break;
        }
        case AST_ASSIGNMENT: {
            code_generation_ast_expression(compiled, parent_instructions, ast_assignment->assignment_data->expression, "eax");
            I(parent_instructions, "mov", iasprintf("[ebp-%d], eax", variable->stack_offset));
            break;
        }
        default:
            fprintf(stderr, "Unknown ast assignment of int type %d", ast_assignment->type);
            exit(1);
    }
}

void code_generation_ast_function_call(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_variable) {
    I(parent_instructions, "call", ast_variable->function_identifier);
}

void code_generation_ast_compound(compiled_t* compiled, list_t* instructions, ast_t* compound);

void code_generation_ast_while_loop(compiled_t* compiled, list_t* parent_instructions, ast_t* ast_while_loop) {
    bool_expression_data_t* bool_expression_data = ast_while_loop->while_loop_data->expression_child->expression_child->bool_expression_data;
    char* before_while_check = predeclare_label("while");
    char* after_loop = predeclare_label("whileend");
    char* compound_start = predeclare_label("whilestart");

    add_predeclared_label(parent_instructions, before_while_check);
    code_generation_ast_expression(compiled, parent_instructions, bool_expression_data->left, "eax");
    code_generation_ast_expression(compiled, parent_instructions, bool_expression_data->right, "ebx");
    I(parent_instructions, "cmp", "eax, ebx");

    switch (bool_expression_data->operand) {
        case BOOL_LESS_THAN:
            I(parent_instructions, "jl", compound_start);
            break;
        default:
            fprintf(stderr, "Unknown ast bool expression operand %d", bool_expression_data->operand);
            exit(1);
    }

    I(parent_instructions, "jmp", after_loop);

    add_predeclared_label(parent_instructions, compound_start);

    code_generation_ast_compound(compiled, parent_instructions, ast_while_loop->while_loop_data->child_compound);

    I(parent_instructions, "jmp", before_while_check);

    add_predeclared_label(parent_instructions, after_loop);
}

void code_generation_ast_compound(compiled_t* compiled, list_t* instructions, ast_t* compound) {
    LIST_FOREACH(compound->compound_data->children, ast_t* child, {
        switch (child->type) {
            case AST_HELLO:
                code_generation_ast_hello(compiled, instructions, child);
                break;
            case AST_EXIT:
                code_generation_ast_exit(compiled, instructions, child);
                break;
            case AST_PRINT_CHAR:
                code_generation_ast_print_char(compiled, instructions, child);
                break;
            case AST_COMPOUND: {
                code_generation_ast_compound(compiled, instructions, child);
                break;
            }
            case AST_VARIABLE:
                code_generation_ast_variable(compiled, instructions, child);
                break;
            case AST_FUNCTION_CALL:
                code_generation_ast_function_call(compiled, instructions, child);
                break;
            case AST_ASSIGNMENT:
            case AST_ASSIGNMENT_ADD:
                code_generation_ast_assignment(compiled, instructions, child);
                break;
            case AST_WHILE_LOOP:
                code_generation_ast_while_loop(compiled, instructions, child);
                break;
            default:
                fprintf(stderr, "Unexpected ast child_compound child of type %d", child->type);
                exit(1);
        }
    })
}

void variables_in_context(list_t* variables, variable_context_t* variable_context) {
    int total_bytes = 0;

    LIST_FOREACH(variable_context->variables, variable_t* var, {
        list_append(variables, var);
    })
    LIST_FOREACH(variable_context->children, variable_context_t* child_variable_context, {
        variables_in_context(variables, child_variable_context);
    })
}

void code_generation_ast_function(compiled_t* compiled, ast_t* ast_function) {
    char* main_function_name = generate_function_identifier("main");

    if (strcmp(ast_function->function_def_data->identifier, main_function_name) == 0) {
        ast_function->function_def_data->identifier = "_main";
    }
    // Check if ast_function already exists
    LIST_FOREACH(compiled->functions, nasm_function_t* existing_function, {
        if (strcmp(existing_function->name, ast_function->function_def_data->identifier) == 0) {
            fprintf(stderr, "Function %s already exists", existing_function->name);
            exit(1);
        }
    })
    nasm_function_t* function = init_nasm_function(ast_function->function_def_data->identifier);

    code_generation_prolog(function->instructions);

    // TODO: Add arguments to stack
    // ast_function stack variable allocation
    variable_context_t* function_variable_context = ast_function->function_def_data->child_compound->compound_data->variable_context;
    list_t* declared_variables = init_list(sizeof(variable_t*));
    variables_in_context(declared_variables, function_variable_context);
    int required_bytes_on_stack = 0;
    LIST_FOREACH(declared_variables, variable_t* var, {
        nasm_stack_variable_t* nasm_variable = init_stack_variable(TYPE_AST2NASM[var->type]);
        required_bytes_on_stack += nasm_variable->size;
        nasm_variable->stack_offset = required_bytes_on_stack;
        var->backend_variable = nasm_variable;
    })

    if(required_bytes_on_stack) {
        I(function->instructions, "sub", iasprintf("esp, %d", required_bytes_on_stack));
    }


    code_generation_ast_compound(compiled, function->instructions, ast_function->function_def_data->child_compound);

    code_generation_epilog(function->instructions);
    I(function->instructions, "ret");

    list_append(compiled->functions, function);
}

void code_generation_ast_root(compiled_t* compiled, ast_t* root) {
    LIST_FOREACH(root->compound_data->children, ast_t* function, {
        assert(function->type == AST_FUNCTION_DEF && "Ast root supports only ast_function definitions as its children");

        code_generation_ast_function(compiled, function);
    })
}

void code_generation_load_std_out(compiled_t* compiled) {
    include_windows_api_function(compiled, WIN_API.get_std_handle);

    I(compiled->global_initializations, "push", "-11");
    I(compiled->global_initializations, "call", WIN_API.get_std_handle);
    I(compiled->global_initializations, "mov ", iasprintf("[%s], eax", DEFAULT_VARIABLES.std_out.name));
}

void code_generation_global_initializations(compiled_t* compiled) {
    code_generation_load_std_out(compiled);
}

char* code_generation_nasm_win32(ast_t* root) {
    compiled_t* compiled = init_compiled();

    assert(root->type == AST_ROOT_COMPOUND && "Ast root must be of type AST_ROOT_COMPOUND");

    register_variable(compiled, (void*) &(DEFAULT_VARIABLES.std_out));

    code_generation_global_initializations(compiled);

    code_generation_ast_root(compiled, root);

    free_ast(root);
    return build_compiled(compiled);
}
#pragma clang diagnostic pop