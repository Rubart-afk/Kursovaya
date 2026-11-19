#include <stdio.h>
#include <stdlib.h>

typedef struct
{
    int h;
    int m;
    int node;
    int messages;
    int errors;
} Event;

static int print_event(const Event *event);

int main(void)
{
    const Event example = {10, 15, 3, 120, 2};

    if (puts("Архив событий почтового сервера. Вариант 11.") == EOF ||
        puts("Пример записи:") == EOF ||
        print_event(&example) == EOF ||
        fflush(stdout) == EOF)
    {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int print_event(const Event *event)
{
    if (printf("%02d:%02d %d %d %d\n", event->h, event->m,
               event->node, event->messages, event->errors) < 0)
    {
        return EOF;
    }

    return 0;
}
