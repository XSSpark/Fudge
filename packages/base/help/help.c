#include <fudge.h>

void main()
{

    char buffer[FUDGE_BSIZE];
    unsigned int offset;
    unsigned int count;

    if (!call_open(FUDGE_IN, FUDGE_ROOT, 21, "ramdisk/home/help.txt"))
        return;

    for (offset = 0; (count = call_read(FUDGE_IN, offset, FUDGE_BSIZE, buffer)); offset += count)
        call_write(FUDGE_OUT, offset, count, buffer);

}

