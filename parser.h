#ifndef PARSER_H_
#define PARSER_H_
#include "inttypes.h"

uint8_t verifyChecksum(uint8_t *record);
uint8_t hexDec4(uint8_t *byte);
uint8_t hexDec8(uint8_t *bytes);
uint16_t hexDec16(uint8_t *bytes);

uint8_t hexDec4(uint8_t *byte){
    // transform hex character to the 4bit equivalent number, using the ascii table indexes
    if (*byte >= '0' && *byte <= '9'){
        return *byte - '0';
    }
    else if (*byte >= 'a' && *byte <='f'){
        return *byte - 'a' + 10;
    }
    else if (*byte >= 'A' && *byte <='F'){
        return *byte - 'A' + 10;
    }   
    return 0;
}

uint8_t hexDec8(uint8_t* bytes){
    return ((hexDec4(bytes) << 4) + hexDec4(bytes + 1));
}

uint16_t hexDec16(uint8_t* bytes){
    return ((hexDec8(bytes) << 8) + hexDec8(bytes + 2));
}

#endif /* PARSER_H_ */