#define FONT_PATH "/usr/share/fonts/truetype/Andika-R.tf"

bool tryftok;

#define TRY_STRLEN 200 // some safety

#define tryft(x, msg) do { \
    FT_Error e;  \
    if ((e = x)) { \
        int len = strnlen(msg, TRY_STRLEN); \
        assert(len < TRY_STRLEN); \
        struct ft_error fe = ft_errors[e]; \
        const char *err_msg = fe.err_msg; \
        int len2 = strlen(err_msg); /* -l- */ \
        char *m = str(sizeof(char) * (3 + 1 + len + len2)); \
        sprintf(m, "%s (%s)", msg, err_msg); \
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


