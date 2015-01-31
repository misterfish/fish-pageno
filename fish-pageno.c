#undef DEBUG 

#define _GNU_SOURCE 

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

#include <fish-util.h>

#include "config.h"

#ifndef CONFIG_NUM_FRAMES_NOT_FILLED
#error CONFIG_NUM_FRAMES_NOT_FILLED undefined.
#endif

#ifndef CONFIG_NUM_FRAMES_FILLED
#error CONFIG_NUM_FRAMES_FILLED undefined.
#endif

#include "fish-pageno.h"
#include "arg.h"
#include "const.h"

static struct {
    Aosd *aosd;
    int page_cnt;
    int cur_page;
    //FT_Face font_face;
    cairo_font_face_t *cairo_face;

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

// OPTION_ARG_OPTIONAL means the value is optional (not the arg)

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

    init(argc, argv);

    f_sig(1, (void*) sig_handler);

    char *path1 = "/usr/share/fonts/truetype/Andika-R.ttf";
    // generic way??
    char *path2 = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
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

/* Can quit.
 */
void init(int argc, char **argv) {
    autoflush();

    //char ***ret_args = (char***) &g.args;

    /* Can quit.
     */
    if (!arg_args(argc, argv, g.args)) {
        piep;
        exit(1);
    }
        
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

