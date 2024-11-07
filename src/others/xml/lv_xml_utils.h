/**
 * @file lv_xml_utils.h
 *
 */

#ifndef LV_XML_UTILS_H
#define LV_XML_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#define USE_STDLIB

#ifdef USE_STDLIB
    #include <stdlib.h> 
#else
    #include <limits.h> 
#endif

/**********************
 * GLOBAL PROTOTYPES
 **********************/
int lv_xml_atoi(const char *str);
static int lv_xml_is_digit(char c, int base);
long lv_xml_strtol(const char *str, char **endptr, int base);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_XML_UTILS_H*/