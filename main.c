#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 255
#define MAX_VALUE 2147483647L
#define SPACE " \t\r\v\f"

typedef struct
{
    int h;
    int m;
    int node;
    int messages;
    int errors;
} Event;

enum
{
    READ_ERROR = -1,
    END_OF_FILE,
    LINE_OK,
    LINE_TOO_LONG,
    LINE_INVALID
};

static int read_line(FILE *file, char *line);
static int read_number(const char **text, int *number);
static const char *parse_event(const char *line, Event *event);
static int print_event(const Event *event);

int main(int argc, char *argv[])
{
    FILE *file;
    char line[MAX_LINE + 2];
    unsigned long line_number = 0;
    int status;

    if (argc != 2)
    {
        fputs("Укажите путь к файлу событий.\n", stderr);
        return EXIT_FAILURE;
    }

    file = fopen(argv[1], "rb");
    if (file == NULL)
    {
        fprintf(stderr, "Не удалось открыть файл: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    while ((status = read_line(file, line)) > END_OF_FILE)
    {
        Event event;
        const char *error;

        line_number++;
        if (status == LINE_TOO_LONG)
        {
            error = "строка длиннее 255 символов";
        }
        else if (status == LINE_INVALID)
        {
            error = "недопустимый символ";
        }
        else if (line[strspn(line, SPACE)] == '\0')
        {
            continue;
        }
        else
        {
            error = parse_event(line, &event);
        }

        if (error != NULL)
        {
            fprintf(stderr, "Строка %lu: %s.\n", line_number, error);
        }
        else if (print_event(&event) == EOF)
        {
            fputs("Ошибка вывода.\n", stderr);
            fclose(file);
            return EXIT_FAILURE;
        }
    }

    if (fclose(file) == EOF || status == READ_ERROR)
    {
        fputs("Ошибка чтения файла.\n", stderr);
        return EXIT_FAILURE;
    }

    if (fflush(stdout) == EOF)
    {
        fputs("Ошибка вывода.\n", stderr);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static int read_line(FILE *file, char *line)
{
    int count = 0;
    int invalid = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF && ch != '\n')
    {
        if (count < MAX_LINE + 1)
        {
            line[count++] = (char)ch;
        }
        else
        {
            count = MAX_LINE + 2;
        }

        if (ch == '\0')
        {
            invalid = 1;
        }
    }

    if (ch == EOF && ferror(file))
    {
        return READ_ERROR;
    }
    if (ch == EOF && count == 0)
    {
        return END_OF_FILE;
    }
    if (ch == '\n' && count > 0 && count <= MAX_LINE + 1 &&
        line[count - 1] == '\r')
    {
        count--;
    }
    if (count > MAX_LINE)
    {
        return LINE_TOO_LONG;
    }

    line[count] = '\0';
    return invalid ? LINE_INVALID : LINE_OK;
}

static int read_number(const char **text, int *number)
{
    char digits[MAX_LINE + 1];
    int length = 0;
    long value;

    if (**text < '0' || **text > '9' ||
        sscanf(*text, "%255[0-9]%n", digits, &length) != 1)
    {
        return 0;
    }

    errno = 0;
    value = strtol(digits, NULL, 10);
    if (errno == ERANGE || value > MAX_VALUE)
    {
        return 0;
    }

    *number = (int)value;
    *text += length;
    return 1;
}

static const char *parse_event(const char *line, Event *event)
{
    const char *text = line + strspn(line, SPACE);
    size_t spaces;

    if (!read_number(&text, &event->h) || event->h > 23 || *text != ':')
    {
        return "неверные часы или пропущено двоеточие";
    }
    text++;
    if (!read_number(&text, &event->m) || event->m > 59)
    {
        return "неверные минуты";
    }

    spaces = strspn(text, " \t");
    text += spaces;
    if (spaces == 0 || !read_number(&text, &event->node))
    {
        return "неверный номер узла";
    }
    spaces = strspn(text, " \t");
    text += spaces;
    if (spaces == 0 || !read_number(&text, &event->messages))
    {
        return "неверное количество сообщений";
    }
    spaces = strspn(text, " \t");
    text += spaces;
    if (spaces == 0 || !read_number(&text, &event->errors))
    {
        return "неверное количество ошибок";
    }
    if (text[strspn(text, SPACE)] != '\0')
    {
        return "лишние данные после последнего поля";
    }

    return NULL;
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
