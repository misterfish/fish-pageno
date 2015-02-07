struct {
    Aosd *aosd;
    //FT_Face font_face;
    cairo_font_face_t *cairo_face;

    int cur_page;
    int total_pages;
    char *cur_page_s;
    char *total_pages_s;
    bool filled_bg;
    int stay_alive_secs;

    /* As perc of width */
    float font_factor_numerator;
    float font_factor_denominator;

    int final_height;
    int final_width;

    int num_frames;


    float numerator_height_perc;
    float numerator_width_perc;
    float denominator_height_perc;
    float denominator_width_perc;

    double slope;

    bool init_boundaries;
   bool show_called;
} g;


