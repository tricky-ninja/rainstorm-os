#include "shell.h"
#include "klog.h"
#include "memory_utils.h"
#include "drivers/x86/keyboard/keyboard.h"
#include "memory_utils.h"

static shell_module *modules;
static size_t loaded_modules_count;

extern shell_module _shell_modules_start;
extern shell_module _shell_modules_end;

void shell_init()
{
    uint8_t *modules_start = (uint8_t *)&_shell_modules_start;
    uint8_t *modules_end = (uint8_t *)&_shell_modules_end;
    loaded_modules_count = modules_end - modules_start;
    if (loaded_modules_count % sizeof(shell_module)) 
    {
        klog_warn("shell_modules are misalligned!");
        loaded_modules_count = 0;
        return;
    }
    loaded_modules_count = loaded_modules_count / sizeof(shell_module);
    modules = &_shell_modules_start;
}


int shell_run(const char *module_name, char *args, multiboot_info *mb_info)
{
    if (module_name == NULL) return -1;
    for (size_t i = 0; i < loaded_modules_count; i++)
    {
        if (!strcmp(modules[i].name, module_name))
            return modules[i].handler(args, mb_info);
    }
    printf("%s is not valid shell command or module\n", module_name);
    return 0;
}

void shell_launch(char *cmd, multiboot_info *mb_info)
{
    char *args = strsplit(cmd, ' ');
    int exit_code = shell_run(cmd, args, mb_info);
    while (1)
    {
        printf("$ ");
        keyboard_get_line(cmd, 256);
        args = strsplit(cmd, ' ');
        if (strlen(cmd) == 0) continue;
        if (!strcmp(cmd, "exit"))
            break;
        if (!strcmp(cmd, "help"))
        {
            printf("-----------------------------------------------\n");
            for (size_t i = 0; i < loaded_modules_count; i++)
            {
                printf("%s - %s\n", modules[i].name, modules[i].desc);
            }
            printf("help - Display this menu\n");
            printf("exit - Exit kernel shell\n");
            printf("-----------------------------------------------\n");
            continue;
        }
        exit_code = shell_run(cmd, args, mb_info);
        if (exit_code != 0) printf("ERR: %d\n", exit_code);
    }
    printf("Exiting kernel shell\n");
}