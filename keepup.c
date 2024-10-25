#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <sys/wait.h>

enum {
    CMD_NOT_FOUND = 127,
};


int isAbsolutePath(char *path) {
    return path[0] == '/';
}

int executeCommand(char *argv[]) {
    int sts;
    int pidchild;
    int pidwait;

    switch (pidchild = fork()) {
        case -1:
            err(EXIT_FAILURE, "fork failed");
            break;
        case 0:
            execv(argv[0], argv);

            if (errno == ENOENT) {
                exit(CMD_NOT_FOUND);
            } else {
                err(EXIT_FAILURE, "execv failed");
            }
            break;
    }

    do {
        pidwait = wait(&sts);
        if (pidwait == pidchild) {
            if (WIFEXITED(sts)) {
                return WEXITSTATUS(sts);
            }
        }
    } while (pidwait != -1 && pidwait != pidchild);

    if (pidwait != pidchild) {
        warnx("extra child %d", pidwait);
    }
    return -1;
}

int main(int argc, char *argv[]) {
    int attempts = 0;
    int result;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    argc--;
    argv++;

    if (!isAbsolutePath(argv[0])) {
        fprintf(stderr, "keepup: the command %s is not an absolute path\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    for (;;) {
        result = executeCommand(argv);
        if (result == 0) {
            exit(EXIT_SUCCESS);
        }
        if (result == CMD_NOT_FOUND) {
            fprintf(stderr, "keepup: cannot execute %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        attempts++;
        fprintf(stderr, "keepup: the command %s exited ntimes: %d\n", argv[0], attempts);
        sleep(1);
    } 

    exit(EXIT_SUCCESS);
}
