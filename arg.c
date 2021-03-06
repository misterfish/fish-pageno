#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <fish-util.h>
#include "config.h"
#include "const.h"
#include "arg.h"

const int NUM_ARGS = 2;

struct {
    char *prog_name;
    int num_args;

    int cur_page;
    int total_pages;
    bool filled_bg;
    int stay_alive_secs;
} g;

/* OPTION_ARG_OPTIONAL means the value is optional (not the arg)
 */


/* Can quit.
 */
bool arg_args(int argc, char **argv, struct args *ret_args) {

    char *stays1 = "Stay alive secs (default=%d)";
    char *stays = malloc(strlen(stays1) - 2 + f_int_length(STAY_ALIVE_SECS) + 1);
    if (!stays) 
        ierr_perr("");
    sprintf(stays, stays1, STAY_ALIVE_SECS);

    g.stay_alive_secs = STAY_ALIVE_SECS;

    struct argp_option options[] = {
        {
            0,
            'h', // 0, //'n', // key
            0, // name
            OPTION_ARG_OPTIONAL, // flags
            0, // text
            0
        }, 
        {
            "filled-bg", // long opt (--new)
            'f', // short opt (-n)
            0, // name (for long usage message, leave as 0 for optional args)
            OPTION_ARG_OPTIONAL, // flags
            "Fill background.",
            0
        },
        {
            "stay-alive-secs",
            's',
            /* for after = in usage,
             * also signals that this is an arg with a value.
             */
            "secs", 
            0,
            stays,
            0
        },
        {0}
    };

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

    ret_args->filled_bg = g.filled_bg;
    ret_args->stay_alive_secs = g.stay_alive_secs;
    ret_args->cur_page = g.cur_page;
    ret_args->total_pages = g.total_pages;

    return true;

}

error_t argp_parser(int key, char *arg, struct argp_state *state) {
    if (key == 'f') {
        g.filled_bg = true;
    }
    else if (key == 's') {
        g.stay_alive_secs = atoi(arg);
    }
    else if (key == 'h') {
        argp_state_help(state, stdout, ARGP_HELP_STD_HELP);
        exit(0);
    }
    else if (key == ARGP_KEY_ARG) { 
        int arg_num = state->arg_num;
        if (arg_num >= NUM_ARGS) 
            argp_state_help(state, stdout, ARGP_HELP_STD_HELP);

        char *arg = state->argv[state->next-1];
        if (arg_num == 0) {
            g.cur_page = atoi(arg);
            if (! g.cur_page) 
                return 1; 
        }
        else if (arg_num == 1) {
            g.total_pages = atoi(arg);
            if (! g.total_pages) 
                return 1;
        }
        g.num_args++;
    }
    return 0;
}

void arg_usage(struct argp args) { // exit 1
    argp_help(&args, stderr, ARGP_HELP_USAGE, g.prog_name);
    exit(1);
}


