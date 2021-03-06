#include "shell.h"
#define BEGIN_CMD() int task_func = _task_func
#define END_CMD() if (task_func == 0) task_deinit(task_get()); else return

static int _task_func = 0;
static fat_entry_t cd;
static char buffer[BUFSZ];
static void cmd_echo();
static void cmd_exit();
static void cmd_cd();
static int ehandler_cd(fat_entry_t * entry);
static void cmd_dir();
static int ehandler_dir(fat_entry_t * entry);
static void cmd_cat();
static int ehandler_cat(fat_entry_t * entry);
static int shandler_cat(char * sector, int length);
static void (*find_procedure(char * cmd))();
extern char sector[SECTOR_SIZE];

void shell()
{
    char c;
    cd.fstClus = 0;
    while (1)
    {
        int p = 0;
        void (*f)();
        puts("$> ");
        while (1)
        {
            c = getc();
            if (c == 13)
            {
                buffer[p++] = '\0';
                ENTER();
                break;
            }
            else if (c == 8)
            {
                if (p > 0)
                {
                    p--;
                    putc(c);
                    putc(' ');
                    putc(c);
                }
            }
            else
            {
                putc(c);
                buffer[p++] = c;
            }
        }
        if ((f = find_procedure(buffer)) == 0)
        {
            puts("command not found!");
            ENTER();
            continue;
        }
        else
        {
            if (buffer[p-2] == '&')
            {
                p -= 2;
                buffer[p] = '\0';
                _task_func = 0;
                task_init(f, 0x1000);
            }
            else
            {
                _task_func = 1;
                f();
            }
        }
    }
}
static void (*find_procedure(char * cmd))()
{
    if (strncmp("echo", buffer, 4) == 0)
        return cmd_echo;
    if (strncmp("exit", buffer, 4) == 0)
        return cmd_exit;
    if (strncmp("dir", buffer, 3) == 0)
        return cmd_dir;
    if (strncmp("cd", buffer, 2) == 0)
        return cmd_cd;
    if (strncmp("cat", buffer, 3) == 0)
        return cmd_cat;
    return 0;
}

static void cmd_echo()
{
    int i;
    BEGIN_CMD();
    for(i = 5; buffer[i] != '\0'; i++)
        putc(buffer[i]);
    ENTER();
    END_CMD();
}

static void cmd_exit()
{
    task_deinit(task_get());
}

static void cmd_dir()
{
    BEGIN_CMD();
    fat_dir_read(&cd, ehandler_dir);
    END_CMD();
}
static int ehandler_dir(fat_entry_t * entry)
{
    if (IS_FREE(entry))
        return 0;
    if (ATTR_DIRECTORY(entry) || ATTR_ARCHIVE(entry))
    {
        puts(entry->name);
        ENTER();
    }
    return 0;
}

static void cmd_cd()
{
    BEGIN_CMD();
    if (fat_dir_read(&cd, ehandler_cd))
        END_CMD();
    puts("No such directory!");
    ENTER();
    END_CMD();
}
static int ehandler_cd(fat_entry_t * entry)
{
    if (IS_FREE(entry))
        return 0;
    if (ATTR_DIRECTORY(entry) || ATTR_ARCHIVE(entry))
        if (strncmp(entry->name, buffer+3, strlen(buffer+3)) == 0)
        {
            cd = *entry;
            return 1;
        }
    return 0;
}

static void cmd_cat()
{
    BEGIN_CMD();
    if (fat_dir_read(&cd, ehandler_cat))
        END_CMD();
    puts("No such file!");
    ENTER();
    END_CMD();
}
static int ehandler_cat(fat_entry_t * entry)
{
    if (IS_FREE(entry))
        return 0;
    if (ATTR_DIRECTORY(entry) || ATTR_ARCHIVE(entry))
        if (strncmp(entry->name, buffer+4, strlen(buffer+4)) == 0)
        {
            fat_file_read(entry, shandler_cat);
            ENTER();
            return 1;
        }
    return 0;
}

static int shandler_cat(char * sector, int length)
{
    int i;
    for (i = 0; i < length; i++)
        putc(sector[i]);
    return 0;
}
