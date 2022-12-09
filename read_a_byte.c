#include <stdio.h>
#include <stdint.h>

void read_a_byte(uint8_t *u8) {
    printf("The byte at virtual address %p is: %d\n", u8, *u8);
}

int main(int argc, char **argv) {
    uint8_t u8 = 42;
    read_a_byte(&u8);
    return 0;
}
