#include "yaforth.h"

static bool                         interactive;
static bool                         generate_assembler = false;

#ifdef _WIN32
#define  _CRT_SECURE_NO_WARNINGS

#include <direct.h>
#include <io.h>
#include <Windows.h>
#include <conio.h>

#define isatty _isatty
#define getch _getch

#else

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#define _getcwd getcwd

unsigned char getch()
{
    return getc(stdin);
}
struct termios old_tio;

#endif

constexpr unsigned int hash(const char* s, int off = 0) {
    return !s[off] ? 5381 : (hash(s, off + 1) * 33) ^ s[off];
}


state_t init();
void generate_asm_code(const char*);

int intro()
{
    const int max_line_size = 1024;
    char* line = (char*)alloca(max_line_size);
    line[max_line_size - 1] = 0;

    printf("Welcome to Yet Another Forth!\n");
    char* cur_dir = _getcwd(line, max_line_size);
    printf("Current direcory is %s\n", line);
    printf("This is interactive mode.\nType bye and press enter to exit or\nuse Forth syntax>\n");
    return 0;
}

// Do not uncomment
#if VIRTUAL_MACHINE_STUB_AND_TESTS
int * return_next_instruction_pointer()
{
    _asm {
        mov     eax, [ebp+4]
    }
}

void test_write_to_code_segment() {
    int* ptr = return_next_instruction_pointer();
    DWORD old;
    result = VirtualProtect( ptr, 4096, PAGE_EXECUTE_READWRITE, &old);
    *ptr = 0xfeedface;
}
#endif

#define SKIP_PREFIX \
if (wait_prefix) { \
    if (*ptr == -17) \
        ptr += 3; \
    wait_prefix = false; \
}

state_t read_stream(FILE * fp)
{
    const int max_line_size = 1024;
    char* line = (char*)alloca(max_line_size);
    line[max_line_size - 1] = 0;

    bool wait_prefix = true;
    state_t state = neutral;

    while (!feof(fp) && state != finish)
    {
        if (state == error && !interactive)
            break;

        char* ptr = fgets(line, max_line_size - 1, fp);
        if (ptr)
        {
            SKIP_PREFIX
                state = forth(ptr);
        }
        if (fp == stdin) {
            void show_status(state_t state);
            show_status(state);
        }
    }
    return state;
}

int main(int argc, char* argv[])
{
    FILE* fp = nullptr;
    int result = 0;
    int stream_count = 0;
    state_t state = neutral;
    const char* generated_name = nullptr;
            
#if _WIN32
    SetConsoleOutputCP(65001);
#endif

    interactive = isatty(0);

#ifndef _WIN32
    if (interactive)
    {
        struct new_tio;
        tcgetattr(STDIN_FILENO, &old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= (~ICANON & ~ECHOE);
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    }
#endif

    init();


        for (int i = 1; i < argc && state == neutral; i++)
        {
            if (*argv[i] == '-') {
                switch (hash(argv[i]))
                {
                case hash("-ansi"):
                    ansi_colors = !ansi_colors;
                    continue;
                case hash("-a"):
                    generate_assembler = !generate_assembler;
                    continue;
                default:
                    fprintf(stderr, "unsupported key: %s\n", argv[i]);
                    i = argc;
                    continue;
                }
            }
            else
            {
                fp = fopen(argv[i], "rt");
                if (!fp) {
                    printf("Unable open file: %s\n", argv[i]);
                    exit(-1);
                }
                if (!generated_name)
                    generated_name = argv[i];
                interactive = false;
                state = read_stream(fp);
                fclose(fp);
                stream_count++;
            }
        }

        if (stream_count == 0) {
            intro();
            fp = stdin;
            read_stream(fp);
        }

    if (interactive )
    {
#ifndef _WIN32
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
#endif
    }
    else if(state != error)
        if(generate_assembler)
            generate_asm_code(generated_name);

    return result;
}
