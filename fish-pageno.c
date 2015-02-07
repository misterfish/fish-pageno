#undef DEBUG 

#define _GNU_SOURCE 

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
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

#include "fish-pageno.h"
#include "arg.h"
#include "const.h"

static struct {
    Aosd *aosd;
    //FT_Face font_face;
    cairo_font_face_t *cairo_face;

    int cur_page;
    int total_pages;
    bool filled_bg;
    int stay_alive_secs;

    float font_factor;

    int final_height;
    int final_width;

    int num_frames;

    bool init_boundaries;
    bool show_called;

    float numerator_height_perc;
    float denominator_height_perc;

    double slope;
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

bool init_boundaries(cairo_t *cr) {
    char *s = "123";
float I = .1;
float J = .5;
float K = .001;

    int left_margin = WIDTH_INIT * MARGIN_SIDES_PERC;
    int top_margin = HEIGHT_INIT * MARGIN_TB_PERC;
    info("lm %d tm %d", left_margin, top_margin);
    int sbx = left_margin;
    int sby = HEIGHT_INIT - top_margin;
    int stx = WIDTH_INIT - left_margin;
    int sty = top_margin;
    /* Conventional slope (positive for positive)
     */
    float slope = (sby - sty) / (stx - sbx);
    float i;
    float prev_i;
    double prev_width, prev_height;
    /* Slowly increase font size until text passes the fraction bar
     */
    for (i = I; i < J; i+=K) {
        double width, height;
        cairo_set_font_size(cr, i * WIDTH_INIT);
        if (!get_metrics(cr, s, &width, &height))
            pieprf;
info("str %s width %f height %f", s, width, height);
        /* bottom right corner of text */
        int x = left_margin + width;
        int y = top_margin + height;
        int y_projected_on_line = HEIGHT_INIT - slope * x;
        if (y > y_projected_on_line) {
            info("stopping at y = %d, projected y = %d, i = %f", y, y_projected_on_line, i);
            break;
        }
        prev_width = width;
        prev_height = height;
        prev_i = i;
    }

    g.init_boundaries = true;
    g.font_factor = prev_i;

info("setting ff %f", g.font_factor);
    g.numerator_height_perc = prev_height / HEIGHT_INIT;
    if (g.font_factor < 0) g.font_factor = 0;
    return true;
}

void renderer(cairo_t *cr, void *user_data) {

    cairo_set_font_face(cr, g.cairo_face);
    cairo_set_font_size(cr, s.font_size);
info("font size %f", s.font_size);

    /* Init after show() has been called at least once.
     */
    if (! g.init_boundaries && g.show_called) {
        if (!init_boundaries(cr))
            piepr;
        s.font_size = s.width * g.font_factor;
    }
    const float *color;

    if (g.filled_bg) {
        //cairo_rectangle(cr, 0, 0, s.width, s.height);
        cairo_rectangle(cr, 0, 0, g.final_width, g.final_height);
        cairo_set_source_rgba(cr, COLOR_BG[0], COLOR_BG[1], COLOR_BG[2], s.alpha);
        cairo_fill(cr);

        color = COLOR_FILLED;
    }
    else {
        color = COLOR_NOT_FILLED;
    }

    cairo_set_line_width(cr, 1);
    cairo_set_source_rgba (cr, color[0], color[1], color[2], s.alpha);

    cairo_move_to(cr, s.x1, s.y1);

    _();

    spr("%d", g.cur_page);
    cairo_show_text(cr, _s);

    cairo_move_to(cr, s.stx, s.sty);
    cairo_line_to(cr, s.sbx, s.sby);
    cairo_stroke(cr);

    cairo_move_to(cr, s.x2, s.y2);

    spr("%d", g.total_pages);
    cairo_show_text(cr, _t);
}

void sig_handler(int signum) {
    /* sighup
     */
    if (signum != 1) 
        ierr;

    if (s.shown) {
        hide();
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

bool get_metrics(cairo_t *cr, char *s, double *width, double *height) {
/*
  typedef struct  FT_Glyph_Metrics_
  {
    FT_Pos  width;
    FT_Pos  height;

    FT_Pos  horiBearingX;
    FT_Pos  horiBearingY;
    FT_Pos  horiAdvance;

    FT_Pos  vertBearingX;
    FT_Pos  vertBearingY;
    FT_Pos  vertAdvance;

  } FT_Glyph_Metrics;
*/

    /* set pixel size of face. This is not for drawing -- that's handled by
     * cairo_xxx. We set it here to the initial (big) value so we can calculate the scales. 

    FT_Set_Char_Size(face, 
     */

/*
void                cairo_scaled_font_text_extents      (cairo_scaled_font_t *scaled_font, const char *utf8, cairo_text_extents_t *extents);

cairo_scaled_font_t * cairo_scaled_font_create          (cairo_font_face_t *font_face, const cairo_matrix_t *font_matrix, const cairo_matrix_t *ctm, const cairo_font_options_t *options); 

void                cairo_get_font_matrix               (cairo_t *cr, cairo_matrix_t *matrix);

*/
/*
typedef struct {
    double x_bearing;
    double y_bearing;
    double width;
    double height;
    double x_advance;
    double y_advance;
} cairo_text_extents_t;
*/

    cairo_text_extents_t extents;
    /* void */
    cairo_text_extents(cr, s, &extents);
    *width = extents.width;
    *height = extents.height;
    return true;
}

int main(int argc, char **argv) {

    init(argc, argv);

    /* sighup
     */
    f_sig(1, sig_handler);

    cairo_font_face_t *cairo_face = get_font(FONT_PATH);

    if (!cairo_face) 
        err("Can't get any fonts.");

    g.cairo_face = cairo_face;

    show();

    s.shown = 1;

    f_sig(1, NULL);

    hide();
    return 0;
}

void update_boundaries() {
    aosd_set_position_with_offset(g.aosd, COORDINATE_MINIMUM, COORDINATE_MINIMUM, s.width, s.height, s.left, s.top);
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
        
    g.cur_page = args.cur_page;
    g.filled_bg = args.filled_bg;
    g.total_pages = args.total_pages;
    g.stay_alive_secs = args.stay_alive_secs;

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
int WIDTH_FINAL = 50;
g.final_width = WIDTH_FINAL;
g.final_height = WIDTH_FINAL;
    g.show_called = true;
    float alpha_fact = 1.0 * (1 - ALPHA_INIT) / g.num_frames;
    float scale_fact = 1.0 * (WIDTH_FINAL - WIDTH_INIT) / g.num_frames;
    
    s.stx_max = .64 * WIDTH_FINAL; 
    s.y2_max = .5 * WIDTH_FINAL;

    for (int i = 0; i < g.num_frames; i++) {
        /*
        s.width += scale_fact;
        s.height += scale_fact;
        */

        /* The line.
         */
        s.sbx = s.width * MARGIN_SIDES_PERC;
        s.sby = s.height * (1 - MARGIN_TB_PERC);
        s.sty = s.height * MARGIN_TB_PERC;
        s.stx = s.width * (1 - MARGIN_SIDES_PERC);

        /* The numerator. Y is the baseline.
         */
        s.x1 = s.width * MARGIN_SIDES_PERC;
        s.y1 = (s.height * g.numerator_height_perc) + s.height * MARGIN_TB_PERC;
        /* The denominator. Y is the baseline.
         */
        s.x2 = s.width * DENOM_X_PERC;
        s.y2 = s.height * DENOM_Y_PERC;

        /* On the first one, we don't know it yet, so font is calculated in
         * render->init
         */
        if (g.font_factor) 
            s.font_size = s.width * g.font_factor;

        update_boundaries();
        s.alpha += alpha_fact;

        update();
    }
    sleep(g.stay_alive_secs);
}

void hide() {
}

void update() {
    aosd_render(g.aosd);
    aosd_loop_once(g.aosd);
}

