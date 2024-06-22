#include <stdio.h>
#include <string.h>
#include <inttypes.h>

uint16_t hexDec(uint8_t *bytes, uint8_t num);

char str[20];

int main(){
    printf("Please input a hex string of length two or four\n");
    scanf("%s", str);
    int len = strlen(str);
    uint16_t h = hexDec(str, len);
    printf("%s of length %d -> %d\n", str, len, h);
    return 0;
}

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