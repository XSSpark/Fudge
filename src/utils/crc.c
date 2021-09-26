#include <fudge.h>
#include <abi.h>

static struct crc sum;

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    crc_read(&sum, mdata, msize);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    message_init(&message, EVENT_DATA);
    message_putvalue(&message, crc_finalize(&sum), 10, 0);
    message_putstring(&message, "\n");
    channel_replymsg(&message);
    channel_close();

}

static void onpath(unsigned int source, void *mdata, unsigned int msize)
{

    if (file_walk2(FILE_L0, mdata))
    {

        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_seek(FILE_L0, 0);

        while ((count = file_read(FILE_L0, buffer, BUFFER_SIZE)))
            crc_read(&sum, buffer, count);

    }

}

void init(void)
{

    channel_setcallback(EVENT_DATA, ondata);
    channel_setcallback(EVENT_MAIN, onmain);
    channel_setcallback(EVENT_PATH, onpath);

}

