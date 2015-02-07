// XX
#define FONT_PATH "/usr/share/fonts/truetype/Andika-R.ttf"
//#define FONT_PATH "/usr/share/fonts/truetype/Andika-R.ttf/usr/share/fonts/truetype/Andika-R.ttf/usr/share/fonts/truetype/Andika-R.ttf"

bool tryftok;

#define TRY_STRLEN 200 // some safety

/* msg can depend on user data (e.g. font path) */
#define tryft(x, msg) do { \
    FT_Error e;  \
    if ((e = x)) { \
        struct ft_error fe = ft_errors[e]; \
        const char *err_msg = fe.err_msg; \
        /* in case literal */ \
        char *msgp = (char*) msg; \
        int len = strnlen(msgp, TRY_STRLEN); \
        if (len == TRY_STRLEN) \
            msgp[len-1] = '\0'; \
        char *errmsgp = str(TRY_STRLEN); \
        int len2 = snprintf(errmsgp, TRY_STRLEN, "%s", err_msg); \
        int mlen = 3 + 1 + len + len2; \
        char *m = str(mlen); \
        snprintf(m, mlen, "%s (%s)", msgp, errmsgp); \
        warn(m); \
        free(m); \
        tryftok = false; \
    } \
    else tryftok = true; \
} while (0); 

/* Trick from fterrors.h to load error mapping:
*/

#undef __FTERRORS_H__                                          
#define FT_ERRORDEF( e, v, s )  { e, s },                      
#define FT_ERROR_START_LIST     {                              
#define FT_ERROR_END_LIST       { 0, 0 } };                    
                                                               
const struct ft_error
{                                                              
  int          err_code;                                       
  const char*  err_msg;                                        
} ft_errors[] =                                                
                                                               
#include FT_ERRORS_H                                           
                                                               
/* End error mapping.
 */

void init(int, char**);
void show();
void hide();
void update();

bool get_metrics(cairo_t *cr, char *s, double *width, double *height);
