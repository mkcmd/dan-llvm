#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>

bool clang_compile(const char *args[], size_t n_args, char **diags_out);

#ifdef __cplusplus
}
#endif
