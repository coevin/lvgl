/**
 * @file lv_xml.h
 *
 */

#ifndef LV_XML_H
#define LV_XML_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl/lvgl.h"
#include "../../libs/expat/expat.h"
#include "../../misc/lv_types.h"
#include "lv_xml_private.h"
#include "lv_xml_utils.h"
#include "lv_xml_component.h"
#include "lv_xml_base_types.h"
#include "lv_xml_obj_parser.h"
#include "lv_xml_button_parser.h"
#include "lv_xml_label_parser.h"
#include "lv_xml_slider_parser.h"
#include "lv_xml_tabview_parser.h"
#include "lv_xml_chart_parser.h"

/*********************
 *      DEFINES
 *********************/

//HACK!!!
#define LV_LABEL_LONG_MODE_SCROLL LV_LABEL_LONG_SCROLL
#define LV_LABEL_LONG_MODE_WRAP LV_LABEL_LONG_WRAP

/**********************
 *      TYPEDEFS
 **********************/

typedef  void * (*lv_xml_widget_process_cb_t)(lv_xml_parser_state_t * state, const char ** attrs);

/**********************
 * GLOBAL PROTOTYPES
 **********************/

void lv_xml_init(void);
lv_result_t lv_xml_register_widget_processor(const char * name, lv_xml_widget_process_cb_t cb);
lv_obj_t * lv_xml_load_data(lv_obj_t * parent, const char * xml_definition, lv_xml_parser_state_t * parent_state);
lv_obj_t * lv_xml_load_file(lv_obj_t * parent, const char * path);
const char * lv_xml_get_value_of(const char ** attrs, const char * name);
void * lv_xml_state_get_parent(lv_xml_parser_state_t * state);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_XML_H*/
