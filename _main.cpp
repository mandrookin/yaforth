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
#include <termios.h>

#define _getcwd getcwd

#define USE_READLINE

#ifdef USE_READLINE
#include <readline/readline.h>
#endif

char * readline_gets(char * buff, int sz, FILE * fp);

static struct termios old_tio;
static struct termios new_tio;
static char prompt[80];

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
int * return_next_instruction_pointer_32bits()
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

        char* ptr = readline_gets(line, max_line_size - 1, fp);
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

void load_term_settings()
{
        tcgetattr(STDIN_FILENO, &old_tio);
        new_tio = old_tio;
        new_tio.c_lflag &= (~ICANON & ~ECHOE);
}

void restore_term_settings()
{
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

void enter_canonical_state()
{
        tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
}

void leave_canonical_state()
{
        tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
}

int read_key()
{
    int ch;
#if _WIN32
    ch = getc(stdin);
#else
    leave_canonical_state();
    ch = getc(stdin);
    enter_canonical_state();
#endif
    return ch;
}

#ifdef USE_READLINE

char * readline_gets(char * buff, int sz, FILE * fp)
{
    static char *line_read = (char *)NULL;
    char  * line_ptr;

    if (line_read)
    {
        free(line_read);
        line_read = (char *)NULL;
    }
    line_ptr = line_read = readline(prompt);

    if (line_ptr)
    {
/*
        while (isspace(*line_ptr))
            line_ptr++;
        if (*line_ptr) {
            int res;
            while ((res = history_search_pos(line_ptr, 1, 0)) >= 0)
                remove_history(res);
            add_history(line_ptr);
        }
*/
        snprintf(buff, sz, "%s\n", line_ptr);
//printf("GOT: %s\n", line_ptr);
    }
    else {
        buff[0] = 0;
    }
    return buff;
}

#else
char * readline_gets(char * buff, int sz, FILE * fp)
{
    fprintf(stdout,"%s\n", prompt);
    return fgets(buff, sz, fp);
}
#endif

void show_status(state_t state)
{
    if (ansi_colors)
    {
        if (state == error)
            strncpy(prompt,"\033[0;35m" "Error\n" "\033[0;37m", sizeof(prompt));
        else if (get_stack_size() != 0)
            snprintf(prompt, sizeof(prompt) , "\033[0;32m" "Ok %d\n"  "\033[0;37m", get_stack_size());
        else
            strncpy(prompt, "\033[0;33m" "Ok\n" "\033[0;37m", sizeof(prompt));
    }
    else
    {
        if (state == error)
            strncpy(prompt,"Error\n", sizeof(prompt));
        else if (get_stack_size() != 0)
            snprintf(prompt, sizeof(prompt) , "Ok %d\n", get_stack_size());
        else
            strncpy(prompt,"Ok\n", sizeof(prompt));
    }
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

    if (interactive)
    {
        load_term_settings();
    }
    

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

                try 
                {
                   state = read_stream(fp);
                }
//                catch(char const *exception)
                catch(char const &exception)
                {
                   fprintf(stderr, "EXCEPRION: %s\n", &exception);
                }
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
        restore_term_settings();
    }
    else if(state != error)
        if(generate_assembler)
            generate_asm_code(generated_name);

    return result;
}
