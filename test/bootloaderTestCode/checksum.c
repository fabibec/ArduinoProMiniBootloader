#include <stdio.h>
#include <inttypes.h>
#include <string.h>

uint16_t hexDec(uint8_t *bytes, uint8_t num){
    uint16_t ret = 0;
    uint8_t c;
    for(uint8_t i = 0; i < num; i++){
        c = bytes[i];
        if(c >= '0' && c <= '9'){
            c -= '0';
        } else if (c >= 'A' && c <= 'F'){
            c -= ('A' - 10);
        } else if (c >= 'a' && c <= 'f'){
            c -= ('a' - 10);
        }
        ret = (ret << 4) | c;
    } 
    return ret;
}
char hexStr[44];

int main(){
    printf("Read hex string:\n");

    scanf("%s", hexStr);
    uint8_t checksum = 0;
    uint8_t temp;
    uint16_t conv;

    // Test 4 Byte adding
    for(uint8_t i = 1; i < 5; i += 4){
        conv = hexDec((uint8_t*) &hexStr[i], 4);
        printf("2 Byte Conv: %u & %u", (uint8_t) conv, (uint8_t) (conv >> 8));
        checksum += (uint8_t) conv;
        checksum += (uint8_t) (conv >> 8);
    }

    // Test 2 Byte adding
    for(uint8_t j = 5; j < (strlen(hexStr) - 2); j += 2){
        temp = hexDec((uint8_t*) &hexStr[j], 2);
        checksum += temp;
    }

    printf("\nChecksum = %X\n", (uint8_t) ~checksum + 1);
    return 0;
}
