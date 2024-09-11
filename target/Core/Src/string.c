/*
 * string.c
 *
 *  Created on: Aug 27, 2024
 *      Author: sahin
 */

#include "main.h"

uint32_t GetUnsignedIntFromASCII(char *ch, uint16_t len)
{
    uint32_t result = 0;

    for (int i = 0; i < len; i++) {
        char c = ch[i];
        uint32_t value = 0;

        // Convert ASCII character to its hex value
        if (isdigit(c)) {
            value = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            value = 10 + (c - 'a');
        } else if (c >= 'A' && c <= 'F') {
            value = 10 + (c - 'A');
        }

        // Shift the previous result left by 4 bits and add the new value
        result = (result << 4) | value;
    }

    return result;
}
