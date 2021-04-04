#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<stdlib.h>


int main()
{
         int sock_fd;   //套接子描述符号
         int recv_num;
         int send_num;
         socklen_t client_len;
         char recv_buf[20];
         struct sockaddr_in  addr_serv;
         struct sockaddr_in  addr_client;//服务器和客户端地址
         sock_fd = socket(AF_INET,SOCK_DGRAM,0);
         if(sock_fd < 0){
                 perror("socket");
                 exit(1);
         } else{
 
                 printf("sock sucessful\n");
         }
         //初始化服务器断地址
         memset(&addr_serv,0,sizeof(struct sockaddr_in));
         addr_serv.sin_family = AF_INET;//协议族
         addr_serv.sin_port = htons(15061);
         addr_serv.sin_addr.s_addr = htonl(INADDR_ANY);//任意本地址
 
         client_len = sizeof(struct sockaddr_in);
         /*绑定套接子*/
         if(bind(sock_fd,(struct sockaddr *)&addr_serv,sizeof(struct sockaddr_in))<0 ){
                 perror("bind");
                 exit(1);

        } else{

                printf("bind sucess\n");
        }
        while(1){
                printf("begin recv:\n");
                recv_num = recvfrom(sock_fd,recv_buf,sizeof(recv_buf),0,(struct sockaddr *)&addr_client,&client_len);
                if(recv_num <  0){
                        printf("bad\n");
                        perror("again recvfrom");
                        exit(1);
                } else{
                        recv_buf[recv_num]='\0';
                        printf("recv sucess:%s\n",recv_buf);
                }
                printf("begin send:\n");
                send_num = sendto(sock_fd,recv_buf,recv_num,0,(struct sockaddr *)&addr_client,client_len);
                if(send_num < 0){
                        perror("sendto");
                        exit(1);
                } else{
                        printf("send sucessful\n");
                }
        }
        close(sock_fd);
        return 0;
}