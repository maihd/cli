#pragma once

#include <string.h>
#include <stdbool.h>

enum {
    CLI_NO_ARGUMENT = 0,
    CLI_REQUIRE_ARGUMENT = 1,
    CLI_OPTIONAL_ARGUMENT = 2,
};

typedef struct {
    const char* arg;
    int         option;

    int         arg_index;
    int         option_index;

    // Internal
    int index, position, argc;
} CliParser;

typedef struct {
    const char* name;
    int arg_type;
    int value;
} CliOption;

static CliParser CLI_PARSER_INIT = { NULL, 0, 1, -1, 1, 0, 0 };

static void cli_permute_arg(const char** __restrict argv, int index, int count)
{
    int i;
    const char* arg = argv[index];
    for (i = 0; i < count; i++)
    {
        argv[index - i] = argv[index - i - 1];
    }

    argv[index - i] = arg;
}

static int cli_parse_options(CliParser* __restrict parser, int argc, const char** __restrict argv, bool permute, const char* __restrict short_options, const CliOption* __restrict options)
{
    int option_value = -1;

    if (permute)
    {
        while (parser->index < argc && (argv[parser->index][0] != '-' || argv[parser->index][1] == '\0'))
        {
            parser->index++;
            parser->argc++;
        }
    }

    parser->arg = NULL;
    parser->option_index = -1;
    
    int start_index = parser->index;
    if (parser->index >= argc || argv[parser->index][0] != '-' || argv[parser->index][1] == '\0')
    {
        parser->index = parser->index - parser->argc;
        return -1;
    }

    if (argv[parser->index][0] == '-' && argv[parser->index][1] == '-')
    {
        if (argv[parser->index][2] == '\0')
        {
            cli_permute_arg(argv, parser->index, parser->argc);
            parser->index++; parser->index = parser->index - parser->argc;
            return -1;
        }

        option_value = '?';
        parser->option = 0;
        parser->position = -1;
        if (options) // Parse named options
        {
            int exact = 0;
            int partial = 0;
            const CliOption* option = NULL;
            const CliOption* option_exact = NULL;
            const CliOption* option_partial = NULL;

            // Find the end of the option name
            int position = 2; // position=2 because skip "--"
            while (argv[parser->index][position] != 0 && argv[parser->index][position] != '=')
            {
                position++;
            }

            int name_length = position - 2;
            const char* name = &argv[parser->index][2]; // Should skip "--"
            for (int i = 0; options[i].name != NULL; i++)
            {
                if (strncmp(name, options[i].name, name_length) == 0)
                {
                    if (options[i].name[name_length] == 0)
                    {
                        option_exact = &options[i];
                        exact++;
                    }
                    else
                    {
                        option_partial = &options[i];
                        partial++;
                    }
                }
            }

            if (exact > 1 || (exact == 0 && partial > 1))
            {
                // Too much options
                return '?';
            }

            option = exact == 1 ? option_exact : (partial == 1 ? option_partial : NULL);
            if (option)
            {
                parser->option = option_value = option->value;
                parser->option_index = option - options;

                if (argv[parser->index][position] == '=')
                {
                    parser->arg = &argv[parser->index][position + 1];
                }

                if (option->arg_type == CLI_REQUIRE_ARGUMENT && argv[parser->index][position] == 0)
                {
                    if (parser->index < argc - 1)
                    {
                        parser->arg = argv[++parser->index];
                    }
                    else
                    {
                        option_value = ':'; // Missing option argument
                    }
                }
            }
        }
    }
    else // A short option 
    {
        const char* ptr;
        if (parser->position == 0)
        {
            parser->position = 1;
        }

        option_value = parser->option = argv[parser->index][parser->position++];
        ptr = strchr((char*)short_options, option_value);
        if (!ptr)
        {
            option_value = '?';
        }
        else if (ptr[1] == ':') // This option need params
        {
            if (argv[parser->index][parser->position] == 0)
            {
                if (parser->index < argc - 1)
                {
                    parser->arg = argv[++parser->index];
                }
                else 
                {
                    parser->option = ':'; // Missing option argument
                }
            }
            else
            {
                parser->arg = &argv[parser->index][parser->position];   
            }
            parser->position = -1;
        }
    }

    if (parser->position < 0 || argv[parser->index][parser->position] == 0)
    {
        parser->index++;
        parser->position = 0;

        if (parser->argc > 0)
        {
            for (int j = start_index; j < parser->index; j++)
            {
                cli_permute_arg(argv, j, parser->argc);
            }
        }
    }

    parser->arg_index = parser->index - parser->argc;
    return option_value;
}
