/**
 * @file lv_xml_chart_parser.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_xml_chart_parser.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_chart_type_t chart_type_string_to_enum_value(const char * txt);
static lv_chart_update_mode_t chart_update_mode_string_to_enum_value(const char * txt);
static lv_chart_axis_t chart_axis_string_to_enum_value(const char * txt);

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void * lv_xml_chart_process(lv_xml_parser_state_t * state, const char ** attrs)
{
    void * item = lv_chart_create(lv_xml_state_get_parent(state));

    lv_obj_xml_apply_attrs(state, item, attrs); /*Apply the common properties, e.g. width, height, styles flags etc*/

    for(int i = 0; attrs[i]; i += 2) {
        const char * name = attrs[i];
        const char * value = attrs[i + 1];

        if(lv_streq("point_count", name)) lv_chart_set_point_count(item, lv_xml_atoi(value));
    }
    return item;
}

void * lv_xml_chart_series_process(lv_xml_parser_state_t * state, const char ** attrs)
{
    const char * color = lv_xml_get_value_of(attrs, "color");
    const char * axis = lv_xml_get_value_of(attrs, "axis");
    void * item = lv_chart_add_series(lv_xml_state_get_parent(state), lv_color_hex(lv_xml_strtol(color, NULL, 16)),
                                      chart_axis_string_to_enum_value(axis));
    lv_obj_t * parent = lv_xml_state_get_parent(state);
    /* There are no properties to process */


    lv_chart_set_all_value(parent, item, lv_rand(10, 90));

    return item;
}

void * lv_xml_chart_cursor_process(lv_xml_parser_state_t * state, const char ** attrs)
{
    const char * color = lv_xml_get_value_of(attrs, "color");
    const char * dir = lv_xml_get_value_of(attrs, "dir");
    void * item = lv_chart_add_cursor(lv_xml_state_get_parent(state), lv_color_hex(lv_xml_strtol(color, NULL, 16)),
                                      lv_xml_dir_string_to_enum_value(dir));

    lv_obj_t * parent = lv_xml_state_get_parent(state);
    lv_point_t p = {30, 40};
    lv_chart_set_cursor_pos(parent, item, &p);

    /* There are no properties to process */

    return item;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static lv_chart_type_t chart_type_string_to_enum_value(const char * txt)
{
    if(lv_streq("none", txt)) return LV_CHART_TYPE_NONE;
    if(lv_streq("line", txt)) return LV_CHART_TYPE_LINE;
    if(lv_streq("bar", txt)) return LV_CHART_TYPE_BAR;
    if(lv_streq("scatter", txt)) return LV_CHART_TYPE_SCATTER;

    LV_LOG_WARN("%s is an unknown value for chart's chart_type", txt);
    return 0; /*Return 0 in lack of a better option. */
}
static lv_chart_update_mode_t chart_update_mode_string_to_enum_value(const char * txt)
{
    if(lv_streq("shift", txt)) return LV_CHART_UPDATE_MODE_SHIFT;
    if(lv_streq("circular", txt)) return LV_CHART_UPDATE_MODE_CIRCULAR;

    LV_LOG_WARN("%s is an unknown value for chart's chart_update_mode", txt);
    return 0; /*Return 0 in lack of a better option. */
}
static lv_chart_axis_t chart_axis_string_to_enum_value(const char * txt)
{
    if(lv_streq("primary_x", txt)) return LV_CHART_AXIS_PRIMARY_X;
    if(lv_streq("primary_y", txt)) return LV_CHART_AXIS_PRIMARY_Y;
    if(lv_streq("secondary_x", txt)) return LV_CHART_AXIS_SECONDARY_X;
    if(lv_streq("secondary_y", txt)) return LV_CHART_AXIS_SECONDARY_Y;

    LV_LOG_WARN("%s is an unknown value for chart's chart_axis", txt);
    return 0; /*Return 0 in lack of a better option. */
}