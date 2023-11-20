#include "common.h"
#include "vm.h"
#include "timer.h"

extern int DEBUG_PRINT_CODE;
extern int DEBUG_TRACE_EXECUTION;

FILE * INPUT;

static const char * EXTENSION = ".uwu";
static int EXTENSION_LENGTH = 4;

static void usage_error()
{
    fprintf(stderr, "usage: uwu <path> [-p | -e]\n");
    exit(64);
}

static void repl()
{
    char line[1024];
    while (true)
    {
        printf("> ");

        if (!fgets(line, sizeof(line), stdin))
        {
            printf("\n");
            break;
        }

        {
            //Timer timer;
            interpret(line);
        }

        printf("\n");
        reset_frame();
    }
}

static bool is_uwu_file(const char * path)
{
    int length = strlen(path);

    if (length < EXTENSION_LENGTH + 1) return false;
    if (memcmp(path + (length - EXTENSION_LENGTH), EXTENSION, EXTENSION_LENGTH) == 0)
        return true;
    return false;
}

static char * read_file(const char * path)
{
    if (is_uwu_file(path))
    {
        INPUT = fopen(path, "rb");
    }
    else
    {
        fprintf(stderr, "error: '%s' is not a '.uwu' file", path);
        exit(66);
    }

    if (!INPUT)
    {
        fprintf(stderr, "error: unable to open file \"%s\"\n", path);
        exit(67);
    }

    fseek(INPUT, 0L, SEEK_END);
    size_t file_size = ftell(INPUT);
    rewind(INPUT);

    char * buffer = (char *)malloc(file_size + 1);

    if (!buffer)
    {
        fprintf(stderr, "error: not enough memory to read file \"%s\".\n", path);
        exit(68);
    }

    size_t bytes_read = fread(buffer, sizeof(char), file_size, INPUT);

    if (bytes_read < file_size)
    {
        fprintf(stderr, "error: could not read file \"%s\".\n", path);
        exit(69);
    }

    buffer[bytes_read] = '\0';

    fclose(INPUT);
    return buffer;
}

static void read_flags(int argc, const char ** argv)
{
    for (int i = 2; i < argc; i++)
    {
        if (argv[i][0] == '-' && strlen(argv[i]) == 2)
        {
            switch (argv[i][1])
            {
                case 'p':
                    if (!DEBUG_PRINT_CODE)
                        DEBUG_PRINT_CODE = 1;
                    else
                        usage_error();
                    break;
                case 'e':
                    if (!DEBUG_TRACE_EXECUTION)
                        DEBUG_TRACE_EXECUTION = 1;
                    else
                        usage_error();
                    break;
            }
        }
        else
        {
            usage_error();
        }
    }
}

static void run(int argc, const char ** argv)
{
    if (argc == 1)
    {
        repl();
        return;
    }
    else if (argc > 4)
    {
        fprintf(stderr, "usage: uwu <path> [-p | -e]\n");
        exit(64);
    }

    read_flags(argc, argv);

	char * source = read_file(argv[1]);
	InterpretResult result = interpret(source);
	free(source);

	if (result == INTERPRET_COMPILE_ERROR) exit(70);
	if (result == INTERPRET_RUNTIME_ERROR) exit(71);
}

int main(int argc, const char ** argv)
{
    initVM();

    run(argc, argv);

    freeVM();

    return 0;
}
