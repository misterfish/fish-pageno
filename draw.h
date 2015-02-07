struct {
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

    float font_size_numerator;
    float font_size_denominator;
} s;

void draw_renderer(cairo_t *cr, void *user_data);

void draw_update_boundaries();
void draw_update();

bool draw_show();
bool draw_hide();

bool draw_init_boundaries(cairo_t *cr);

static bool get_metrics(cairo_t *cr, char *s, double *width, double *height);
