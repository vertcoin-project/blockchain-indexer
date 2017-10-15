#ifndef LIBBASE58_H
#define LIBBASE58_H

#include <stdbool.h>
#include <stddef.h>

bool b58enc(char *b58, size_t *b58sz, const unsigned char *bin, size_t binsz);
#endif
