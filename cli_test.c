#include <stdio.h>
#include "cli.h"

int main(int argc, const char** argv)
{
    const CliOption options[] = {
        { "foo", CLI_NO_ARGUMENT, 301 },
        { "bar", CLI_NO_ARGUMENT, 302 },
        { NULL }
    };

    int type = 0;
    CliParser parser = CLI_PARSER_INIT;
    while ((type = cli_parse_options(&parser, argc, argv, false, "", options)) >= 0)
    {
        if (type == 301)
        {
            printf("has foo\n");
        }
        else if (type == 302)
        {
            printf("has bar\n");
        }
        else if (type == '?')
        {
            printf("unknown argument\n");
        }
        else if (type == ':')
        {
            printf("missing argument\n");
        }
    }

    printf("hello world\n");
    return 0;
}