#include <fudge.h>
#include <abi.h>

#define MAXSIZE 1200
#define BLOCKSIZE 512
#define OK 1
#define ERROR 0

struct request
{

    unsigned int source;
    unsigned int offset;
    unsigned int count;
    unsigned int blocksector;
    unsigned int blockoffset;
    unsigned int blockcount;
    unsigned int blockreads;

} requests[8];

static char blocks[BLOCKSIZE * 4];

static void request_init(struct request *request, unsigned int source, unsigned int offset, unsigned int count)
{

    request->source = source;
    request->offset = offset;
    request->count = count;
    request->blocksector = offset / BLOCKSIZE;
    request->blockoffset = offset % BLOCKSIZE;
    request->blockcount = ((BLOCKSIZE - 1) + request->blockoffset + request->count) / BLOCKSIZE;
    request->blockreads = 0;

}

static void request_send(struct request *request)
{

    struct {struct message_header header; struct event_blockrequest blockrequest;} message;

    message.blockrequest.sector = request->blocksector;
    message.blockrequest.count = request->blockcount;

    message_initheader(&message.header, EVENT_BLOCKREQUEST, sizeof (struct event_blockrequest));
    file_writeall(FILE_G1, &message, message.header.length);

}

static unsigned int request_poll(struct request *request)
{

    struct message message;

    while (channel_polleventsystem(EVENT_DATA, &message))
    {

        request->blockreads += buffer_write(blocks, BLOCKSIZE * 4, message.data.buffer, message_datasize(&message.header), request->blockreads * BLOCKSIZE) / BLOCKSIZE;

        if (request->blockreads == request->blockcount)
            return request->count;

    }

    return 0;

}

static unsigned int sendpoll(struct request *request, unsigned int source, unsigned int offset, unsigned int count)
{

    request_init(request, source, offset, count);
    request_send(request);

    return request_poll(request);

}

static void *getdata(struct request *request)
{

    return ((char *)blocks + request->blockoffset);

}

static unsigned int walk(unsigned int source, struct request *request, unsigned int length, char *path)
{

    unsigned int offset = 0;

    while (sendpoll(request, source, offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = getdata(request);

        if (!cpio_validate(header))
            break;

        if (header->namesize == 11)
        {

            if (buffer_match("TRAILER!!!", header + 1, 11))
                break;

        }

        if (header->namesize == length + 1)
        {

            if (buffer_match(path, header + 1, length))
                return OK;

        }

        offset = request->offset + cpio_next(header);

    }

    return ERROR;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    file_link(FILE_G0);

    while (channel_process());

    file_unlink(FILE_G0);

}

static void on9popen(unsigned int source, struct p9p_event *p9p)
{

    struct message message;

    p9p_mkropen(&message, p9p_read2(p9p, P9P_OFFSET_TAG), 0, 0);
    channel_sendmessageto(source, &message);

}

static void on9pread(unsigned int source, struct p9p_event *p9p)
{

    struct request *request = &requests[0];

    channel_sendvalue(EVENT_DATA, p9p_read4(p9p, P9P_OFFSET_DATA), 10, 0);

    file_link(FILE_G1);

    if (sendpoll(request, source, request->offset, sizeof (struct cpio_header) + 1024))
    {

        struct cpio_header *header = getdata(request);

        if (cpio_validate(header))
        {

            unsigned int count;

            if ((count = sendpoll(request, source, request->offset + cpio_filedata(header), cpio_filesize(header))))
            {

                char buffer[MESSAGE_SIZE];

                channel_sendbufferto(source, EVENT_P9P, p9p_mkrread(buffer, p9p_read2(p9p, P9P_OFFSET_TAG), count, getdata(request)), buffer);

            }

        }

    }

    file_unlink(FILE_G1);

}

static void on9pversion(unsigned int source, struct p9p_event *p9p)
{

    unsigned int msize = p9p_read4(p9p, P9P_OFFSET_DATA);
    char buffer[MESSAGE_SIZE];

    if (msize > MAXSIZE)
        msize = MAXSIZE;

    channel_sendbufferto(source, EVENT_P9P, p9p_mkrversion(buffer, p9p_read2(p9p, P9P_OFFSET_TAG), msize, "9P2000"), buffer);

}

static void on9pwalk(unsigned int source, struct p9p_event *p9p)
{

    struct request *request = &requests[0];
    unsigned int status;
    char buffer[MESSAGE_SIZE];

    file_link(FILE_G1);

    status = walk(source, request, p9p_read2(p9p, P9P_OFFSET_DATA + 10), (char *)p9p + P9P_OFFSET_DATA + 12);

    file_unlink(FILE_G1);

    if (status == OK)
        channel_sendbufferto(source, EVENT_P9P, p9p_mkrwalk(buffer, p9p_read2(p9p, P9P_OFFSET_TAG), 0, 0), buffer);
    else
        channel_sendbufferto(source, EVENT_P9P, p9p_mkrerror(buffer, p9p_read2(p9p, P9P_OFFSET_TAG), "File not found"), buffer);

}

static void onp9p(unsigned int source, void *mdata, unsigned int msize)
{

    struct p9p_event *p9p = mdata;

    if (p9p_read4(p9p, P9P_OFFSET_SIZE) < P9P_OFFSET_DATA)
    {

        channel_sendstring(EVENT_DATA, "Error: Packet is too small\n");

        return;

    }

    if (p9p_read4(p9p, P9P_OFFSET_SIZE) > MAXSIZE)
    {

        channel_sendstring(EVENT_DATA, "Error: Packet is too big\n");

        return;

    }

    if (p9p_read4(p9p, P9P_OFFSET_SIZE) != msize)
    {

        channel_sendstring(EVENT_DATA, "Error: Packet has incorrect size\n");

        return;

    }

    switch (p9p_read1(p9p, P9P_OFFSET_TYPE))
    {

    case P9P_TOPEN:
        on9popen(source, p9p);

        break;

    case P9P_TREAD:
        on9pread(source, p9p);

        break;

    case P9P_TVERSION:
        on9pversion(source, p9p);

        break;

    case P9P_TWALK:
        on9pwalk(source, p9p);

        break;

    default:
        channel_sendstring(EVENT_DATA, "Error: Packet has unknown type\n");

        break;

    }

}

void init(void)
{

    if (!file_walk2(FILE_G0, "system:service/fd0"))
        return;

    if (!file_walk2(FILE_G1, "system:block/if:0/data"))
        return;

    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_P9P, onp9p);

}

