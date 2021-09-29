#include <fudge.h>
#include <abi.h>

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    struct message message;

    channel_replystring(EVENT_DATA, "If you press f I will quit\n");

    while (channel_pollsourceevent(source, EVENT_CONSOLEDATA, &message))
    {

        struct event_consoledata *consoledata = (struct event_consoledata *)message.data.buffer;

        if (consoledata->data == 'f')
        {

            channel_replystring(EVENT_DATA, "\nYou clicked f, goodbye!\n");

            break;

        }

        else if (consoledata->data == '\r')
        {

            channel_replystring(EVENT_DATA, "\n");

        }

        else
        {

            struct message reply;

            message_init(&reply, EVENT_DATA);
            message_putbuffer(&reply, message_datasize(&message.header), message.data.buffer);
            channel_replymessage(&reply);

        }

    }

    channel_close();

}

void init(void)
{

    channel_bind(EVENT_MAIN, onmain);

}

