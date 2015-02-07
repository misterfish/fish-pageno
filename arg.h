#include <argp.h>

struct args {
    bool filled_bg;
    int cur_page;
    int total_pages;
    int stay_alive_secs;
};

bool arg_args(int argc, char **argv, struct args *args);
void arg_usage(struct argp args);

static 
error_t argp_parser(int key, char* arg, struct argp_state *state);

