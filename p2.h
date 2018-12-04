#include <stdio.h>
#include "getword.h"

#define MAXITEM 100 /* max number of words per line */

extern int special_flag;

int parse();
void my_handler(int signum);

