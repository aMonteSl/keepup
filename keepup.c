#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <errno.h>
#include <sys/wait.h>

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
            // Verificar si el error es debido a que el archivo no existe
            if (errno == ENOENT) {
                exit(127); // 127 indica "comando no encontrado"
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
    return -1; // Código de error general
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

    do {
        result = executeCommand(argv);
        if (result == 0) {
            exit(EXIT_SUCCESS); // Comando terminó exitosamente
        }
        if (result == 127) { // Error de execv, comando no encontrado
            fprintf(stderr, "keepup: cannot execute %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }

        attempts++;
        fprintf(stderr, "keepup: the command %s exited ntimes: %d\n", argv[0], attempts);
        sleep(1);
    } while (1); // Reintenta hasta que el comando sea exitoso o termine por señal

    return EXIT_SUCCESS;
}
