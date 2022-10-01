#include <fudge.h>
#include <net.h>
#include <abi.h>
#include <socket.h>

static struct option options[32];
static struct socket local;
static struct socket remote;
static struct socket router;
static char inputbuffer[BUFFER_SIZE];
static struct ring input;

static void interpret(void *buffer, unsigned int count)
{

    char *data = buffer;

    if (data[0] == '/')
    {

        socket_send_tcp(FILE_G1, &local, &remote, &router, count - 1, data + 1);

    }

    else
    {

        char *text = "PRIVMSG #fudge :";
        char outputdata[BUFFER_SIZE];
        unsigned int offset = 0;

        offset += cstring_write(outputdata, BUFFER_SIZE, text, offset);
        offset += buffer_write(outputdata, BUFFER_SIZE, buffer, count, offset);

        socket_send_tcp(FILE_G1, &local, &remote, &router, offset, outputdata);

    }

}

static void resolve(void)
{

    unsigned int id = file_spawn("/bin/dns");

    if (id)
    {

        struct message message;

        message_init(&message, EVENT_OPTION);
        message_putstringz(&message, "domain");
        message_putstringz(&message, option_getstring(options, "domain"));
        channel_redirectback(id, EVENT_QUERY);
        channel_redirectback(id, EVENT_CLOSE);
        channel_sendmessageto(id, &message);
        channel_sendto(id, EVENT_MAIN);

        while (channel_pollfrom(id, &message) != EVENT_CLOSE)
        {

            if (message.header.event == EVENT_QUERY)
            {

                char *key = message.data.buffer;
                char *value = key + cstring_lengthz(key);

                if (cstring_match(key, "data"))
                    socket_bind_ipv4s(&remote, value);

            }

        }

    }

    else
    {

        channel_error("Could not spawn process");

    }

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;
    char buffer[BUFFER_SIZE];
    unsigned int count;

    if (!remote.resolved)
        return;

    switch (consoledata->data)
    {

    case '\0':
        break;

    case '\f':
        break;

    case '\t':
        break;

    case '\b':
    case 0x7F:
        break;

    case '\r':
        consoledata->data = '\n';

    case '\n':
        ring_write(&input, &consoledata->data, 1);
        channel_sendbuffer(EVENT_DATA, 1, &consoledata->data);

        count = ring_read(&input, buffer, BUFFER_SIZE);

        if (count)
            interpret(buffer, count);

        break;

    default:
        ring_write(&input, &consoledata->data, 1);
        channel_sendbuffer(EVENT_DATA, 1, &consoledata->data);

        break;

    }

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    socket_bind_ipv4s(&local, option_getstring(options, "local-address"));
    socket_bind_tcps(&local, option_getstring(options, "local-port"), 42);
    socket_bind_ipv4s(&router, option_getstring(options, "router-address"));

    if (!file_walk2(FILE_L0, option_getstring(options, "ethernet")))
        channel_warning("Could not open ethernet");

    if (file_walk(FILE_L1, FILE_L0, "addr"))
        socket_resolvelocal(FILE_L1, &local);
    else
        channel_error("Could not find address");

    resolve();

    if (file_walk(FILE_G1, FILE_L0, "data"))
    {

        char *request = "NICK jfu_fudge\nUSER jfu_fudge 0 * :Jens Fudge\nJOIN #fudge\n";
        char buffer[BUFFER_SIZE];
        unsigned int count;

        file_link(FILE_G1);
        socket_resolveremote(FILE_G1, &local, &router);
        socket_connect_tcp(FILE_G1, &local, &remote, &router);
        socket_send_tcp(FILE_G1, &local, &remote, &router, cstring_length(request), request);

        while ((count = socket_receive_tcp(FILE_G1, &local, &remote, &router, buffer, BUFFER_SIZE)))
            channel_sendbuffer(EVENT_DATA, count, buffer);

        socket_disconnect_tcp(FILE_G1, &local, &remote, &router);
        file_unlink(FILE_G1);

    }

    else
    {

        channel_error("Could not find data");

    }

    channel_close();

}

static void onoption(unsigned int source, void *mdata, unsigned int msize)
{

    char *key = mdata;
    char *value = key + cstring_lengthz(key);

    option_set(options, key, value);

}

void init(void)
{

    ring_init(&input, BUFFER_SIZE, inputbuffer);
    socket_init(&local);
    socket_init(&remote);
    socket_init(&router);
    option_add(options, "ethernet", "system:ethernet/if:0");
    option_add(options, "local-address", "10.0.5.1");
    option_add(options, "local-port", "50003");
    option_add(options, "router-address", "10.0.5.80");
    option_add(options, "domain", "");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_MAIN, onmain);
    channel_bind(EVENT_OPTION, onoption);

}

