#ifndef ANVIL_H
#define ANVIL_H

#include "frame.h"
#include "../../COMMON/bytecode.h"

int anvil_compile(ASTNode *root, const char *output_filename);
int anvil_generate_code(ASTNode *root, const char *output_filename);

#endif
