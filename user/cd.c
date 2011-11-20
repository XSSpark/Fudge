#include <fudge.h>

void get_path(char *buffer, char *arg)
{

    int cwd = file_open("dev", "/cwd");
    unsigned int count = file_read(cwd, 256, buffer);

    if (arg)
    {

        if (arg[0] == '/')
            string_write(buffer, arg);
        else
            string_write_concat(buffer, arg);

    }

    if (buffer[string_length(buffer) - 1] != '/')
        string_write_concat(buffer, "/");

    file_close(cwd);

}

void main(int argc, char *argv[])
{

    char path[256];

    if (argc == 1)
    {

        get_path(path, 0);
        file_write_format(FILE_STDOUT, path);
        file_write_byte(FILE_STDOUT, '\n');

        call_exit();

    }

    get_path(path, argv[1]);

    int new = file_open("initrd", path);

    if (!new)
    {

        file_write_format(FILE_STDOUT, "Directory does not exist.\n");
        file_close(new);

        call_exit();

    }

    file_close(new);

    int cwd = file_open("dev", "/cwd");
    file_write_format(cwd, path);
    file_close(cwd);

    call_exit();

}

