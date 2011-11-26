#include <lib/memory.h>

void *memory_clear(void *out, unsigned int count)
{

    char *op = out;

    for (; count; op++, count--)
        *op = 0;

    return out;

}

int memory_compare(const void *in1, const void *in2, unsigned int count)
{

    const char *ip1 = in1;
    const char *ip2 = in2;

    for (; count; ip1++, ip2++, count--)
    {

        if (*ip1 != *ip2)
            return *ip1 - *ip2;

    }

    return 0;

}

void *memory_copy(void *out, const void *in, unsigned int count)
{

    char *op = out;
    const char *ip = in;

    for (; count; op++, ip++, count--)
        *op = *ip;

    return out;

}

unsigned int memory_index(const void *in, char value, unsigned int count, unsigned int skip)
{

    const char *ip = in;

    for (; count; ip++, count--)
    {

        if (*ip == value)
        {

            if (skip)
                skip--;
            else
                break;

        }

    }

    return ip - (char *)in;

}

unsigned int memory_index_reversed(const void *in, char value, unsigned int count, unsigned int skip)
{

    const char *ip = in + count - 1;

    for (; count > 0; ip--, count--)
    {

        if (*ip == value)
        {

            if (skip)
                skip--;
            else
                break;

        }

    }

    return count - 1;

}

void *memory_replace(void *out, char value1, char value2, unsigned int count)
{

    char *op = out;

    for (; count; op++, count--)
    {

        if (*op == value1)
            *op = value2;

    }

    return out;

}

void *memory_set(void *out, char value, unsigned int count)
{

    char *op = out;

    for (; count; op++, count--)
        *op = value;

    return out;

}

