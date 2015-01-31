#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <fish-util.h>
#include "arg.h"

const int NUM_ARGS = 2;

struct {
    char *prog_name;
    int num_args;
    char *args[2];
    bool filled_bg;

} g;

static struct argp_option options[] = {
    {
        0,
        'h', // 0, //'n', // key
        0, // name
        OPTION_ARG_OPTIONAL, // flags
        0, // text
        0
    }, 
    {
        "filled_bg", // long opt (--new)
        'f', // short opt (-n)
        0, // name (for long usage message, leave as 0 for optional args)
        OPTION_ARG_OPTIONAL, // flags
        "Fill background.",
        0
    }, 
    {0}
};

/* Can quit.
 */
bool arg_args(int argc, char **argv, char *ret_args[2]) {
    g.prog_name = argv[0];

    struct argp args = {0};
    args.args_doc = "cur-page total-pages"; // text in usage msg, after args
    //args.doc = "Before options \v after options"; // help msg

    args.options = options;
    args.parser = (argp_parser_t) argp_parser;

    int arg_index;

    if(
        argp_parse(&args, argc, argv, 
            0,
            &arg_index, 
            0 // input = extra data for parsing function
            )
    ) 
        arg_usage(args);

    if (g.num_args < NUM_ARGS) 
        arg_usage(args);

    memcpy(ret_args, g.args, 2 * sizeof(char*));

    return true;

}

error_t argp_parser(int key, char* arg, struct argp_state *state) {
    if (key == 'f') {
        g.filled_bg = true;
    }
    else if (key == 'h') {
        argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
        exit(0);
    }
    else if (key == ARGP_KEY_ARG) { 
        if (state->arg_num >= NUM_ARGS) 
            argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
            //usage(state->root_argp);
        g.args[state->arg_num] = state->argv[state->next-1];
        g.num_args++;
    }
    return 0;
}

void arg_usage(struct argp args) { // exit 1
    argp_help(&args, stderr, ARGP_HELP_USAGE, g.prog_name);
    exit(1);
}


