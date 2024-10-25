#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <err.h>
#include <sys/wait.h>

/*
Escribe un programa keepup.c en C para Linux que reciba un comando como parámetro
y sus argumentos.
El primer parámetro que recibe keepup tiene que ser un path absoluto en el que se encuentre
el ejecutable y el resto de parámetros son los argumentos que va a recibir dicho comando en su ejecución.
El comando se ejecutará con sus argumentos y se esperará a que acabe.
Si acaba con éxito, keepup saldrá con éxito. Si el programa sale con
error, keepup informará de lo que ha sucedido por la salida de error (explicitando
el número de veces que ha sucedido) y rearrancará el programa.

Ejemplo:

$> ./keepup /usr/bin/ls .
a b c d
$> echo $?
0
$> ./keepup /usr/bin/ls noexiste
ls: cannot access 'noexiste': No such file or directory
keepup: the command /usr/bin/ls exited ntimes: 1
ls: cannot access 'noexiste': No such file or directory
keepup: the command /usr/bin/ls exited ntimes: 2
ls: cannot access 'noexiste': No such file or directory
keepup: the command /usr/bin/ls exited ntimes: 3
ls: cannot access 'noexiste': No such file or directory
keepup: the command /usr/bin/ls exited ntimes: 4
ls: cannot access 'noexiste': No such file or directory
keepup: the command /usr/bin/ls exited ntimes: 5
ls: cannot access 'noexiste': No such file or directory
keepup: the command /usr/bin/ls exited ntimes: 6
ctrl^c
$> touch noexiste
$> ./keepup /usr/bin/ls noexiste
noexiste
$> ./keepup /usr/bin/blabla
keepup: cannot execute /usr/bin/blabla
$> echo $?
1
*/

int executeCommand(char* argv[]){
    
    int sts;
    int pidchild;
    int pidwait;

    switch (pidchild = fork())
    {
    case -1:
        err(EXIT_FAILURE, "fork failed");
        break;
    case 0:
        execv(argv[0], argv);
        err(EXIT_FAILURE, "execv failed");
        break;
    }

    do {
        pidwait = wait(&sts);
        if (WIFEXITED(sts)) {
            if (WEXITSTATUS(sts) == 0){
                return 0;
            }
            return -1;
        }
    } while (pidwait != -1 && pidwait != pidchild);
    if (pidwait != pidchild){
        warnx("extra child %d", pidwait);
    }
    return -1;
}


int main(int argc, char* argv[]){
    int attempts = 0;
    int result;

    if(argc < 2){
        fprintf(stderr, "Usage: %s <command> [args]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    argc--;
    argv++;

    do {
        result = executeCommand(argv);
        if (result == -1){
            attempts++;
            fprintf(stderr, "keepup: the command %s exited ntimes: %d\n", argv[0], attempts);
            sleep(1);
        } else if (result == -2){
            fprintf(stderr, "keepup: cannot execute %s\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    } while (result == -1);


    exit(EXIT_SUCCESS);
}