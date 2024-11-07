/**
 * @file lv_xml_component.h
 *
 */

#ifndef LV_LABEL_XML_COMPONENT_H
#define LV_LABEL_XML_COMPONENT_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <string.h>
#include <stdio.h>
#include "../../lvgl.h"
#include "lv_xml.h"
#include "lv_xml_utils.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef  void * (*lv_xml_component_process_cb_t)(lv_obj_t * parent, const char * data, const char ** attrs);

typedef struct lv_xml_component_t {
    const char * name;
    const char * xml_definition;
    void * item;
} lv_xml_component_t;

typedef struct _lv_component_processor_t {
    const char * name;
    const char * xml_definition;
    lv_ll_t style_ll;
    lv_ll_t param_ll;
    lv_ll_t const_ll;
    struct _lv_component_processor_t * next;
} lv_component_processor_t;

/**********************
 *  EXTERN VARIABLES
 **********************/

extern lv_component_processor_t * component_processor_head;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

lv_obj_t * lv_xml_create_component(lv_component_processor_t * processor, lv_obj_t * parent, lv_ll_t * parent_styles, const char ** attrs);
lv_result_t lv_xml_load_component_data(const char * name, const char * xml_definition);
lv_result_t lv_xml_load_component_file(const char * path);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_XML_COMPONENT_H*/


