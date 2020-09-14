#pragma once

#include <string.h>
#include <stdbool.h>

typedef enum {
    CliType_NoArgument = 0,
    CliType_RequireArgument = 1,
    CliType_OptionalArgument = 2,
} CliType;

typedef struct {
    const char* arg;
    int         option;

    int         argIndex;
    int         optionIndex;

    // Internal
    int index, position, argc;
} CliParser;

typedef struct {
    const char* name;
    CliType     type;
    int         value;
} CliOption;

static CliParser CLI_PARSER_INIT = { NULL, 0, 1, -1, 1, 0, 0 };

static void Cli_permuteArgument(const char** __restrict argv, int index, int count)
{
    int i;
    const char* arg = argv[index];
    for (i = 0; i < count; i++)
    {
        argv[index - i] = argv[index - i - 1];
    }

    argv[index - i] = arg;
}

static int Cli_parseOptions(CliParser* __restrict parser, int argc, const char** __restrict argv, bool permute, const char* __restrict shortOptions, const CliOption* __restrict options)
{
    int optionValue = -1;

    if (permute)
    {
        while (parser->index < argc && (argv[parser->index][0] != '-' || argv[parser->index][1] == '\0'))
        {
            parser->index++;
            parser->argc++;
        }
    }

    parser->arg = NULL;
    parser->optionIndex = -1;
    
    int startIndex = parser->index;
    if (parser->index >= argc || argv[parser->index][0] != '-' || argv[parser->index][1] == '\0')
    {
        parser->index = parser->index - parser->argc;
        return -1;
    }

    if (argv[parser->index][0] == '-' && argv[parser->index][1] == '-')
    {
        if (argv[parser->index][2] == '\0')
        {
            Cli_permuteArgument(argv, parser->index, parser->argc);
            parser->index++; parser->index = parser->index - parser->argc;
            return -1;
        }

        optionValue = '?';
        parser->option = 0;
        parser->position = -1;
        if (options) // Parse named options
        {
            int exact = 0;
            int partial = 0;
            const CliOption* option = NULL;
            const CliOption* optionExact = NULL;
            const CliOption* optionPartial = NULL;

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
                        optionExact = &options[i];
                        exact++;
                    }
                    else
                    {
                        optionPartial = &options[i];
                        partial++;
                    }
                }
            }

            if (exact > 1 || (exact == 0 && partial > 1))
            {
                // Too much options
                return '?';
            }

            option = exact == 1 ? optionExact : (partial == 1 ? optionPartial : NULL);
            if (option)
            {
                parser->option = optionValue = option->value;
                parser->optionIndex = option - options;

                if (argv[parser->index][position] == '=')
                {
                    parser->arg = &argv[parser->index][position + 1];
                }

                if (option->type == CliType_RequireArgument && argv[parser->index][position] == 0)
                {
                    if (parser->index < argc - 1)
                    {
                        parser->arg = argv[++parser->index];
                    }
                    else
                    {
                        optionValue = ':'; // Missing option argument
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

        optionValue = parser->option = argv[parser->index][parser->position++];
        ptr = strchr((char*)shortOptions, optionValue);
        if (!ptr)
        {
            optionValue = '?';
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
            for (int j = startIndex; j < parser->index; j++)
            {
                Cli_permuteArgument(argv, j, parser->argc);
            }
        }
    }

    parser->argIndex = parser->index - parser->argc;
    return optionValue;
}
