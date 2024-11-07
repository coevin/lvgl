/**
 * @file lv_xml.c
 *
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_xml.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

typedef struct _lv_widget_processor_t {
    const char * name;
    lv_xml_widget_process_cb_t cb;
    struct _lv_widget_processor_t * next;
} lv_widget_processor_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void start_element_handler(void * user_data, const char * name, const char ** attrs);
static void end_element_handler(void * user_data, const char * name);
//static void character_data_handler(void *user_data, const char *s, int len);

/**********************
 *  STATIC VARIABLES
 **********************/
static lv_widget_processor_t * widget_processor_head;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lv_xml_init(void)
{
    lv_xml_register_widget_processor("obj", lv_xml_obj_process);
    lv_xml_register_widget_processor("button", lv_xml_button_process);
    lv_xml_register_widget_processor("label", lv_xml_label_process);
    lv_xml_register_widget_processor("slider", lv_xml_label_process);
    lv_xml_register_widget_processor("tabview", lv_xml_tabview_process);
    lv_xml_register_widget_processor("tabview-tab_bar", lv_xml_tabview_tab_bar_process);
    lv_xml_register_widget_processor("tabview-tab", lv_xml_tabview_tab_process);
    lv_xml_register_widget_processor("chart", lv_xml_chart_process);
    lv_xml_register_widget_processor("chart-cursor", lv_xml_chart_cursor_process);
    lv_xml_register_widget_processor("chart-series", lv_xml_chart_series_process);
}

lv_result_t lv_xml_register_widget_processor(const char * name, lv_xml_widget_process_cb_t cb)
{
    lv_widget_processor_t * p = lv_malloc(sizeof(lv_widget_processor_t));
    lv_memzero(p, sizeof(lv_widget_processor_t));

    p->cb = cb;
    p->name = lv_strdup(name);

    if(widget_processor_head == NULL) widget_processor_head = p;
    else {
        p->next = widget_processor_head;
        widget_processor_head = p;
    }
    return LV_RESULT_OK;
}

lv_obj_t * lv_xml_load_data(lv_obj_t * parent, const char * xml_definition, lv_xml_parser_state_t * parent_state)
{
    // Initialize the parser state
    lv_xml_parser_state_t state = {};
    lv_ll_init(&state.parent_ll, sizeof(lv_obj_t *));
    lv_obj_t ** parent_node = lv_ll_ins_head(&state.parent_ll);
    *parent_node = parent;

    // Initialize styles - either from parent or new
    lv_ll_init(&state.style_ll, sizeof(lv_xml_style_t));
    if (parent_state) {
        // Copy styles from parent state
        lv_xml_style_t * style;
        _LV_LL_READ(&parent_state->style_ll, style) {
            lv_xml_style_t * new_style = lv_ll_ins_tail(&state.style_ll);
            *new_style = *style;  
        }
    }

    // Create an XML parser and set handlers
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &state);
    XML_SetElementHandler(parser, start_element_handler, end_element_handler);
    //  XML_SetCharacterDataHandler(parser, character_data_handler);

    // Parse the XML
    if(XML_Parse(parser, xml_definition, lv_strlen(xml_definition), XML_TRUE) == XML_STATUS_ERROR) {
        LV_LOG_ERROR("XML parsing error: %s on line %lu", XML_ErrorString(XML_GetErrorCode(parser)), XML_GetCurrentLineNumber(parser));
        XML_ParserFree(parser);
        return NULL;
    }

    XML_ParserFree(parser);

    return state.parent;
}

lv_obj_t * lv_xml_load_file(lv_obj_t * parent, const char * path)
{

    lv_obj_t * item;

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        LV_LOG_ERROR("Couldn't open %s", path);
        return NULL;  
    }

    // Determine file size
    fseek(f, 0, SEEK_END);  
    size_t file_size = ftell(f); 
    rewind(f);  

    // Create the buffer
    char *xml_buf = lv_malloc(file_size + 1);  
    if (xml_buf == NULL) {
        LV_LOG_ERROR("Memory allocation failed for file %s", path);
        //Housekeeping
        fclose(f);  
        return NULL;
    }

    // Read the file content 
    size_t rn = fread(xml_buf, 1, file_size, f);
    if (rn != file_size) {
        LV_LOG_ERROR("Couldn't read %s fully", path);
        // Housekeeping
        lv_free(xml_buf);  
        fclose(f);  
        return NULL;
    }

    // Null-terminate the buffer (in case it is expected)
    xml_buf[rn] = '\0';  

    item = lv_xml_load_data(parent, xml_buf, NULL);

    if(item == NULL) LV_LOG_WARN("An error occurred while processing %s", path);

    // Housekeeping
    lv_free(xml_buf);
    fclose(f);

    return item;

}

const char * lv_xml_get_value_of(const char ** attrs, const char * name)
{
    for(int i = 0; attrs[i]; i += 2) {
        if(lv_streq(attrs[i], name)) return attrs[i + 1];
    }

    LV_LOG_WARN("No value found for name:%s", name);
    return NULL;
}

void * lv_xml_state_get_parent(lv_xml_parser_state_t * state)
{
    return state->parent;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void start_element_handler(void * user_data, const char * name, const char ** attrs)
{
    lv_xml_parser_state_t * state = (lv_xml_parser_state_t *)user_data;

    bool is_view = false;
    if(lv_streq(name, "view")) {
        const char * extends = lv_xml_get_value_of(attrs, "extends");
        name = extends ? extends : "obj";
        is_view = true;
    }

    if(lv_streq(name, "styles")) return;
    else if(lv_streq(name, "params")) return;
    else if(lv_streq(name, "consts")) return;
    else if(lv_streq(name, "style")) lv_xml_style_process(state, attrs);
    else {

        lv_obj_t ** current_parent_p = lv_ll_get_tail(&state->parent_ll);
        if(current_parent_p == NULL){
            if(state->parent==NULL){
                LV_LOG_ERROR("There is no parent object available for %s. This also should never happen.", name);
                return;
            }
            else {
                current_parent_p = &state->parent;
            }
        }
        else{
            state->parent = *current_parent_p;
        }
        
        void * item = NULL;

        /* Select the widget specific parser type based on the name */
        lv_widget_processor_t * p = widget_processor_head;
        while(p) {
            if(lv_streq(p->name, name)) {
                item = p->cb(state, attrs);
                break;
            }

            p = p->next;
        }

        

        /* If not a widget, check if it is a component */
        if(item == NULL) {
            lv_component_processor_t * p_c = component_processor_head;
            while(p_c) {
                if(lv_streq(p_c->name, name)) {
                    item = lv_xml_create_component(p_c, state->parent, &state->style_ll, attrs);
                    if(item) {
                        lv_obj_xml_apply_attrs_with_params(state, item, attrs);
                    }
                    break;
                }
                p_c = p_c->next;
            }
        }

        /* If it isn't a component either then it is unknown */
        if(item == NULL) {
            LV_LOG_WARN("%s in not a known widget or element", name);
            return;
        }

        void ** new_parent = lv_ll_ins_tail(&state->parent_ll);

        *new_parent = item;

        if(is_view) {
            state->view = item;
        }
    }
}
static void end_element_handler(void * user_data, const char * name)
{
    lv_xml_parser_state_t * state = (lv_xml_parser_state_t *)user_data;

    lv_obj_t ** current_parent = lv_ll_get_tail(&state->parent_ll);
    if(current_parent) lv_ll_remove(&state->parent_ll, current_parent);
}


//char *strip(char *str) {
//    char *end;
//
//    // Trim leading spaces
//    while (isspace((unsigned char)*str)) str++;
//
//    if (*str == 0) { // All spaces?
//        return str;
//    }
//
//    // Trim trailing spaces
//    end = str + strlen(str) - 1;
//    while (end > str && isspace((unsigned char)*end)) end--;
//
//    // Write new null terminator
//    *(end + 1) = '\0';
//
//    return str;
//}
//
//static void character_data_handler(void *user_data, const char *s, int len)
//{
//  char t[1000] = {};
//  memcpy(t, s, len);
//  t[len] = '\0';
//  const char * t2 = strip(t);
//  if(strlen(t2)) printf("%s\n", t2);
//}
