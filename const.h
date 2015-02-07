/* Needs config.h 
 */

static const int NUM_FRAMES_FILLED = CONFIG_NUM_FRAMES_FILLED;
static const int NUM_FRAMES_NOT_FILLED = CONFIG_NUM_FRAMES_NOT_FILLED;

static const int WIDTH_INIT = 400;
static const int HEIGHT_INIT = 400;
static const int LEFT = 0;
static const int TOP = 0;

static const int FONT_MINIMUM = 10;
// XX
static const int WIDTH_FINAL = 60;

static const int STAY_ALIVE_SECS = 2;

static const float MARGIN_SIDES_PERC = .1;
static const float MARGIN_TB_PERC = .1;

/* After finding the point the glyphs touch the fraction bar, scale by this
 * amount.
 */
static const float FONT_FACTOR_SCALE = .8;

static const float COLOR_NOT_FILLED[] = { .3, .2, .05 };
static const float COLOR_FILLED[] = { .7, .7, .4 };

static const float COLOR_BG[] = { .1, .1, .3 };

static const float ALPHA_INIT = .1;


