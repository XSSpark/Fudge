#include <fudge.h>
#include <abi.h>

static void ondata(unsigned int source, void *mdata, unsigned int msize)
{

    channel_sendbuffer(EVENT_DATA, msize, mdata);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    unsigned int id = file_spawn(FILE_L0, "/bin/echo");

    if (!id)
        channel_error("Could not spawn process");

    channel_redirectback(id, EVENT_DATA);
    channel_redirectback(id, EVENT_CLOSE);
    channel_sendstringzto(id, EVENT_PATH, "/data/help.txt");
    channel_sendto(id, EVENT_MAIN);
    channel_wait(id, EVENT_CLOSE);
    channel_close();

}

void init(void)
{

    channel_bind(EVENT_DATA, ondata);
    channel_bind(EVENT_MAIN, onmain);

}

