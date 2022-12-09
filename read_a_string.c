#include <stdio.h>
#include <string.h>
#include <stdint.h>

void read_a_string(const char *s) {
    for (int i = 0; i <= strlen(s); i++) {
        printf("s[%d] = \'%c\'\n", i, s[i]);
    }
}

int main(int argc, char **argv) {
    const char msg[] = "Hello, MacEwan";
    read_a_string(msg);
    return 0;
}
