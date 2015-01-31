#include <argp.h>

bool arg_args(int argc, char **argv, char *ret_args[2]);
void arg_usage(struct argp args);

static 
error_t argp_parser(int key, char* arg, struct argp_state *state);
