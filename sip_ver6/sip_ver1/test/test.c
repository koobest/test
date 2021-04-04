#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

int ip_to_hostname(const char* ip);

void _main(void);

int main(int argc, char * * argv)
{
    struct sockaddr_in cli;
    char host[128];
    char service[128];
    int flags;
    int err;   
    cli.sin_family = AF_INET; /* IPv4 */
    cli.sin_port = htons(2049); /* the port of NFS */
    inet_pton(AF_INET, "127.0.0.1", &cli.sin_addr.s_addr); /* "220.181.111.86" is the address of baidu.com */
    flags = 0; /* NI_NUMERICHOST | NI_NUMERICSERV */    
    err = getnameinfo((struct sockaddr *)(&cli), sizeof(struct sockaddr),
                      host, sizeof(host), service, sizeof(service), flags);
    if(err != 0) {
        gai_strerror(err);
        exit(-1);
    }
    printf("host=%s, serv=%s\n", host, service);
    ip_to_hostname("127.0.0.1");
    _main();
    return 0;
}


int ip_to_hostname(const char* ip)
{
        int ret = 0;
 
        if(!ip)
        {
            printf("invalid params\n");
            return -1;
        }
 
        struct addrinfo hints;
        struct addrinfo *res, *res_p;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME | AI_NUMERICHOST;
        hints.ai_protocol = 0;
 
        ret = getaddrinfo(ip, NULL, &hints, &res);
        if(ret != 0)
        {
                printf("getaddrinfo: %s\n", gai_strerror(ret));
                return -1;
        }
 
        for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
        {
                char host[1024] = {0};
                ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NAMEREQD);
                if(ret != 0)
                {
                        printf("getnameinfo: %s\n", gai_strerror(ret));
                }
                else
                {
                        printf("hostname: %s\n", host);
                }
        }
 
        freeaddrinfo(res);
        return ret;
}

int hostname_to_ip(const char* hostname)
{
        int ret = 0;
 
        if(!hostname)
        {
                printf("invalid params\n");
                return -1;
        }
 
        struct addrinfo hints;
        struct addrinfo *res, *res_p;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_CANONNAME;
        hints.ai_protocol = 0;
 
        ret = getaddrinfo(hostname, NULL, &hints, &res);
        if(ret != 0)
        {
                printf("getaddrinfo: %s\n", gai_strerror(ret));
                return -1;
        }
 
        for(res_p = res; res_p != NULL; res_p = res_p->ai_next)
        {
                char host[1024] = {0};
                ret = getnameinfo(res_p->ai_addr, res_p->ai_addrlen, host, sizeof(host), NULL, 0, NI_NUMERICHOST);
                if(ret != 0)
                {
                        printf("getnameinfo: %s\n", gai_strerror(ret));
                }
                else
                {
                        printf("ip: %s\n", host);
                }
        }
 
        freeaddrinfo(res);
        return ret;
}

void _main(void)
{
    struct hostent *host = gethostbyname("127.0.0.1");
    if(!host){
        puts("Get IP address error!");
        system("pause");
        exit(0);
    }
    //别名
    for(int i=0; host->h_aliases[i]; i++){
        printf("Aliases %d: %s\n", i+1, host->h_aliases[i]);
    }
    //地址类型
    printf("Address type: %s\n", (host->h_addrtype==AF_INET) ? "AF_INET": "AF_INET6");
    //IP地址
    for(int i=0; host->h_addr_list[i]; i++){
        printf("IP addr %d: %s\n", i+1, inet_ntoa( *(struct in_addr*)host->h_addr_list[i] ) );
    }
    printf("%s\n", host->h_name);
}