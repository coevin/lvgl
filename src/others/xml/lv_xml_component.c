/**
 * @file lv_xml_component.c
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_xml_component.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static void start_metadata_handler(void * user_data, const char * name, const char ** attrs);
static void end_metadata_handler(void * user_data, const char * name);
static void process_param_element(lv_xml_parser_state_t * state, const char * name, const char ** attrs);
static void process_const_element(lv_xml_parser_state_t * state, const char ** attrs);
static char * process_const_substitutions(const char * value, lv_ll_t * const_ll);
static char * process_param_substitutions(const char * value, lv_ll_t * param_ll);
static void lv_xml_style_process_with_consts(lv_xml_parser_state_t * state, const char ** attrs);
static char * str_replace(const char * str, const char * search, const char * replace);

/**********************
 *  EXTERN VARIABLES
 **********************/

lv_component_processor_t * component_processor_head = NULL;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_xml_create_component(lv_component_processor_t * processor, lv_obj_t * parent, lv_ll_t * parent_styles, const char ** attrs)
{
    // Create a new parser state for this component instance
    lv_xml_parser_state_t state = {};
    lv_ll_init(&state.parent_ll, sizeof(lv_obj_t *));
    lv_obj_t ** parent_node = lv_ll_ins_head(&state.parent_ll);
    *parent_node = parent;

    // Initialize style list
    lv_ll_init(&state.style_ll, sizeof(lv_xml_style_t));

    // First copy parent styles if provided
    if (parent_styles) {
        lv_xml_style_t * parent_style;
        _LV_LL_READ(parent_styles, parent_style) {
            lv_xml_style_t * new_style = lv_ll_ins_tail(&state.style_ll);
            *new_style = *parent_style;
        }
    }

    // Then copy component's own styles from processor
    lv_xml_style_t * style;
    _LV_LL_READ(&processor->style_ll, style) {
        lv_xml_style_t * new_style = lv_ll_ins_tail(&state.style_ll);
        *new_style = *style;
    }

    // Initialize and copy params with default values
    lv_ll_init(&state.param_ll, sizeof(lv_xml_param_t));
    lv_xml_param_t * param;
    _LV_LL_READ(&processor->param_ll, param) {
        lv_xml_param_t * new_param = lv_ll_ins_tail(&state.param_ll);
        *new_param = *param;  // This copies the default value
    }

    // Update params with instance attributes
    if (attrs) {
        for(int i = 0; attrs[i]; i += 2) {
            const char * attr_name = attrs[i];
            const char * attr_value = attrs[i + 1];
            
            // Look for matching parameter
            lv_xml_param_t * param;
            _LV_LL_READ(&state.param_ll, param) {
                if (lv_streq(param->name, attr_name)) {
                    if (param->value) lv_free((void*)param->value);
                    LV_LOG_USER("Parameter %s found. New value: %s", param->name, attr_value);
                    param->value = lv_strdup(attr_value);
                    break;
                }
            }
        }
    }

    // Initialize and copy consts
    lv_ll_init(&state.const_ll, sizeof(lv_xml_const_t));
    lv_xml_const_t * cnst;
    _LV_LL_READ(&processor->const_ll, cnst) {
        lv_xml_const_t * new_const = lv_ll_ins_tail(&state.const_ll);
        *new_const = *cnst;
    }

    // Create a copy of the XML definition for processing
   char * processed_xml = lv_strdup(processor->xml_definition);
    
    // Process each parameter
    lv_xml_param_t * param_iter;
    _LV_LL_READ(&state.param_ll, param_iter) {
        if (param_iter->value) {
            char pattern[64];
            lv_snprintf(pattern, sizeof(pattern), "${%s}", param_iter->name);
            
            char * new_xml = str_replace(processed_xml, pattern, param_iter->value);
            lv_free(processed_xml);
            processed_xml = new_xml;
        }
    }
    
    // Create the component using the processed XML definition
    lv_obj_t * component = lv_xml_load_data(parent, processed_xml , &state);
    
    // Clean up
    //lv_free(processed_xml);

    return component;
}

// Register a component definition designed in XML, for later use
lv_result_t lv_xml_load_component_data(const char * name, const char * xml_definition)
{
    lv_component_processor_t * p = lv_malloc(sizeof(lv_component_processor_t));
    lv_memzero(p, sizeof(lv_component_processor_t));

    p->name = lv_strdup(name);
    p->xml_definition = lv_strdup(xml_definition);

    // Initialize linked lists
    lv_ll_init(&p->style_ll, sizeof(lv_xml_style_t));
    lv_ll_init(&p->param_ll, sizeof(lv_xml_param_t));
    lv_ll_init(&p->const_ll, sizeof(lv_xml_const_t));

    // Create a temporary parser state to extract styles/params/consts
    lv_xml_parser_state_t state = {};
    lv_ll_init(&state.style_ll, sizeof(lv_xml_style_t));
    lv_ll_init(&state.param_ll, sizeof(lv_xml_param_t));
    lv_ll_init(&state.const_ll, sizeof(lv_xml_const_t));
    
    // Parse the XML to extract metadata
    XML_Parser parser = XML_ParserCreate(NULL);
    XML_SetUserData(parser, &state);
    XML_SetElementHandler(parser, start_metadata_handler, end_metadata_handler);
    
    if(XML_Parse(parser, xml_definition, lv_strlen(xml_definition), XML_TRUE) == XML_STATUS_ERROR) {
        LV_LOG_ERROR("XML parsing error: %s on line %lu", 
                     XML_ErrorString(XML_GetErrorCode(parser)), 
                     (unsigned long)XML_GetCurrentLineNumber(parser));
        XML_ParserFree(parser);
        return LV_RESULT_INVALID;
    }
    
    XML_ParserFree(parser);

    // Copy extracted metadata to component processor
    p->style_ll = state.style_ll;
    p->param_ll = state.param_ll;
    p->const_ll = state.const_ll;

    if(component_processor_head == NULL) component_processor_head = p;
    else {
        p->next = component_processor_head;
        component_processor_head = p;
    }

    return LV_RESULT_OK;
}

lv_result_t lv_xml_load_component_file(const char * path)
{
    // Extract component name from path
    const char *filename = strrchr(path, '/');  // Find last occurrence of '/'
    if (filename == NULL) {
        filename = path;  // No '/' found, use entire path
    } else {
        filename++;  // Skip the '/'
    }
    
    // Create a copy of the filename to modify
    char *name = lv_strdup(filename);
    char *dot = strrchr(name, '.');
    if (dot != NULL) {
        *dot = '\0';  // Remove extension
    }

    FILE *f = fopen(path, "r");
    if (f == NULL) {
        LV_LOG_ERROR("Couldn't open %s", path);
        lv_free(name);
        return LV_RESULT_INVALID;  
    }

    // Determine file size
    fseek(f, 0, SEEK_END);  
    size_t file_size = ftell(f); 
    rewind(f);  

    // Create the buffer
    char *xml_buf = lv_malloc(file_size + 1);  
    if (xml_buf == NULL) {
        LV_LOG_ERROR("Memory allocation failed for file %s", path);
        lv_free(name);
        fclose(f);  
        return LV_RESULT_INVALID;
    }

    // Read the file content 
    size_t rn = fread(xml_buf, 1, file_size, f);
    if (rn != file_size) {
        LV_LOG_ERROR("Couldn't read %s fully", path);
        lv_free(name);
        lv_free(xml_buf);  
        fclose(f);  
        return LV_RESULT_INVALID;
    }

    // Null-terminate the buffer
    xml_buf[rn] = '\0';  

    // Register the component
    lv_result_t res = lv_xml_load_component_data(name, xml_buf);

    // Housekeeping
    lv_free(name);
    lv_free(xml_buf);
    fclose(f);

    return res;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static char * process_const_substitutions(const char * value, lv_ll_t * const_ll)
{
    if (value == NULL) return NULL;
    
    char * result = lv_strdup(value);
    char * start;
    
    // Look for #{...} patterns
    while ((start = strstr(result, "#{")) != NULL) {
        char * end = strchr(start, '}');
        if (end == NULL) break;
        
        // Extract const name
        size_t const_name_len = end - (start + 2);
        char const_name[64] = {0};
        lv_memcpy(const_name, start + 2, const_name_len);
        const_name[const_name_len] = '\0';
        
        // Find const value
        lv_xml_const_t * cnst;
        _LV_LL_READ(const_ll, cnst) {
            if (lv_streq(cnst->name, const_name)) {
                // Replace #{const} with its value
                size_t prefix_len = start - result;
                size_t suffix_len = strlen(end + 1);
                size_t value_len = strlen(cnst->value);
                
                char * new_result = lv_malloc(prefix_len + value_len + suffix_len + 1);
                lv_memcpy(new_result, result, prefix_len);
                lv_memcpy(new_result + prefix_len, cnst->value, value_len);
                lv_memcpy(new_result + prefix_len + value_len, end + 1, suffix_len + 1);
                
                lv_free(result);
                result = new_result;
                break;
            }
        }
    }
    
    return result;
}

static char * process_param_substitutions(const char * value, lv_ll_t * param_ll)
{
   
    if (value == NULL) return NULL;
    
    char * result = lv_strdup(value);
    char * start;
    
    // Look for ${...} patterns
    while ((start = strstr(result, "${")) != NULL) {
        char * end = strchr(start, '}');
        if (end == NULL) {
            LV_LOG_WARN("Found ${ but no matching }");
            break;
        }
        
        // Extract param name
        size_t param_name_len = end - (start + 2);
        char param_name[64] = {0};
        lv_memcpy(param_name, start + 2, param_name_len);
        param_name[param_name_len] = '\0';
        
        LV_LOG_USER("Looking for param: %s", param_name);
        
        // Debug: Print all available parameters in the linked list
        LV_LOG_USER("Available parameters in linked list:");
        lv_xml_param_t * debug_param;
        _LV_LL_READ(param_ll, debug_param) {
            LV_LOG_USER("  - Name: %s, Value: %s", 
                       debug_param->name, 
                       debug_param->value ? debug_param->value : "NULL");
        }
        
        // Find param value
        bool found = false;
        lv_xml_param_t * param;
        _LV_LL_READ(param_ll, param) {
            LV_LOG_USER("Comparing with param: %s = %s", 
                       param->name, 
                       param->value ? param->value : "NULL");
            
            if (lv_streq(param->name, param_name)) {
                found = true;
                const char * param_value = param->value ? param->value : "";
                LV_LOG_USER("MATCH FOUND! Parameter %s = %s", param_name, param_value);
                
                // Replace ${param} with its value
                size_t prefix_len = start - result;
                size_t suffix_len = strlen(end + 1);
                size_t value_len = strlen(param_value);
                
                char * new_result = lv_malloc(prefix_len + value_len + suffix_len + 1);
                lv_memcpy(new_result, result, prefix_len);
                lv_memcpy(new_result + prefix_len, param_value, value_len);
                lv_memcpy(new_result + prefix_len + value_len, end + 1, suffix_len + 1);
                
                LV_LOG_USER("Replacement details:");
                LV_LOG_USER("  - Prefix length: %d", (int)prefix_len);
                LV_LOG_USER("  - Value length: %d", (int)value_len);
                LV_LOG_USER("  - Suffix length: %d", (int)suffix_len);
                
                lv_free(result);
                result = new_result;
                LV_LOG_USER("After substitution: %s", result);
                break;
            }
        }
        
        if (!found) {
            LV_LOG_WARN("NO MATCH FOUND for parameter: %s", param_name);
        }
    }
    
    return result;
}

static void lv_xml_style_process_with_consts(lv_xml_parser_state_t * state, const char ** attrs)
{
    // Process style attributes with const substitutions first
    const char ** processed_attrs = lv_malloc(sizeof(char*) * 32); // Assuming max 16 attr pairs
    for(int i = 0; attrs[i]; i += 2) {
        processed_attrs[i] = attrs[i];
        processed_attrs[i+1] = process_const_substitutions(attrs[i+1], &state->const_ll);
    }
    
    // Now process the style with substituted values
    lv_xml_style_process(state, processed_attrs);
    
    // Clean up
    for(int i = 0; attrs[i]; i += 2) {
        if (i % 2 == 1) { // Only free processed values
            lv_free((void*)processed_attrs[i]);
        }
    }
    lv_free(processed_attrs);
}

void lv_obj_xml_apply_attrs_with_params(lv_xml_parser_state_t * state, lv_obj_t * obj, const char ** attrs)
{
    
    // Process attributes with param substitutions first
    const char ** processed_attrs = lv_malloc(sizeof(char*) * 32); // Assuming max 16 attr pairs
    for(int i = 0; attrs[i]; i += 2) {
        processed_attrs[i] = attrs[i];
        processed_attrs[i+1] = process_param_substitutions(attrs[i+1], &state->param_ll);
    }
    
    // Now apply the processed attributes
    lv_obj_xml_apply_attrs(state, obj, processed_attrs);
    
    // Clean up
    for(int i = 0; attrs[i]; i += 2) {
        if (i % 2 == 1) { // Only free processed values
            lv_free((void*)processed_attrs[i]);
        }
    }
    lv_free(processed_attrs);
}

static void process_param_element(lv_xml_parser_state_t * state, const char * name, const char ** attrs)
{
    const char * param_name = lv_xml_get_value_of(attrs, "name");
    const char * default_value = lv_xml_get_value_of(attrs, "default");
    
    LV_LOG_USER("Processing parameter element:");
    LV_LOG_USER("  Type: %s", name);  // Now we use the element name as the type
    LV_LOG_USER("  Name: %s", param_name);
    LV_LOG_USER("  Default: %s", default_value ? default_value : "NULL");
    
    lv_xml_param_t * param = lv_ll_ins_tail(&state->param_ll);
    param->name = lv_strdup(param_name);
    
    // Convert type string to enum based on element name
    if(lv_streq(name, "int")) param->type = LV_XML_PARAM_TYPE_INT;
    else if(lv_streq(name, "string")) param->type = LV_XML_PARAM_TYPE_STRING;
    else if(lv_streq(name, "color")) param->type = LV_XML_PARAM_TYPE_COLOR;
    
    param->value = default_value ? lv_strdup(default_value) : NULL;
}

static void process_const_element(lv_xml_parser_state_t * state, const char ** attrs)
{
    const char * name = lv_xml_get_value_of(attrs, "name");
    const char * value = lv_xml_get_value_of(attrs, "value");
    
    lv_xml_const_t * cnst = lv_ll_ins_tail(&state->const_ll);
    lv_xml_set_const(cnst, name, value);
}

static enum {
    CONTEXT_NONE,
    CONTEXT_PARAMS,
    CONTEXT_CONSTS,
    CONTEXT_STYLES
} current_context = CONTEXT_NONE;

static void start_metadata_handler(void * user_data, const char * name, const char ** attrs)
{
    lv_xml_parser_state_t * state = (lv_xml_parser_state_t *)user_data;
    
    // Check for context changes
    if(lv_streq(name, "params")) {
        current_context = CONTEXT_PARAMS;
        return;
    }
    else if(lv_streq(name, "consts")) {
        current_context = CONTEXT_CONSTS;
        return;
    }
    else if(lv_streq(name, "styles")) {
        current_context = CONTEXT_STYLES;
        return;
    }
    
    // Process elements based on current context
    switch(current_context) {
        case CONTEXT_PARAMS:
            LV_LOG_USER("  Processing parameter element: %s", name);
            process_param_element(state, name, attrs);
            break;
            
        case CONTEXT_CONSTS:
            process_const_element(state, attrs);
            break;
            
        case CONTEXT_STYLES:
            if(lv_streq(name, "style")) {
                lv_xml_style_process_with_consts(state, attrs);
            }
            break;
            
        default:
            break;
    }
}

static void end_metadata_handler(void * user_data, const char * name)
{
    // Reset context when leaving a block
    if(lv_streq(name, "params") || 
       lv_streq(name, "consts") || 
       lv_streq(name, "styles")) {
        current_context = CONTEXT_NONE;
    }
}

static char * str_replace(const char * str, const char * search, const char * replace) 
{
    LV_LOG_USER("str_replace: searching for '%s' to replace with '%s'", search, replace);
    
    char * result = lv_strdup(str);
    char * pos = strstr(result, search);
    
    if (pos) {
        size_t search_len = strlen(search);
        size_t replace_len = strlen(replace);
        size_t tail_len = strlen(pos + search_len);
        
        char * new_str = lv_malloc(strlen(result) - search_len + replace_len + 1);
        
        // Copy the part before the match
        size_t prefix_len = pos - result;
        memcpy(new_str, result, prefix_len);
        
        // Copy the replacement
        memcpy(new_str + prefix_len, replace, replace_len);
        
        // Copy the rest
        memcpy(new_str + prefix_len + replace_len, pos + search_len, tail_len + 1);
        
        lv_free(result);
        result = new_str;
        
        LV_LOG_USER("Replacement done, result: '%s'", result);
    }
    
    return result;
}







