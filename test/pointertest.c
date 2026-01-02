# include <stdio.h>

int main() {
    int x = 10;
    int *p = &x;

    printf("x의 값         : %d\n", x);
    printf("x의 주소        : %p\n", (void*)&x);
    printf("p가 가리키는 주소 : %p\n", (void*)p);
    printf("p의 접근 값     : %d\n", *p);

    *p = 20;
    
    printf("변경 후 x의 값   : %d\n", x);

    return 0;
}