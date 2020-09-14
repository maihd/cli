#include <stdio.h>
#include "Cli.h"

int main(int argc, const char** argv)
{
    const CliOption options[] = {
        { "foo", CliType_NoArgument, 301 },
        { "bar", CliType_NoArgument, 302 },
        { NULL }
    };

    int type = 0;
    CliParser parser = CLI_PARSER_INIT;
    while ((type = Cli_parseOptions(&parser, argc, argv, false, "", options)) >= 0)
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