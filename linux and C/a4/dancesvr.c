#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int port = 1234;
int listenfd;
char banner[] = "dancecard 1\r\n";

#define MAXHANDLE 50  /* maximum permitted handle size, not including \0 */

#define FOLLOW 0
#define LEAD 1
#define BOTH 2

struct dancer {
    int fd;
    struct in_addr ipaddr;
    char handle[MAXHANDLE + 1];  /* zero-terminated; handle[0]==0 if not set */
    int role;  /* -1 if not set yet */
    char buf[200];  /* data in progress */
    int bytes_in_buf;  /* how many data bytes already read in buf */
    struct dancer *partner;  /* null if not yet partnered */
    struct dancer *next;
} *dancers = NULL;

int nlead = 0, nfollow = 0, nboth = 0, someone_is_partnered = 0, total = 0;

extern void parseargs(int argc, char **argv);
extern void makelistener();
extern void newclient(int fd, struct sockaddr_in *r);
extern void clientactivity(struct dancer *p);
extern void removeclient(struct dancer *p);
extern void do_something(struct dancer *p, char *wherenewline);
extern void begindance();
extern char *memnewline(char *p, int size);  /* finds \r _or_ \n */
extern void who(struct dancer *p);
extern void partnerup(struct dancer *p,struct dancer *check);
extern void reset();
extern int partnercheck(struct dancer *p,struct dancer *check);
extern int leftdance();
extern void minus(struct dancer *p, int add);



int main(int argc, char **argv)
{
    int did_something,maxfd;
    struct dancer *p, *n;

    parseargs(argc, argv);
    makelistener();

    while (1) {

        did_something = 0;

        /* check if we have a full input line in memory for some client */
        for (p = dancers; p; p = n) {
            char *q = memnewline(p->buf, p->bytes_in_buf);
            n = p->next;  /* stash the pointer now in case 'p' is deleted */
            if (q) {
                do_something(p, q);
                did_something = 1;
            }
        }

        if (!did_something) {
            /* nothing left which has already been read -- call select */    
            fd_set fds;  
            maxfd = listenfd;
            FD_ZERO(&fds);
            FD_SET(listenfd, &fds);
            for(p = dancers; p; p = p->next){
                FD_SET(p->fd, &fds);
                if (p->fd > maxfd)
                maxfd = p->fd;
            }
            if (select(maxfd + 1, &fds, NULL, NULL, NULL) < 0) {
                perror("select");
            } else {
                for (p = dancers; p; p = p->next)
                if (FD_ISSET(p->fd, &fds))
                    break;
                /*
                 * it's not very likely that more than one client will drop at
                 * once, so it's not a big loss that we process only one each
                 * select(); we'll get it later...
                 */
                if (p)
                clientactivity(p);  /* might remove p from list, so can't be in the loop */
                if (FD_ISSET(listenfd, &fds)){
                    int fd;
                    struct sockaddr_in r;
                    socklen_t len = sizeof r;
                    if ((fd = accept(listenfd, (struct sockaddr *)&r, &len)) < 0) {
                        perror("accept");
                    } else {
                        printf("connection from %s\n", inet_ntoa(r.sin_addr));
                        fflush(stdout);
                        newclient(fd, &r);
                    }

                }
            }
            
        }
    }

    return(0);
}


void parseargs(int argc, char **argv)
{
    int c, status = 0;
    while ((c = getopt(argc, argv, "p:")) != EOF) {
        switch (c) {
        case 'p':
            port = atoi(optarg);
            break;
        default:
            status++;
        }
    }
    if (status || optind != argc) {
        fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
        exit(1);
    }
}


void makelistener()
{
// ... bind and listen ...
    struct sockaddr_in r;

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket");
    exit(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(port);

    if (bind(listenfd, (struct sockaddr *)&r, sizeof r)) {
    perror("bind");
    exit(1);
    }

    if (listen(listenfd, 5)) {
    perror("listen");
    exit(1);
    }
}


void newclient(int fd, struct sockaddr_in *r)
{
  
    struct dancer *new = malloc(sizeof(struct dancer));
    if(!new){
        fprintf(stderr, "out of memory!\n");  /* highly unlikely to happen */
        exit(1);
    }
    new->fd = fd;
    new->ipaddr = r->sin_addr;
    *(new->handle) = '\0';
    new->role = -1;
    write(fd, banner, strlen(banner));
    new->partner = NULL;
    new->next = dancers;
    dancers = new;

    
    // ...
}


void clientactivity(struct dancer *p)
{
    // printf("ClientActivity reached\n");
    char *q;
    int len;

    if((len = read(p->fd,p->buf, sizeof(p->buf))) <= 0){
        if(len < 0){
            perror("read");
        }
        else if(len == 0){
            // printf("The client has closed\n");
            // fflush(stdout);
            removeclient(p);
            
            return;
        }

    }

    // do an appropriate read() into p->buf, given the existing p->bytes_in_buf
    p->bytes_in_buf += len;

    if ((q = memnewline(p->buf, p->bytes_in_buf)))
        do_something(p, q);
    else if (p->bytes_in_buf == sizeof p->buf)
        /*
         * We don't have a newline, but our buffer is full, so we'd
         * better process it anyway.
         */
        do_something(p, p->buf + sizeof p->buf - 1);
}


void removeclient(struct dancer *p)
{
    char bye[75];
    strcpy(bye,p->handle);
    strcat(bye," bids you all good night.\r\n");
    struct dancer *check;
    for (check = dancers; check; check = check->next){
        if(check->fd != listenfd && check->fd != p->fd)
        write(check->fd,bye,strlen(bye));
    }

    total--;
    if(p->partner == NULL){
        if(p->role == FOLLOW){
            nfollow--;
        }
        else if(p->role == LEAD){
            nlead--;
        }

        else{
            nboth--;
        }    
    }
    

    if(p->partner != NULL){
        someone_is_partnered--;
        (p->partner)->partner = NULL;
        char *apology = "I'm sorry, your partner seems to have left the dance hall!\r\nYou'll need to find someone else to dance with.\r\n";
        write((p->partner)->fd,apology,strlen(apology));
        p->partner = NULL;

    }

    printf("disconnecting client %s\n", inet_ntoa(p->ipaddr));
    fflush(stdout);
    struct dancer **pp;

    /* find the (struct dancers *) which points to the item to be deleted */
    for (pp = &dancers; *pp && (*pp)->fd != p->fd; pp = &(*pp)->next)
        ;

    if (*pp && (*pp)->fd == p->fd) {
        close(p->fd);
        struct dancer *t = (*pp)->next;
        fflush(stdout);
        free(*pp);
        *pp = t;
    }
}


/* there is a command in the buffer; do it */
void do_something(struct dancer *p, char *wherenewline){
    // printf("Do somthing activated\n");
    int len,i;
    // Gets the length of message
    len = wherenewline - p->buf;
    // changes the \r or \n into a \0 to make it into a
    // of that size.i.e. dosen't read further
    *wherenewline = '\0';
    char *clrf = "\r\n";
    char *command [] = {"who","begin","debug"};
    struct dancer *check;

    if (len == 0)
        ;/* ignore blank lines */
    else if (p->handle[0] == '\0') {
        // set the handle...
        // printf("printing Handle:\n");
        // printf("%s\n", p->buf);
        char *error = "Sorry, that word is a command, so it can't be used as a handle.\r\n";
        char *already = "Sorry, someone is already using that handle.  Please choose another.\r\n";
        int errset = 0;
        for(i = 0; i < 3; i++){
            if(strcmp(command[i],p->buf) == 0){
                printf("refusing to accept handle '%s'\n", p->buf);
                write(p->fd,error,strlen(error));
                errset = 1;
                break;
            }
        }
        for (check = dancers; check; check = check->next){
             if(strcmp(check->handle,p->buf) == 0){
                printf("refusing to accept handle '%s'\n", p->buf);
                write(p->fd,already,strlen(already));
                errset = 1;
                break;
            }
        }
        if(errset == 0){
            strncpy(p->handle,p->buf,MAXHANDLE);
            write(p->fd,clrf,strlen(clrf));
            printf("set handle of fd %d to %s\n", p->fd, p->handle );
        }

    }

    else if (p->role == -1){
        // printf("reached here\n");
        // printf("The buf gives us %s\n", p->buf);
        // fflush(stdout);
        int errset = 1;
        char *lead[2] = {"lead", "l"};
        char *follow[2] = {"follow", "f"};
        char *both[2] = {"both", "b"};
        for(i = 0; i < 2; i++){
            if(strcmp(lead[i],p->buf) == 0){
                p->role = LEAD;
                printf("set role of fd %d to %d\n", p->fd, p->role);
                write(p->fd,clrf,strlen(clrf));
                nlead++;
                total++;
                errset = 0;
                break;
            }
        }
        for(i = 0; i < 2; i++){
            if(strcmp(follow[i],p->buf) == 0){
                p->role = FOLLOW;
                printf("set role of fd %d to %d\n", p->fd, p->role);
                write(p->fd,clrf,strlen(clrf));
                nfollow++;
                total++;
                errset = 0;
                break;
            }
        }
        for(i = 0; i < 2; i++){
            if(strcmp(both[i],p->buf) == 0){
                p->role = BOTH;
                printf("set role of fd %d to %d\n", p->fd, p->role);
                write(p->fd,clrf,strlen(clrf));
                errset = 0;
                nboth++;
                total++;
                break;
            }
        }

        if(errset == 1){
            // printf("Error caused\n");
            char *error = "Invalid role.  Type lead, follow, or both.\r\n";
            write(p->fd,error, strlen(error));
        }
        else{
            char *welcome = "Welcome to the dance!\r\n";
            write(p->fd,welcome,strlen(welcome));
            who(p);
            char join[75];
            strcpy(join,p->handle);
            strcat(join," has joined the dance!\r\n");
            for (check = dancers; check; check = check->next){
                if(check->fd != listenfd && check->fd != p->fd)
                write(check->fd,join,strlen(join));
            }
        }
    }

    else{
        int errset = 1;
        // printf("%s\n", p->buf);
        for(i = 0;i <3;i++){
            // printf("command is %s\n", command[i]);
            if(strcmp(p->buf,command[i]) == 0){
                if(i == 0){
                    who(p);
                }
                else if(i == 1){
                    begindance();
                }
                else{
                    char debug[80];
                    sprintf(debug, "nlead %d, nfollow %d, nboth %d\r\n",nlead,nfollow,nboth);
                    write(p->fd,debug,strlen(debug));
                }
                errset = 0;
                break;
            }
        }

        for (check = dancers; check; check = check->next){
            if(strcmp(check->handle,p->buf) == 0 ){
                if(strcmp(check->handle,p->handle) == 0){
                    char *dkm = "This is a couples dance -- you can't dance with yourself.\r\n";
                    write(p->fd,dkm,strlen(dkm));
                    errset = 0;
                    break;
                }
                else{
                    if(partnercheck(p,check) == 0){
                        partnerup(p,check);
                        if(nlead == 0){
                            if(nboth == 0 || nfollow == 0){
                                begindance();
                            }
                        }
                        else if (nboth == 0 && nfollow == 0){
                            begindance();
                        }
                    }
                    errset = 0;
                    break;    
                }
                
            }
        }

        if(errset == 1){
            char *error = "There is no dancer by that name.\r\n";
            write(p->fd,error,strlen(error));
        }

    }

    // ... etc...

    /* now remove this command from the buffer */
    /* p->buf[len] was either \r or \n.  How about p->buf[len+1]? */
    len++;
    if (len < p->bytes_in_buf && (p->buf[len] == '\r' || p->buf[len] == '\n'))
        len++;
    p->bytes_in_buf -= len;
    memmove(p->buf, p->buf + len, p->bytes_in_buf);
}


void begindance()
{
    static char message1[] = "Dance begins\r\n";
    static char message2[] = "Dance ends.  Your partner thanks you for the dance!\r\nTime to find a new partner.  Type 'who' for a list of available dancers.\r\n";
    struct dancer *p;
    for (p = dancers; p; p = p->next)
        write(p->fd, message1, sizeof message1 - 1);
    sleep(5);
    for (p = dancers; p; p = p->next) {
        write(p->fd, message2, sizeof message2 - 1);
        p->partner = NULL;
    }
    someone_is_partnered = 0;
    reset();
    // something to reset nlead, etc --
    //   I suggest counting again from scratch, probably as a separate function
}


char *memnewline(char *p, int size)  /* finds \r _or_ \n */
        /* This is like min(memchr(p, '\r'), memchr(p, '\n')) */
        /* It is named after memchr().  There's no memcspn(). */
{
    for (; size > 0; p++, size--){
        if (*p == '\r' || *p == '\n'){
            return(p);
        }
    }
    return(NULL);
}

void who(struct dancer *p){
    struct dancer *check;
    if(total == 1){
        char *none = "No one else is here yet, but I'm sure they'll be here soon!\r\n";
        write(p->fd,none,strlen(none));
    }
    else if(total > 1){
        char *avail="Unpartnered dancers are:\r\n";
        write(p->fd,avail,strlen(avail));
        for (check = dancers; check; check = check->next){
            if(p->fd != check->fd && !(check->partner)){
                if(check->role == FOLLOW && p->role == FOLLOW){
                    char send[MAXHANDLE + 25];
                    strcpy(send,"[");
                    strcat(send,check->handle);
                    strcat(send," only dances follow]\r\n");
                    write(p->fd,send,strlen(send));
                }
                else if(check->role == LEAD && p->role == LEAD){
                    char send[MAXHANDLE + 25];
                    strcpy(send,"[");
                    strcat(send,check->handle);
                    strcat(send," only dances lead]\r\n");
                    write(p->fd,send,strlen(send));
                }
                else{
                    char send[MAXHANDLE + 3];
                    strncpy(send,check->handle,MAXHANDLE);
                    strcat(send,"\r\n");
                    write(p->fd,send,strlen(send));
                }
            }
        }
    }
}

void partnerup(struct dancer *p,struct dancer *check){
    char checkcongrats[100];
    strcpy(checkcongrats,p->handle);
    strcat(checkcongrats," has asked you to dance.  You accept!\r\n");
    write(check->fd,checkcongrats,strlen(checkcongrats));
    char pcongrats[100];
    strcpy(pcongrats,check->handle);
    strcat(pcongrats, " accepts!\r\n");
    write(p->fd,pcongrats,strlen(pcongrats));
    p->partner = check;
    check->partner = p;
}

void reset(){
    nlead = 0;
    nboth = 0;
    nfollow = 0;
    struct dancer *p;
    for (p = dancers; p; p = p->next) {
        p->partner = NULL;
        if(p->role == FOLLOW){
            nfollow++;
        }
        else if(p->role == LEAD){
            nlead++;
        }
        else{
            nboth++;
        }
    }
}

int partnercheck(struct dancer *p,struct dancer *check){
    int p_role = p->role;
    int check_role = check->role;
    if(p_role == check_role){
        if(p->role == LEAD){
            char sorry[100];
            sprintf(sorry,"Sorry, %s can only dance lead.\r\n", check->handle);
            write(p->fd,sorry, strlen(sorry));
            return(1);
        }
        else if(p->role == FOLLOW){
            char sorry[100];
            sprintf(sorry,"Sorry, %s can only dance follow.\r\n", check->handle);
            write(p->fd,sorry, strlen(sorry));
            return(1);
        }
    }
    else{
        int oldnum = leftdance();
        // printf("old num is %d\n", oldnum);
        minus(p,0);
        minus(check,0);
        int newnum = leftdance();
        // printf("new num is %d\n", newnum);
        if(newnum > oldnum){
            minus(p,1);
            minus(check,1);
            char sorry[100];
            sprintf(sorry,"Please don't ask %s to dance, as that would leave people out.\r\n", check->handle);
            write(p->fd,sorry, strlen(sorry));
            return(1);
        }

    }

    return(0);
    
}

int leftdance(){
    int x;
    if(nlead>nfollow){
        x = nlead - nfollow;
        x-=nboth;
    }
    else{
        x = nfollow - nlead;
        x-=nboth;
    }
    if (x < 0)
        return 0;
    else
        return x;

}

void minus(struct dancer *p, int add){
    int *gp[] = {&nfollow, &nlead, &nboth};
    if(add == 0){
        (*gp[p->role])--;
    }
    else{
        (*gp[p->role])++;
    }
}