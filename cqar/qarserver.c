#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <netdb.h>
#include <sys/socket.h>

#define LISTENPORT 2222
#define DEFHOST "gorge"

#define STDIN 0
#define STDOUT 1

#define WRITECONST(f,s) fputs("!", stdout); fflush(stdout); write((f),(s),sizeof(s)-1)

#define ASSERT(q)                                                          \
    if (!(q)) {                                                            \
        fprintf(stderr, "qar assert failed in %s, line %d: " #q "\n",      \
                __FILE__, __LINE__);                                       \
        perror("qar sys error (may not be related)");                      \
        exit(-1);                                                          \
    }

#define VERIFY(q)                                                          \
    if ((q) == -1) {                                                         \
        fprintf(stderr, "qar verify failed in %s, line %d: " #q "\n",      \
                __FILE__, __LINE__);                                       \
        perror("qar sys error");                                           \
        exit(-1);                                                          \
    }

typedef struct kidhandle_t {
    pid_t pid;
    int fdin;
    int fdout;
    FILE *in;
    FILE *out;
} kidhandle;

typedef int (doline_func)(kidhandle *, char *);

void printline(char *line)
{
    int li, di;
    char disp[1024];
    for (li=0, di=0; li<strlen(line) && di<sizeof(disp)-3; ++li, ++di)
    {
        if (line[li] < 27 && line[li] != '\n')
        {
            disp[di] = '^';
            disp[++di] = line[li] + 'A' - 1;
        }
        else
        {
            disp[di] = line[li];
        }
    }
    disp[di] = '\0';
    fputs(disp, stdout);
    fflush(stdout);
}

int x_login(kidhandle *kid, char *line)
{
    static nopage = 0;

    if (strcmp(line, "\rUsername: ") == 0)
    {
        WRITECONST(kid->fdin, "CHOUSER\r");
    }
    if (strcmp(line, "\rPassword: ") == 0)
    {
        WRITECONST(kid->fdin, "HKJLKJHH\r");
    }
    else if (strcmp(line, "\033Z") == 0)
    {
        WRITECONST(kid->fdin, "\033[?10c"); /* DEC LA100 */
    }
    else if (strcmp(line, "Which QAR database? ") == 0)
    {
        WRITECONST(kid->fdin, "osf_qar\r");
    }
    else if (strcmp(line, "QAR> ") == 0 && nopage == 0)
    {
        nopage = 1;
        WRITECONST(kid->fdin, "nopage\r");
    }
    else if (strcmp(line, "QAR> ") == 0)
    {
        return 0;
    }
    return 1;
}

void be_a_kid(int kidstdin, int kidstdout, char *host)
{
    VERIFY(dup2(kidstdin, STDIN));
    VERIFY(dup2(kidstdout, STDOUT));
    VERIFY(execl("/usr/bin/telnet", "telnet", host, NULL));
    /* --never reached-- */
}

void initstreams(kidhandle *kid)
{
    VERIFY(fcntl(kid->fdout, F_SETFL, O_NONBLOCK));
    ASSERT(kid->out = fdopen(kid->fdout, "r"));
    ASSERT(kid->in = fdopen(kid->fdin, "w"));
}

void readkid(kidhandle *kid, doline_func *doline)
{
    char buf[1024];
    fd_set readfds;
    struct timeval timeout;
    int hungry = 1;

    while (!feof(kid->out) && hungry)
    {
        if (!fgets(buf, sizeof(buf), kid->out))
        {
            ASSERT(errno == EAGAIN);

            FD_ZERO(&readfds);
            FD_SET(kid->fdout, &readfds);
            timeout.tv_sec = 4;
            timeout.tv_usec = 0;

            VERIFY(select(kid->fdout+1, &readfds, NULL, NULL, &timeout));
        }
        else
        {
            printline(buf);
            hungry = doline(kid, buf);
        }
    }
}

int initlisten()
{
    struct protoent *tcp;
    int fdlisten;
    struct sockaddr_in address;

    bzero((char *) &address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(LISTENPORT);

    VERIFY(fdlisten = socket(AF_INET, SOCK_STREAM, 0));
    VERIFY(setsockopt(fdlisten, SOL_SOCKET, SO_REUSEADDR, &1, sizeof(1)));
    VERIFY(bind(fdlisten, (struct sockaddr *)&address, sizeof(address)));
    VERIFY(listen(fdlisten, SOMAXCONN));

    return fdlisten;
}

void be_a_mom(kidhandle *kid)
{
    int fdlisten, fdclient;
    char line[128];
    FILE *client;
    
    fdlisten = initlisten();
    initstreams(kid);
    readkid(kid, x_login);

    while (1)
    {
        fputs("\nlistening...\n", stderr);
        VERIFY(fdclient = accept(fdlisten, NULL, 0));
        ASSERT(client = fdopen(fdclient, "r"));
        ASSERT(fgets(line, sizeof(line), client));
        line[strlen(line)-1] = '\r';
        fputs("sending command...\n", stderr);
        ASSERT(write(kid->fdin, line, strlen(line)) == strlen(line));
        readkid(kid, x_login);
    }
}

int main(int argc, char *argv[])
{
    int kidstdinpair[2], kidstdoutpair[2];
    kidhandle kid;

    VERIFY(pipe(kidstdinpair));
    VERIFY(pipe(kidstdoutpair));

    kid.pid = fork();
    if (kid.pid)
    {
        kid.fdout = kidstdoutpair[0];
        kid.fdin  = kidstdinpair[1];
        be_a_mom(&kid);
    }
    else
    {
        be_a_kid(kidstdinpair[0], kidstdoutpair[1], argc>1?argv[1]:DEFHOST);
    }
    return 0;
}
