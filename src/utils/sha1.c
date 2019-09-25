#include <fudge.h>
#include <abi.h>

static struct sha1 s;

static void ondata(struct channel *channel, void *mdata, unsigned int msize)
{

    sha1_read(&s, mdata, msize);

}

static void onfile(struct channel *channel, void *mdata, unsigned int msize)
{

    struct event_file *file = mdata;
    char buffer[FUDGE_BSIZE];
    unsigned int count;

    file_open(file->descriptor);

    while ((count = file_read(file->descriptor, buffer, FUDGE_BSIZE)))
        sha1_read(&s, buffer, count);

    file_close(file->descriptor);

}

static void onopen(struct channel *channel, void *mdata, unsigned int msize)
{

    sha1_init(&s);

}

static void onclose(struct channel *channel, void *mdata, unsigned int msize)
{

    unsigned int id = channel_reply(channel, EVENT_DATA);
    unsigned char digest[20];
    unsigned int i;

    sha1_write(&s, digest);

    for (i = 0; i < 20; i++)
        channel_appendvaluepadded(channel, digest[i], 16, 2);

    channel_appendstring(channel, "\n");
    channel_place(channel, id);
    channel_exit(channel);

}

void main(void)
{

    struct channel channel;

    channel_init(&channel);
    channel_setsignal(&channel, EVENT_OPEN, onopen);
    channel_setsignal(&channel, EVENT_CLOSE, onclose);
    channel_setsignal(&channel, EVENT_DATA, ondata);
    channel_setsignal(&channel, EVENT_FILE, onfile);
    channel_listen(&channel);

}

