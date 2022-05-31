#pragma once

#include "ast.h"
#include "token.h"
#include "list.h"


ast_t* parser_ast_from_tokens(list_t* tokens);