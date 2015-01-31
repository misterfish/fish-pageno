#undef DEBUG 

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>

#include <string.h> // strdup

#include <aosd.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <cairo-ft.h>

#include <glib.h>

#include <argp.h>

#include <fish-util.h>
#include "config.h"

void init(int, char**);
void show();
void hide();
void update();

#ifndef CONFIG_NUM_FRAMES_NOT_FILLED
#error CONFIG_NUM_FRAMES_NOT_FILLED undefined.
#endif

#ifndef CONFIG_NUM_FRAMES_FILLED
#error CONFIG_NUM_FRAMES_FILLED undefined.
#endif

static const int NUM_FRAMES_FILLED = CONFIG_NUM_FRAMES_FILLED;
static const int NUM_FRAMES_NOT_FILLED = CONFIG_NUM_FRAMES_NOT_FILLED;

static const float FONT_FACTOR = .35;

static const int WIDTH_INIT = 400;
static const int HEIGHT_INIT = 400;
static const int WIDTH = 50; // final
static const int HEIGHT = 50; // final
static const int LEFT = 0;
static const int TOP = 0;

static const int STAY_ALIVE_SECS = 2;

static const float LINE_X1_PERC = .2;
static const float LINE_Y1_PERC = .8;
static const float LINE_X2_PERC = .8;
static const float LINE_Y2_PERC = .2;

static const float NUMER_X_PERC = .23;
static const float NUMER_Y_PERC = .4;
static const float DENOM_X_PERC = .55;
static const float DENOM_Y_PERC = .75;

//float LINE_SLOPE = .25 / (LINE_X2_PERC - LINE_X1_PERC);
//static const float LINE_SLOPE = .2 / .4;

static const float COLOR_NOT_FILLED[] = { .3, .2, .05 };
static const float COLOR_FILLED[] = { .7, .7, .4 };

static const float COLOR_BG[] = { .1, .2, .4 };

static const float ALPHA_INIT = .1;

//static char* USAGE;

static const int NUM_ARGS = 2;

static struct {
    Aosd *aosd;
    int page_cnt;
    int cur_page;
    //FT_Face font_face;
    cairo_font_face_t *cairo_face;

    int num_args;
    char *args[2];

    bool filled_bg;

    int num_frames;
} g;

static struct {
    int shown;
    float alpha;
    float width;
    float height;
    int left;
    int top;
    float x1;
    float y1;
    float stx;
    float sty;

    float stx_max;
    float y2_max;

    float sbx;
    float sby;
    float x2;
    float y2;
    float font_size;
} s;

static void usage(struct argp args) { // exit 1
    argp_help(&args, stderr, ARGP_HELP_USAGE, "allen");
    exit(1);
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

// OPTION_ARG_OPTIONAL means the value is optional (not the arg)

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

void renderer(cairo_t* cr, void* user_data) {

    const float *color;

    if (g.filled_bg) {
        //cairo_rectangle(cr, 0, 0, s.stx + 10, s.y2 + 10);
        //cairo_rectangle(cr, 0, 0, s.stx_max + MARGIN_RIGHT, s.y2_max + MARGIN_BOTTOM);
        cairo_rectangle(cr, 0, 0, s.width, s.height);
        cairo_set_source_rgba(cr, COLOR_BG[0], COLOR_BG[1], COLOR_BG[2], s.alpha);
        cairo_fill(cr);

        color = COLOR_FILLED;
    }
    else {
        color = COLOR_NOT_FILLED;
    }

    cairo_set_line_width(cr, 1);
    cairo_set_source_rgba (cr, color[0], color[1], color[2], s.alpha);

    cairo_set_font_face(cr, g.cairo_face);
    cairo_set_font_size(cr, s.font_size);
    cairo_move_to(cr, s.x1, s.y1);

    _();

    spr("%d", g.cur_page);
    cairo_show_text(cr, _s);

    cairo_move_to(cr, s.stx, s.sty);
    cairo_line_to(cr, s.sbx, s.sby);
    cairo_stroke(cr);

    cairo_move_to(cr, s.x2, s.y2);

    spr("%d", g.page_cnt);
    cairo_show_text(cr, _t);
}

// Doesn't return to what it was doing.
void *sig_handler(int signum) {
    if (signum == 1) {
        if (s.shown) {
            hide();
            exit(0);
        }
    }
    else {
        err ("Got unexpected signal %d", CY_(spr_("%s", signum)));
    }
}

cairo_font_face_t* get_font(char* path) {
    FT_Face face;
    FT_Library library;

    if (FT_Init_FreeType( &library )) {
        err ("Can't make library.");
    }

    if ( FT_New_Face(
            library, 
            path,
            // face-index
            0,
            &face
        )
    ) {
        warn ("Can't get font (path: %s).", CY_(path));
        return NULL;
    }

    cairo_font_face_t *cairo_face = 
        cairo_ft_font_face_create_for_ft_face (face, FT_LOAD_TARGET_NORMAL);

    return cairo_face;

}

int main(int argc, char **argv) {

    //USAGE = malloc( 100 * sizeof(char));
    //sprintf(USAGE, "Usage: %s [-f to fill bg] cur_page total_pages", argv[0]);

    init(argc, argv);

    f_sig(1, (void*) sig_handler);

    char* path1 = "/usr/share/fonts/truetype/Andika-R.ttf";
    // generic way??
    char* path2 = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
    cairo_font_face_t *cairo_face;
    cairo_face = get_font(path1);
    if (! cairo_face) {
        cairo_face = get_font(path2);
    }
    if (! cairo_face) {
        err("Can't get any fonts.");
    }
    g.cairo_face = cairo_face;

    // HUP
    show();

    s.shown = 1;

    f_sig(1, NULL);

    hide();
    exit;
}

void update_boundaries() {
    aosd_set_position_with_offset(g.aosd, COORDINATE_MINIMUM, COORDINATE_MINIMUM, s.width, s.height, s.left, s.top);
}

// can die
void init(int argc, char **argv) {
    autoflush();

    /* Can also init with: static struct argp args */
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
        usage(args);

    if (g.num_args < NUM_ARGS) 
        usage(args);

    int cur = stoi(g.args[0]);
    int total = stoi(g.args[1]);
    g.page_cnt = total;
    g.cur_page = cur;

    g.num_frames = g.filled_bg ? NUM_FRAMES_FILLED : NUM_FRAMES_NOT_FILLED;

    s.shown = 0;
    s.alpha = ALPHA_INIT;
    s.width = WIDTH_INIT;
    s.height = HEIGHT_INIT;
    s.left = LEFT;
    s.top = TOP;

    Aosd *aosd = aosd_new();
    g.aosd = aosd;

    aosd_set_transparency(aosd, TRANSPARENCY_COMPOSITE);

    aosd_set_renderer(aosd, (AosdRenderer) renderer, NULL);

    update_boundaries();

    aosd_show(aosd);
}

void show() {
    float alpha_fact = 1.0 * (1 - ALPHA_INIT) / g.num_frames;
    float scale_fact = 1.0 * (WIDTH - WIDTH_INIT) / g.num_frames;
    
    s.stx_max = .64 * WIDTH; 
    s.y2_max = .5 * WIDTH;

    for (int i = 0; i < g.num_frames; i++) {
        s.width += scale_fact;
        s.height += scale_fact;

        /* The line.
         */
        s.stx = s.width * LINE_X2_PERC;
        s.sbx = s.width * LINE_X1_PERC;
        s.sby = s.height * LINE_Y1_PERC;
        s.sty = s.height * LINE_Y2_PERC;

        s.x1 = s.width * NUMER_X_PERC;
        s.y1 = s.width * NUMER_Y_PERC;
        s.x2 = s.width * DENOM_X_PERC;
        s.y2 = s.width * DENOM_Y_PERC;

        s.font_size = s.width * FONT_FACTOR;

        update_boundaries();
        s.alpha += alpha_fact;

        update();
    }
    sleep(STAY_ALIVE_SECS);
}

void hide() {
}

void update() {
    aosd_render(g.aosd);
    aosd_loop_once(g.aosd);
}

