/**
 * @file lv_xml_base_types.h
 *
 */

#ifndef LV_XML_BASE_TYPES_H
#define LV_XML_BASE_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include "lv_xml.h"
#include "../../../lvgl.h"
#include "../../../src/lvgl_private.h"
#include "../../misc/lv_types.h"
#include "../../misc/lv_area.h"

/**********************
 *      TYPEDEFS
 **********************/
typedef struct _lv_xml_parser_state_t lv_xml_parser_state_t;

typedef enum {
    LV_XML_PARAM_TYPE_INT,
    LV_XML_PARAM_TYPE_PX,
    LV_XML_PARAM_TYPE_PERCENT,
    LV_XML_PARAM_TYPE_CONTENT,
    LV_XML_PARAM_TYPE_SUBJECT,
    LV_XML_PARAM_TYPE_STRING,
    LV_XML_PARAM_TYPE_STRING_ARRAY,
    LV_XML_PARAM_TYPE_COLOR,
    LV_XML_PARAM_TYPE_OPA,
    LV_XML_PARAM_TYPE_GRAD,
    LV_XML_PARAM_TYPE_OBJ,
    LV_XML_PARAM_TYPE_EVENT_CB,
    LV_XML_PARAM_TYPE_ANIMATION,
    LV_XML_PARAM_TYPE_FONT,
    LV_XML_PARAM_TYPE_IMAGE_SRC,
    LV_XML_PARAM_TYPE_STYLE,
    LV_XML_PARAM_TYPE_BOOL,
    LV_XML_PARAM_TYPE_POINT,
    LV_XML_PARAM_TYPE_ARGLIST,
    LV_XML_PARAM_TYPE_TYPE
} lv_xml_param_type_t;

typedef struct {
    const char *name;
    lv_xml_param_type_t type;
    char *value;  
} lv_xml_param_t;

typedef struct {
    const char *name;
    const char *value; 
} lv_xml_const_t;


/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_state_t lv_xml_state_string_to_enum_value(const char * txt);
lv_part_t lv_xml_part_string_to_enum_value(const char * txt);
lv_align_t lv_xml_align_string_to_enum_value(const char * txt);
lv_dir_t lv_xml_dir_string_to_enum_value(const char * txt);
void lv_xml_styles_add(lv_xml_parser_state_t * state, lv_obj_t * obj, const char * text);
void lv_xml_style_process(lv_xml_parser_state_t * state, const char ** attrs);
void lv_xml_set_param(lv_xml_param_t *param, const char *value);
const char* lv_xml_get_param(const lv_xml_param_t *param);
void lv_xml_set_const(lv_xml_const_t *const_item, const char *name, const char *value);
const char* lv_xml_get_const(const lv_xml_const_t *const_item);

/**
 * Apply attributes to an object with parameter substitutions
 * @param state Current parser state
 * @param obj Object to apply attributes to
 * @param attrs Array of attribute name/value pairs
 */
void lv_obj_xml_apply_attrs_with_params(lv_xml_parser_state_t * state, lv_obj_t * obj, const char ** attrs);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_XML_BASE_TYPES_H*/
