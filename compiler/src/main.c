#include <stdio.h>
#include "libcfloor/string.h"

int main(int argc, const char *argv[])
{
    String string = string_from_cstr("Yay!");
    printf("%s\n", string_to_cstr(string));
    return 0;
}
