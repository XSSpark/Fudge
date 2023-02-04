#include <fudge.h>
#include <abi.h>

#define JOBSIZE                         32
#define INPUTSIZE                       128

static char inputbuffer[INPUTSIZE];
static struct ring input;
static struct job_worker workers[JOBSIZE];
static struct job job;
static unsigned int keymod = KEYMOD_NONE;
static unsigned int escaped;

static void print(void *buffer, unsigned int count)
{

    file_writeall(FILE_G1, buffer, count);

}

static void printprompt(void)
{

    print("$ ", 2);

}

static unsigned int runslang(void *obuffer, unsigned int ocount, void *ibuffer, unsigned int icount)
{

    unsigned int id = file_spawn2(FILE_L0, FILE_G8, "/bin/slang");
    unsigned int offset = 0;

    if (id)
    {

        char data[MESSAGE_SIZE];
        unsigned int count;

        channel_redirect(id, EVENT_DATA);
        channel_redirect(id, EVENT_ERROR);
        channel_redirect(id, EVENT_CLOSE);
        channel_sendbuffer(id, EVENT_DATA, icount, ibuffer);
        channel_send(id, EVENT_MAIN);

        while ((count = channel_readfrom(id, EVENT_DATA, MESSAGE_SIZE, data)))
            offset += buffer_write(obuffer, ocount, data, count, offset);

    }

    else
    {

        channel_error("Could not spawn process");

    }

    return offset;

}

static void interpret(void)
{

    char ibuffer[INPUTSIZE];
    unsigned int icount = ring_read(&input, ibuffer, INPUTSIZE);

    if (icount)
    {

        char obuffer[1000];
        unsigned int ocount = runslang(obuffer, 1000, ibuffer, icount);

        if (ocount)
        {

            job_init(&job, workers, JOBSIZE);
            job_parse(&job, obuffer, ocount);

            if (job_spawn(&job, FILE_L1, FILE_G8))
            {

                struct message message;
                char data[MESSAGE_SIZE];

                job_listen(&job, EVENT_CLOSE);
                job_listen(&job, EVENT_DATA);
                job_listen(&job, EVENT_ERROR);
                job_listen(&job, EVENT_PATH);
                job_pipe(&job, EVENT_DATA);
                job_run(&job);

                while (job_pick(&job, &message, MESSAGE_SIZE, data))
                {

                    switch (message.event)
                    {

                    case EVENT_CLOSE:
                        job_close(&job, message.source);

                        break;

                    case EVENT_ERROR:
                        channel_dispatch(&message, data);

                        break;

                    case EVENT_DATA:
                        print(data, message_datasize(&message));

                        break;

                    case EVENT_PATH:
                        if (file_walk(FILE_L0, FILE_G8, data))
                            file_duplicate(FILE_G8, FILE_L0);

                        break;

                    }

                }

            }

            else
            {

                job_killall(&job);

            }

        }

    }

    printprompt();

}

static void complete(void)
{

    char buffer[INPUTSIZE];
    char prefix[INPUTSIZE];
    char resultdata[BUFFER_SIZE];
    struct ring result;
    struct message message;
    char data[MESSAGE_SIZE];
    unsigned int count = ring_readcopy(&input, buffer, INPUTSIZE);

    ring_init(&result, BUFFER_SIZE, resultdata);
    job_init(&job, workers, JOBSIZE);

    job.workers[0].program = "/bin/ls";
    job.count = 1;

    if (count)
    {

        unsigned int lastwordoffset = buffer_lastbyte(buffer, count, ' ');
        unsigned int searchoffset = lastwordoffset + buffer_lastbyte(buffer + lastwordoffset, count - lastwordoffset, '/');
        unsigned int searchcount = count - searchoffset;

        if (lastwordoffset)
        {

            if (searchoffset > lastwordoffset)
            {

                char path[INPUTSIZE];

                cstring_writezero(path, INPUTSIZE, buffer_write(path, INPUTSIZE, buffer + lastwordoffset, searchoffset - lastwordoffset - 1, 0));

                job.workers[0].paths[0] = path;
                job.workers[0].npaths = 1;

            }

        }

        else
        {

            job.workers[0].paths[0] = "/bin";
            job.workers[0].npaths = 1;

        }

        if (searchcount)
        {

            unsigned int prefixcount = cstring_writezero(prefix, INPUTSIZE, buffer_write(prefix, INPUTSIZE, (char *)buffer + searchoffset, searchcount, 0));

            if (prefixcount)
            {

                job.workers[1].program = "/bin/grep";
                job.workers[1].options[0].key = "prefix";
                job.workers[1].options[0].value = prefix;
                job.workers[1].noptions = 1;
                job.count = 2;

            }

        }

    }

    if (job_spawn(&job, FILE_L1, FILE_G8))
    {

        job_listen(&job, EVENT_CLOSE);
        job_listen(&job, EVENT_DATA);
        job_listen(&job, EVENT_ERROR);
        job_pipe(&job, EVENT_DATA);
        job_run(&job);

        while (job_pick(&job, &message, MESSAGE_SIZE, data))
        {

            switch (message.event)
            {

            case EVENT_CLOSE:
                job_close(&job, message.source);

                break;

            case EVENT_ERROR:
                channel_dispatch(&message, data);

                break;

            case EVENT_DATA:
                ring_write(&result, data, message_datasize(&message));

                break;

            }

        }

        if (ring_count(&result))
        {

            if (ring_each(&result, '\n') == ring_count(&result))
            {

                char *resultbuffer = resultdata + cstring_length(prefix);
                unsigned int resultcount = ring_count(&result) - cstring_lengthzero(prefix);

                ring_write(&input, resultbuffer, resultcount);
                print(resultbuffer, resultcount);

            }

            else
            {

                print("\n", 1);
                print(resultdata, ring_count(&result));
                printprompt();
                print(buffer, ring_readcopy(&input, buffer, INPUTSIZE));

            }

        }

    }

    else
    {

        job_killall(&job);

    }

}

static void clear(void)
{

    char sequence[2] = {0x1B, 'c'};
    char buffer[INPUTSIZE];
    unsigned int count = ring_readcopy(&input, buffer, INPUTSIZE);

    print(sequence, 2);
    printprompt();
    print(buffer, count);

}

static void onconsoledata(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_consoledata *consoledata = mdata;

    if (job_count(&job))
    {

        switch (consoledata->data)
        {

        case 0x03:
            job_sendfirst(&job, EVENT_TERM, 0, 0);

            break;

        default:
            job_sendfirst(&job, EVENT_CONSOLEDATA, msize, mdata);

            break;

        }

    }

    else
    {

        switch (escaped)
        {

        case 1:
            escaped = (consoledata->data == '[') ? 2 : 0;

            break;

        case 2:
            if (consoledata->data >= 0x20 && consoledata->data <= 0x2F)
            {

                /* intermediate bytes */

            }

            if (consoledata->data >= 0x30 && consoledata->data <= 0x3F)
            {

                /* parameter bytes */

            }

            if (consoledata->data >= 0x40 && consoledata->data <= 0x7E)
            {

                escaped = 0;

            }

            break;

        default:
            switch (consoledata->data)
            {

            case '\0':
                break;

            case 0x1B:
                escaped = 1;

                break;

            case '\f':
                clear();

                break;

            case '\t':
                complete();

                break;

            case '\b':
            case 0x7F:
                if (!ring_skipreverse(&input, 1))
                    break;

                print("\b \b", 3);

                break;

            case '\r':
                consoledata->data = '\n';

            case '\n':
                print(&consoledata->data, 1);
                ring_write(&input, &consoledata->data, 1);
                interpret();

                break;

            default:
                ring_write(&input, &consoledata->data, 1);
                print(&consoledata->data, 1);

                break;

            }

            break;

        }

    }

}

static void onkeypress(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keypress *keypress = mdata;
    struct keymap *keymap = keymap_load(KEYMAP_US);
    struct keycode *keycode = keymap_getkeycode(keymap, keypress->scancode, keymod);

    keymod = keymap_modkey(keypress->scancode, keymod);

    if (job_count(&job))
    {

        switch (keypress->scancode)
        {

        case 0x2E:
            if ((keymod & KEYMOD_CTRL))
                job_sendfirst(&job, EVENT_TERM, 0, 0);
            else
                job_sendfirst(&job, EVENT_CONSOLEDATA, keycode->length, keycode->value);

            break;

        default:
            job_sendfirst(&job, EVENT_CONSOLEDATA, keycode->length, keycode->value);

            break;

        }

    }

    else
    {

        switch (keypress->scancode)
        {

        case 0x0E:
            if (!ring_skipreverse(&input, 1))
                break;

            print("\b \b", 3);

            break;

        case 0x0F:
            complete();

            break;

        case 0x1C:
            print(keycode->value, keycode->length);
            ring_write(&input, keycode->value, keycode->length);
            interpret();

            break;

        default:
            ring_write(&input, keycode->value, keycode->length);
            print(keycode->value, keycode->length);

            break;

        }

    }

}

static void onkeyrelease(unsigned int source, void *mdata, unsigned int msize)
{

    struct event_keyrelease *keyrelease = mdata;

    keymod = keymap_modkey(keyrelease->scancode, keymod);

}
 
static void onerror(unsigned int source, void *mdata, unsigned int msize)
{

    print("Error: ", 7);
    print(mdata, msize);
    print("\n", 1);

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    if (!file_walk2(FILE_G0, option_getstring("input")))
        channel_warning("Could not get input device");

    if (!file_walk2(FILE_G1, option_getstring("output")))
        channel_warning("Could not get output device");

    printprompt();
    file_link(FILE_G0);

    while (channel_process());

    file_unlink(FILE_G0);

}

void init(void)
{

    file_duplicate(FILE_G8, FILE_PW);
    ring_init(&input, INPUTSIZE, inputbuffer);
    option_add("input", "system:console/if:0/event");
    option_add("output", "system:console/if:0/data");
    channel_bind(EVENT_CONSOLEDATA, onconsoledata);
    channel_bind(EVENT_KEYPRESS, onkeypress);
    channel_bind(EVENT_KEYRELEASE, onkeyrelease);
    channel_bind(EVENT_ERROR, onerror);
    channel_bind(EVENT_MAIN, onmain);

}

