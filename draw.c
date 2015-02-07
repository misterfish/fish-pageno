#define _GNU_SOURCE 

#include <unistd.h>
#include <stdbool.h>

#include <aosd.h>

#include <fish-util.h>

#include "config.h"
#include "const.h"
#include "global.h"
#include "draw.h"

/* First we draw the fraction bar with fixed slope (1).
 * Then find a font for the numerator which fits.
 * Then find one for the denominator which fits.
 * So the fonts can be different. 
 * The whole thing could also be done by first setting a font for top, then
 * making the line, then using the same font for bottom.
 */
void draw_renderer(cairo_t *cr, void *user_data) {

    /* Init after show() has been called at least once.
     */
    if (! g.init_boundaries && g.show_called) {
        if (!draw_init_boundaries(cr))
            piepr;
        /* Don't worry about the recentering, it's just the first frame.
         */
        s.font_size_numerator = s.width * g.font_factor_numerator;
        s.font_size_denominator = s.width * g.font_factor_denominator;
    }
    const float *color;

    cairo_set_font_face(cr, g.cairo_face);


#ifdef TESTING
    cairo_rectangle(cr, 0, 0, WIDTH_INIT, HEIGHT_INIT);
    cairo_set_source_rgba(cr, COLOR_BG[0], COLOR_BG[1], COLOR_BG[2], s.alpha);
    cairo_fill(cr);
#endif

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

    cairo_set_font_size(cr, s.font_size_numerator);
    cairo_show_text(cr, g.cur_page_s);

    cairo_move_to(cr, s.stx, s.sty);
    cairo_line_to(cr, s.sbx, s.sby);
    cairo_stroke(cr);

    cairo_move_to(cr, s.x2, s.y2);

    cairo_set_font_size(cr, s.font_size_denominator);
    cairo_show_text(cr, g.total_pages_s);
}


void draw_update_boundaries() {
    /* void */
    aosd_set_position_with_offset(g.aosd, COORDINATE_MINIMUM, COORDINATE_MINIMUM, s.width, s.height, s.left, s.top);
}


void draw_update() {
    aosd_render(g.aosd);
    aosd_loop_once(g.aosd);
}

bool draw_hide() {
    return true;
}

bool draw_show() {

    /* final should be based on a minimum font XX
     */
//int WIDTH_FINAL = 50;
g.final_width = WIDTH_FINAL;
g.final_height = WIDTH_FINAL;
    g.show_called = true;
    float alpha_fact = 1.0 * (1 - ALPHA_INIT) / g.num_frames;
    float scale_fact = 1.0 * (WIDTH_FINAL - WIDTH_INIT) / g.num_frames;
    
    s.stx_max = .64 * WIDTH_FINAL; 
    s.y2_max = .5 * WIDTH_FINAL;

    for (int i = 0; i < g.num_frames; i++) {
#ifndef TESTING
        s.width += scale_fact;
        s.height += scale_fact;
#endif

        /* The line.
         */
        s.sbx = s.width * MARGIN_SIDES_PERC;
        s.sby = s.height * (1 - MARGIN_TB_PERC);
        s.sty = s.height * MARGIN_TB_PERC;
        s.stx = s.width * (1 - MARGIN_SIDES_PERC);

        int margin_side = s.width * MARGIN_SIDES_PERC;
        int margin_tb = s.height * MARGIN_TB_PERC;
        /* The numerator. Y is the baseline.
         */
        int width_numerator = s.width * g.numerator_width_perc;
        int height_numerator = s.height * g.numerator_height_perc;
        s.x1 = margin_side;
        s.y1 = height_numerator + margin_tb;
        /* The denominator. Y is the baseline.
         */
        int width_denominator = s.width * g.denominator_width_perc;
        int height_denominator = s.width * g.denominator_height_perc;
        s.x2 = s.width - margin_side - width_denominator;
        s.y2 = s.height - margin_tb;

        /* On the first one, we don't know it yet, so font is calculated in
         * render->init.
         *
         * Reduce the font scales a bit and shift the starting points to
         * compensate.
         */
        if (g.font_factor_numerator) {
            float ff = g.font_factor_numerator * FONT_FACTOR_SCALE;
            int new_width = FONT_FACTOR_SCALE * width_numerator;
            int new_height = FONT_FACTOR_SCALE * height_numerator;
            s.x1 -= 1.0 * (width_numerator - new_width) / 2;
            s.y1 += 1.0 * (height_numerator - new_height) / 2;
            s.font_size_numerator = s.width * ff;
        }
        if (g.font_factor_denominator) {
            float ff = g.font_factor_denominator * FONT_FACTOR_SCALE;
            int new_width = FONT_FACTOR_SCALE * width_denominator;
            int new_height = FONT_FACTOR_SCALE * height_denominator;
            s.x2 += 1.0 * (width_denominator - new_width) / 2;
            s.y2 -= 1.0 * (height_denominator - new_height) / 2;
            s.font_size_denominator = s.width * ff;
        }

        draw_update_boundaries();
        s.alpha += alpha_fact;

        draw_update();
    }
    sleep(g.stay_alive_secs);

    return true;
}

bool draw_init_boundaries(cairo_t *cr) {
    int margin_side = WIDTH_INIT * MARGIN_SIDES_PERC;
    int margin_tb = HEIGHT_INIT * MARGIN_TB_PERC;
    int sbx = margin_side;
    int sby = HEIGHT_INIT - margin_tb;
    int stx = WIDTH_INIT - margin_side;
    int sty = margin_tb;
    /* Conventional slope (positive for positive)
     */
    float slope = (sby - sty) / (stx - sbx);

float I = .1;
float J = .5;
float K = .001;

    float i;
    float prev_i;
    double prev_width, prev_height;
    char *s;
    s = g.cur_page_s;

    /* Slowly increase font size until text passes the fraction bar
     */
    for (i = I; i < J; i+=K) {
        double width, height;
        cairo_set_font_size(cr, i * WIDTH_INIT);
        if (!get_metrics(cr, s, &width, &height))
            pieprf;
        /* bottom right corner of text */
        int x = margin_side + width;
        int y = margin_tb + height;
        int y_projected_on_line = HEIGHT_INIT - slope * x;
        if (y > y_projected_on_line) {
            //info("stopping at y = %d, projected y = %d, i = %f", y, y_projected_on_line, i);
            break;
        }
        prev_width = width;
        prev_height = height;
        prev_i = i;
    }
    g.font_factor_numerator = prev_i;
    g.numerator_height_perc = prev_height / HEIGHT_INIT;
    g.numerator_width_perc = prev_width / WIDTH_INIT;

    s = g.total_pages_s;
    for (i = I; i < J; i+=K) {
        double width, height;
        cairo_set_font_size(cr, i * WIDTH_INIT);
        if (!get_metrics(cr, s, &width, &height))
            pieprf;
        /* top left corner of text */
        int x = WIDTH_INIT - margin_side - width;
        int y = HEIGHT_INIT - margin_tb - height;
        int y_projected_on_line = HEIGHT_INIT - slope * x;
        if (y < y_projected_on_line) {
            //info("stopping at y = %d, projected y = %d, i = %f", y, y_projected_on_line, i);
            break;
        }
        prev_width = width;
        prev_height = height;
        prev_i = i;
    }

    g.font_factor_denominator = prev_i;
    g.denominator_height_perc = prev_height / HEIGHT_INIT;
    g.denominator_width_perc = prev_width / WIDTH_INIT;

    /*
    g.font_factor_numerator *= FONT_FACTOR_SCALE;
    g.font_factor_denominator *= FONT_FACTOR_SCALE;
    */

    g.init_boundaries = true;

    return true;
}

static bool get_metrics(cairo_t *cr, char *s, double *width, double *height) {
    cairo_text_extents_t extents;
    /* void */
    cairo_text_extents(cr, s, &extents);
    *width = extents.width;
    *height = extents.height;
    return true;
}


