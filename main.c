#include "program.h"

int main(int argc, char *argv[])
{
    initEditor(argc, argv);
    runEditor();
    deinitEditor();

    return 0;
}