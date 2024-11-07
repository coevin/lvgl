/*********************
 *      INCLUDES
 *********************/
#include "lv_xml_utils.h"

/*********************
 *      FUNCTIONS
 *********************/

// Wrapper function to convert string to int
int lv_xml_atoi(const char *str) {
#ifdef USE_STDLIB
    return atoi(str);
#else
    const char *s = str;
    int result = 0;
    int sign = 1;

    // Skip leading whitespace (only ' ' and '\t' considered)
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    // Handle optional sign
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Convert the string
    while (*s) {
        if (*s >= '0' && *s <= '9') {
            int digit = *s - '0';

            // Check for overflow before it happens
            if (result > (INT_MAX - digit) / 10) {
                return (sign == 1) ? INT_MAX : INT_MIN; // Return limits on overflow
            }

            result = result * 10 + digit;
            s++;
        } else {
            break; // Non-digit character
        }
    }

    return result * sign;
#endif
}

#ifndef USE_STDLIB
// Helper function for custom implementation to check valid digit
static int lv_xml_is_digit(char c, int base) {
    if (base <= 10) {
        return (c >= '0' && c < '0' + base);
    } else {
        return (c >= '0' && c <= '9') || (c >= 'a' && c < 'a' + (base - 10)) || (c >= 'A' && c < 'A' + (base - 10));
    }
}
#endif

// Wrapper function to convert string to long
long lv_xml_strtol(const char *str, char **endptr, int base) {
#ifdef USE_STDLIB
    return strtol(str, endptr, base);
#else
    const char *s = str;
    long result = 0;
    int sign = 1;

    // Skip leading whitespace (only ' ' and '\t' considered)
    while (*s == ' ' || *s == '\t') {
        s++;
    }

    // Handle optional sign
    if (*s == '-') {
        sign = -1;
        s++;
    } else if (*s == '+') {
        s++;
    }

    // Determine base if 0 is passed as base
    if (base == 0) {
        if (*s == '0') {
            if (*(s + 1) == 'x' || *(s + 1) == 'X') {
                base = 16;
                s += 2;
            } else {
                base = 8;
                s++;
            }
        } else {
            base = 10;
        }
    }

    // Convert the string
    while (*s) {
        int digit;

        if (lv_xml_is_digit(*s, base)) {
            if (*s >= '0' && *s <= '9') {
                digit = *s - '0';
            } else if (*s >= 'a' && *s <= 'f') {
                digit = *s - 'a' + 10;
            } else if (*s >= 'A' && *s <= 'F') {
                digit = *s - 'A' + 10;
            } else {
                // This should not happen due to lv_xml_is_digit check
                break;
            }

            // Check for overflow
            if (result > (LONG_MAX - digit) / base) {
                result = (sign == 1) ? LONG_MAX : LONG_MIN;
                if (endptr) *endptr = (char *)s;
                return result;
            }

            result = result * base + digit;
            s++;
        } else {
            // Non-digit character
            break;
        }
    }

    // Set end pointer to the last character processed
    if (endptr) {
        *endptr = (char *)s;
    }

    return result * sign;
#endif
}

