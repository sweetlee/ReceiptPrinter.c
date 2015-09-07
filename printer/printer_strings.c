//
//  strings.c
//  AdyenPOSTerminal
//
//  Created by Taras Kalapun on 12/11/14.
//
//

#include "strings.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int str_extract_int(char *str) {
    int num = 0;
    char *p = str;
    
    if (!p) {
        return 0;
    }
    
    while (*p) { // While there are more characters to process...
        if (isdigit(*p)) { // Upon finding a digit, ...
            num = (int)strtol(p, &p, 10); // Read a number, ...
        } else { // Otherwise, move on to the next character.
            p++;
        }
    }
    //free(p);
    return num;
}

char* str_word_wrap(char* buffer, char* string, int line_width) {
    int i = 0;
    int k, counter;
    
    // nl = "\n"
    
    while(i < strlen( string ) )
    {
        // copy string until the end of the line is reached
        for ( counter = 1; counter <= line_width; counter++ )
        {
            // check if end of string reached
            if ( i == strlen( string ) )
            {
                buffer[ i ] = 0;
                return buffer;
            }
            buffer[ i ] = string[ i ];
            // check for newlines embedded in the original input
            // and reset the index
            if ( buffer[ i ] == '\n' )
            {
                counter = 1;
            }
            i++;
        }
        // check for whitespace
        if ( isspace( string[ i ] ) )
        {
            buffer[i] = '\n';
            i++;
        }
        else
        {
            // check for nearest whitespace back in string
            for ( k = i; k > 0; k--)
            {
                if ( isspace( string[ k ] ) )
                {
                    buffer[ k ] = '\n';
                    // set string index back to character after this one
                    i = k + 1;
                    break;
                }
            }
        }
    }
    buffer[ i ] = 0;
    
    return buffer;
}

/**
 *  Render a string line
 *
 *  prefix | lstr | rstr | suffix
 *
 *  @param lstr     left part of string
 *  @param rstr     right part of string
 *  @param position 0 - left, 1 - right, 2 - center
 *  @param width    width of string (lstr + rstr)
 *  @param prefix   prefix, control characters in beginning
 *  @param suffix   suffix, control characters at end, Ex: `\n`
 *
 *  @return string, 0 terminated
 */

char *str_render_line(const char *lstr, const char *rstr,
                      int position, int width,
                      const char *prefix, const char *suffix)
{

//    int l_lstr = (int)strlen(lstr);
//    int l_rstr = (int)strlen(rstr);
//    int l_pre  = (int)strlen(prefix);
//    int l_suf  = (int)strlen(suffix);
//    
//    // length of string
//    int l_str = (l_rstr > 0) ? l_lstr + 1/* space */ + l_rstr : l_lstr;
//
//    
//    
//    
//    int len = 0;
//    int pad = 0;
//    
//    
//    
//    if (str_len > 0) {
//        
//        if ((markup & 0x03) == PrinterLineMarkupAlignmentLeft) {
//            len = str_len;
//        } else {
//            len = data->characters_per_line;
//            if ((markup & 0x03) == PrinterLineMarkupAlignmentRight) {
//                pad = len - str_len;
//            } else if ((markup & 0x03) == PrinterLineMarkupAlignmentCenter) {
//                pad = (len - str_len) / 2;
//                len = pad + str_len;
//            }
//            if (pad < 0) {
//                len = str_len;
//            }
//        }
//        
//        len += 1; // 0-end
//        char *buf = malloc(len);
//        
//        
//        if (pad > 0) {
//            snprintf(buf, len, "%*s%s", pad, " ", str);
//        } else {
//            snprintf(buf, len, "%s", str);
//        }
//        
//        
//        bytebuf_append_bytes(ob, buf, len-1);
//        free(buf);
//    }
//    
//    
//    bytebuf_append_bytes(ob, "\n", 1);
//    //free(buf);
//    
//    
    return NULL;
}

//static int
//drv_render_line_key_value(bytebuf *ob, const char *key, const char *value, const PrinterLineMarkup markup, printer_driver_data *data) {
//    _printer_log("rendering line key & val");
//    
//    int len = data->characters_per_line;
//    
//    //+1 is because we need space between
//    int pad = len - (int)(strlen(key) + strlen(value) +1);
//    
//    len += 1; // 0-end
//    char *buf = malloc(len);
//    
//    if (pad >= 0) {
//        snprintf(buf, len, "%s %*s%s", key, pad, " ", value);
//        bytebuf_append_bytes(ob, buf, len-1);
//        bytebuf_append_bytes(ob, "\n", 1);
//        free(buf);
//    } else {
//        // pad is negative, split key & val on different lines
//        drv_render_line(ob, key, PrinterLineMarkupAlignmentLeft, data);
//        drv_render_line(ob, value, PrinterLineMarkupAlignmentRight, data);
//    }
//    
//    return 0;
//}