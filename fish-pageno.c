#undef DEBUG 

#define _GNU_SOURCE 

//#define TESTING

#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <assert.h>

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

#include "arg.h"
#include "const.h"
#include "global.h"
#include "draw.h"
#include "fish-pageno.h"

/* global.h
 */

//extern struct g;

/* draw.h
 */
//extern struct s;

void sig_handler(int signum) {
    /* sighup
     */
    if (signum != 1) 
        ierr;

    if (s.shown) {
        if (!draw_hide())
            piep;
        exit(0);
    }
}

cairo_font_face_t *get_font(char *path) {

    /* typedef struct FT_FaceRec_* FT_Face */
    FT_Face face;
    FT_Library library;

    tryft(FT_Init_FreeType(&library), "Can't make library"); 
    if (!tryftok)
        return NULL;

    _();
    BR(path);
    spr("Can't get font (%s)", _s);
    tryft(FT_New_Face(
        library, 
        path,
        // face-index
        0,
        &face
    ), _t);

    if (!tryftok)
        return NULL;

    cairo_font_face_t *cairo_face = 
        cairo_ft_font_face_create_for_ft_face (face, FT_LOAD_TARGET_NORMAL);

    return cairo_face;

}

int main(int argc, char **argv) {

    init(argc, argv);

    /* sighup
     */
    f_sig(1, sig_handler);

    if (!draw_show())
        piep;

    s.shown = 1;

    f_sig(1, NULL);

    if (!draw_hide())
        piep;

    return 0;
}

/* Can quit.
 */
void init(int argc, char **argv) {
    autoflush();

    struct args args = {0};

    /* Can quit.
     */
    if (!arg_args(argc, argv, &args)) {
        piep;
        exit(1);
    }
        
    cairo_font_face_t *cairo_face = get_font(FONT_PATH);

    if (!cairo_face) 
        err("Can't get any fonts.");

    g.cairo_face = cairo_face;

    g.cur_page = args.cur_page;
    g.filled_bg = args.filled_bg;
    g.total_pages = args.total_pages;
    g.stay_alive_secs = args.stay_alive_secs;

    g.cur_page_s = str(int_length(g.cur_page) + 1);
    g.total_pages_s = str(int_length(g.total_pages) + 1);
    sprintf(g.cur_page_s, "%d", g.cur_page);
    sprintf(g.total_pages_s, "%d", g.total_pages);

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

    aosd_set_renderer(aosd, (AosdRenderer) draw_renderer, NULL);

    draw_update_boundaries();

    aosd_show(aosd);
}


