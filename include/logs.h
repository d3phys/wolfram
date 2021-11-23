#ifndef LOGS_H
#define LOGS_H

#include <stdio.h>

extern FILE *logs;

#define $(code) fprintf(logs, "%s: %s\n", __PRETTY_FUNCTION__, #code); code

#endif /* LOG_H */
