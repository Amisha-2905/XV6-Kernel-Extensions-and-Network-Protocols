#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

char *syscall_name(int mask)
{
    for (int i = 0; i < 31; i++)
    {
        // printf("%d\n",i);
        if (mask == (1 << i))
        {
            switch (i)
            {
            case 1:
                return "fork";
            case 2:
                return "exit";
            case 3:
                return "wait";
            case 4:
                return "pipe";
            case 5:
                return "read";
            case 6:
                return "kill";
            case 7:
                return "exec";
            case 8:
                return "fstat";
            case 9:
                return "chdir";
            case 10:
                return "dup";
            case 11:
                return "getpid";
            case 12:
                return "sbrk";
            case 13:
                return "sleep";
            case 14:
                return "uptime";
            case 15:
                return "open";
            case 16:
                return "write";
            case 17:
                return "mknod";
            case 18:
                return "unlink";
            case 19:
                return "link";
            case 20:
                return "mkdir";
            case 21:
                return "close";
            case 22:
                return "waitx";
            case 23:
                return "getsyscount";
            case 24:
                return "sigalarm";
            case 25:
                return "sigreturn";
            default:
                return "unknown";
            }
        }
    }
    return "invalid";
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage: syscount <mask> command [args]\n");
        exit(1);
    }

    int mask = atoi(argv[1]);
    if (mask <= 0)
    {
        printf("Invalid mask\n");
        exit(1);
    }

    int pid = fork();
    if (pid < 0)
    {
        printf("Fork failed\n");
        exit(1);
    }

    if (pid == 0)
    {
        exec(argv[2], &argv[2]);
        printf("Exec failed\n");
        exit(1);
    }
    else
    {
        int status;
        int waited_pid = wait(&status);
        if (waited_pid < 0)
        {
            printf("Wait failed\n");
            exit(1);
        }
        int count = getsyscount(mask);
        if (count >= 0)
        {
            char *command = syscall_name(mask);
            if (strcmp("invalid", command) != 0)
            {
                printf("PID %d called %s %d times.\n", pid, syscall_name(mask), count);
                exit(0);
            }
            else
            {
                printf("Invalid mask!\n");
                exit(1);
            }
        }
        else
        {
            printf("Failed to get syscall count\n");
            exit(1);
        }
    }
}