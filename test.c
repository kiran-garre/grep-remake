#include <stdio.h>
int main(void) {
    int max = 1;
    for (int i = 0; i < max; i++) {
        printf("%d\n", i);
        if (i == 0) {
            max++;
        }
    }

}