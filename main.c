#include "safeio.h"
#include <windows.h>

int main(int, const char **)
{

    FILE *fp = sfopen("output.txt", "wt");

    fprintf(fp, "Hello this is a test\n");

    sfrename(fp, "output.txt");
    fclose(fp);
    return 0;
}
