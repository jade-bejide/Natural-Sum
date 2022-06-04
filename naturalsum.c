#include<stdio.h>
#include<errno.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>

#define STDIN 0
#define STDOUT 1


int ok;
char buffer[100];

typedef int fd;
typedef fd Pipe[2];
fd Reader(Pipe p) { return p[0]; }
fd Writer(Pipe p) { return p[1]; }

char* command = "/usr/bin/bc";
char* argv[] = {"bc", NULL};
char* envp[] = {NULL};

void check(int ok, char* where) {
    if (ok < 0) {
        fprintf(stderr, "Error in %s: %s\n", where, strerror(errno));
        exit(1);
    }
}

int evaluate(char* buffer, int len, fd reader, fd writer) {
  //want to read from stdout and write to stdout

  int reading = 1, writing = 1;

  while(writing) { //safe write
    ok = write(writer, buffer, strlen(buffer));
    check(ok, "Writing to writer");
    writing = 0;
  }

  while(reading) { //safe read
    ok = read(reader, buffer, len);
    check(ok, "Reading from reader");
    buffer[ok] = '\0'; //null terminate the string
    reading = 0;
  }

  return ok;
}

void createPipes(Pipe up, Pipe down) {
    ok = pipe(up); check(ok, "creating up pipe.");
    ok = pipe(down); check(ok, "creating down pipe.");  
}

void launchChildProcess(Pipe up, Pipe down) {
    ok = dup2(Reader(down), STDIN); check(ok, "redirects standard input to the down reader");
    ok = dup2(Writer(up), STDOUT); check(ok, "redirects standard output to the up writer");
    ok = execve(command, argv, envp); check(ok, "exec failure"); //here if ok < 0, then an error has occured and we need to check errno
}

void performParentProcess(char* buffer, Pipe up, Pipe down, int x) {

    strcpy(buffer, "1+2\n");
    ok = evaluate(buffer, sizeof(buffer), Reader(up), Writer(down)); check(ok, "evaluating sum.");

    for (int i=3; i<=x; i++) {
        int bufSize = strlen(buffer)-1;

        if (buffer[bufSize] == '\n') {
            buffer[bufSize] = '\0';
        } else {
           fprintf(stderr, "no new line from bc\n");
           exit(1);
        }

        sprintf(buffer + bufSize, "+%u\n", i);
        ok = evaluate(buffer, sizeof(buffer), Reader(up), Writer(down)); check(ok, "evaluating sum.");
    }
}

void naturalSum(char* buffer, int x) {
    Pipe up;
    Pipe down;

    createPipes(up, down);
    ok = fork(); check(ok, "forking the processs.");
    if (ok == 0) {
        /*CHILD PROCESS */
        launchChildProcess(up, down);
    }

    /*PARENT PROCESS */
    printf("Launches a child process, it has pid %u.\n", ok);
    performParentProcess(buffer, up, down, x);

    ok = dup2(STDOUT, Writer(down)); check(ok, "redirecting down pipe.");
    ok = dup2(STDIN, Reader(up)); check(ok, "redirecting up pipe.");
}

int main(int n, char* argv[]) {

    if (n < 2 || n > 2) {
        fprintf(stderr, "USE %s INTEGER\n", argv[0]);
        return 1;
    }

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    

    int val = atoi(argv[1]);

    if (val < 4 || val > 4472) {
        fprintf(stderr, "Please choose an integer in the valid range 4-4472\n");
        return 1;
    }
    
    naturalSum(buffer, atoi(argv[1]));
    printf("The result is %s\n", buffer);

    return 0;
}
