#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pty.h>
#include <termios.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#define ERR_EXIT(m) \
    do { \
        perror(m);\
        exit(EXIT_FAILURE);\
    }while(0)
#define MAX_CHARNUM (1024)
#define MAX_RULES (10)
#define ENUM_TYPE_CASE(x)   case x: return(#x);
typedef enum action_type {
    ACTION_SEND,
    ACTION_EXE,
        ACTION_MAX,
}action_type_t;
typedef struct rule{
        char matchstr[MAX_CHARNUM];
        action_type_t action;
        char data[MAX_CHARNUM];
}rule_t;
static inline const char *type_to_string(enum action_type type)
{
    switch (type)
    {
        ENUM_TYPE_CASE(ACTION_SEND)
        ENUM_TYPE_CASE(ACTION_EXE)
    }
    return "Unsupported";
}

struct termios orig_term;
rule_t rule[MAX_RULES];
pid_t pid;
int status=0;
pid_t wait_id;
void die(int sig)
{
    wait_id=waitpid( pid, &status, 0 );
}

const char* strnocasestr(const char* str, const char* subStr)
{
    int len = strlen(subStr);
    if(len == 0)
    {
        return NULL;
    }
    while(*str)
    {
        if(strncasecmp(str, subStr, len) == 0)
        {
                return str;
        }
        ++str;
    }
    return NULL;
}

void handlemsg(char *msg, int fd)
{
    fd_set writefds;
    int i=0;
    int match_index = MAX_RULES;
    if(msg == NULL)
            ERR_EXIT("null pointer");

    printf("%s", msg);
    for(i=0; i<MAX_RULES; i++)
    {
            if(strnocasestr(msg,rule[i].matchstr ) != NULL)
            {
                    match_index = i;
                    break;
            }
    }
    if(!(match_index < MAX_RULES))
    {   
        return ;
    }
    switch(rule[match_index].action)
    {
        case ACTION_SEND:
        {
            FD_ZERO(&writefds);
            FD_SET(fd, &writefds);
            if (select(fd + 1, NULL, &writefds, NULL, NULL) < 0)
            {
                if (!(errno == EINTR || errno == EAGAIN))
                    exit(1);
            }
            if (FD_ISSET(fd, &writefds))
            {
                //display and send data
                printf("%s\r\n", rule[i].data);
                write(fd, rule[i].data, strlen(rule[i].data)+1);
                write(fd, "\n\r", strlen("\n\r")+1);
            }
        }
    break;
    case ACTION_EXE:
        //printf("%s ...",rule[match_index].data);
        system(rule[match_index].data);
    break;
    default:
        break;
    }
}
int addRule_from_file(const char *filename,int cur_num)
{
    FILE *fp;
    int  numread, index=cur_num;
    char matchstr[MAX_CHARNUM],action[MAX_CHARNUM],data[MAX_CHARNUM];

    fp = fopen(filename,"r");
    if(fp == NULL)
        return index;
    while((index < MAX_RULES) && ((numread = fscanf(fp," %[^,], %[^,], %[^\n]",matchstr,action,data)) == 3))  //blank in format is used to remove any blank when reading
    {
        strcpy(rule[index].matchstr,matchstr);
        if(strncasecmp(type_to_string(ACTION_SEND), action, strlen(type_to_string(ACTION_SEND))) == 0)
            rule[index].action = ACTION_SEND;
        else if(strncasecmp(type_to_string(ACTION_EXE), action, strlen(type_to_string(ACTION_EXE))) == 0)
            rule[index].action = ACTION_EXE;
        else
        {
            rule[index].action = ACTION_MAX;
            printf("wrong action type: %s, only surport ACTION_%s|%s\n",action,type_to_string(ACTION_SEND),type_to_string(ACTION_EXE));
        }
        strcpy(rule[index].data,data);
        index++;
    }
    return index;
}
int addRule_from_opt(const char *str,int cur_num)
{

    int  numread,readbytes, index=cur_num;
    char matchstr[MAX_CHARNUM],action[MAX_CHARNUM],data[MAX_CHARNUM];
    const char *p=str;

    while((index < MAX_RULES) && ((numread = sscanf(p," %[^,], %[^,], %[^;];%n",matchstr,action,data,&readbytes)) == 3))  //blank in format is used to remove any blank when reading
    {
        strcpy(rule[index].matchstr,matchstr);
        if(strncasecmp(type_to_string(ACTION_SEND), action, strlen(type_to_string(ACTION_SEND))) == 0)
            rule[index].action = ACTION_SEND;
        else if(strncasecmp(type_to_string(ACTION_EXE), action, strlen(type_to_string(ACTION_EXE))) == 0)
            rule[index].action = ACTION_EXE;
        else
        {
            rule[index].action = ACTION_MAX;
            printf("wrong action type: %s, only surport ACTION_%s|%s\n",action,type_to_string(ACTION_SEND),type_to_string(ACTION_EXE));
        }
        strcpy(rule[index].data,data);
        p+=readbytes;
        index++;
    }
    return index;
}
void dumpRule(int start, int end)
{
    int i=0;
    for(i=start; i<end; i++)
            printf("%60s %-20s%-50s\n",rule[i].matchstr,type_to_string(rule[i].action),rule[i].data);
}
void displayhelp()
{
        printf("Usage: autocom [option]\n \
available option:\n \
  -f read rule from file, each line contains one rule\n \
  -c \"command to execute\"\n \
  -r \"match string_1,action_1,data_1;match string_2,action_2,data_2;\", every rule end with ';'\n \
     current support action: ACTION_SEND, ACTION_EXE\n \
  -h display this help infomation\n");
  exit(0);
}
main(int argc, char **argv)
{
    int fd;
    const char *progname;
    const char *filename;
    const char * cmd;
    int  num=0;
    int opt;
    progname = argv[0];

    signal(SIGCHLD, die);
    while( (opt=getopt(argc, argv, "+f:c:r:Hh"))!=-1 ) {
    switch( opt ) {
            case 'f':
                    filename=optarg;
                    num = addRule_from_file(filename,num);
        break;
            case 'r':
                    num = addRule_from_opt(optarg, num);
            break;
            case 'c':
                    cmd = optarg;
            break;
            case 'H':
            case 'h':
                    displayhelp();
            break;
            case '?':
        printf("Unknown option: %c\n",(char)optopt);
                    displayhelp();
            case ':':
                    printf("invalid arguments\n");
                    displayhelp();
    break;
    }
    }
    num = addRule_from_file("./defrule.txt",num);
    //dumpRule(0, num);
    if (tcgetattr(0, &orig_term) < 0)
    {
        memset(&orig_term, 0, sizeof(struct termios));
        pid = forkpty(&fd, NULL, NULL, NULL);
    }
    else
        pid = forkpty(&fd, NULL, &orig_term, NULL);
    if (pid < 0)
        ERR_EXIT("fork");
    if(pid > 0)
    {
        char msg[4096];
        fd_set readfds;
        do
        {
            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            if (select(fd + 1, &readfds, NULL, NULL,NULL) > 0)
            {         
                if (FD_ISSET(fd, &readfds))
                {
                    if(num = read(fd, &msg, sizeof(msg)))
                    {
                        msg[num] = '\0';
                        handlemsg(msg,fd);
                    }
                }
            }
            }while( wait_id==0 || (!WIFEXITED( status ) && !WIFSIGNALED( status )) );
    }
    if(pid == 0)
    {
        system(cmd);
    }
    return 0;
}

