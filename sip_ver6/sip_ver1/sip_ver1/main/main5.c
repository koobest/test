#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_heap_caps.h"
#include <netinet/in.h>
#include <eXosip2/eXosip.h>
#include "lwip/err.h"
#include "lwip/sys.h"
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip2.h>

void wifi_sta_start_up(void);
//_eXosip_read_message
//eXosip_execute

char *src_ip = "192.168.43.111";
char *dst_ip = "192.168.43.6";//client ip

static void uac(void *par)
{
    const char *tag = "UAC";
    vTaskDelay(1000/portTICK_PERIOD_MS);
    struct eXosip_t *context_eXosip;
    eXosip_event_t *je;
    osip_message_t *reg=NULL;
    osip_message_t *invite=NULL;
    osip_message_t *ack=NULL;
    osip_message_t *info=NULL;
    osip_message_t *message=NULL;

    int call_id = 0,dialog_id = 0;
    int i,flag = 0;
    int flag1=1;

    // char *identity="sip:140@127.0.0.1";   //UAC1，端口是15060
    // char *registar="sip:133@127.0.0.1:15061"; //UAS,端口是15061

    char source_call[64];
    char dest_call[64];
    memset(source_call, 0, 64);
    memset(dest_call, 0, 64);
    snprintf(source_call, 64, "sip:133@%s", src_ip);
    snprintf(dest_call, 64, "sip:140@%s:15061", dst_ip);

    // char *source_call="sip:133@192.168.200.90";
    // char *dest_call="sip:140@192.168.200.173:15061";
    //identify和register这一组地址是和source和destination地址相同的
    //在这个例子中，uac和uas通信，则source就是自己的地址，而目的地址就是uac1的地址
    char command;
    char *tmp = (char *)malloc(4096);
    printf("1 free heap: %d, size: %d\n", (int)heap_caps_get_free_size(1), sizeof(struct eXosip_t));
    context_eXosip = eXosip_malloc();
    printf("-1 free heap: %d\n", (int)heap_caps_get_free_size(1));
    //初始化
    i=eXosip_init(context_eXosip);
    printf("2 free heap: %d\n", (int)heap_caps_get_free_size(1));
    if(i!=0)
    {
        printf("UAC: Couldn't initialize eXosip!\n");
        vTaskDelete(NULL);
    }
    else
    {
        printf("UAC: eXosip_init successfully!\n");
    }
    vTaskDelay(500/portTICK_PERIOD_MS);
    //绑定uac自己的端口15060，并进行端口监听
    i=eXosip_listen_addr(context_eXosip, IPPROTO_UDP, src_ip,15060,AF_INET,0);
    printf("3 free heap: %d\n", (int)heap_caps_get_free_size(1));
    if(i != 0)
    {
        eXosip_quit(context_eXosip);
        printf("UAC: Couldn't initialize transport layer!\n");
        vTaskDelete(NULL);
    }
    printf("UAC: listen suc\n");
    flag=1;
    while(flag)
    {
        //输入命令
        printf("UAC: Please input the command:\n");
        vTaskDelay(1000/portTICK_PERIOD_MS);
        command = 'i';
        switch(command)
        {
        case 'r':
            printf("UAC: This modal is not completed!\n");
            break;
        case 'i'://INVITE，发起呼叫请求
            i=eXosip_call_build_initial_invite(context_eXosip, &invite,dest_call,source_call,NULL,"This is a call for conversation");
            if(i!=0)
            {
                printf("UAC: Initial INVITE failed!\n");
                break;
            }
            //符合SDP格式，其中属性a是自定义格式，也就是说可以存放自己的信息，
            //但是只能有两列，比如帐户信息
            //但是经过测试，格式vot必不可少，原因未知，估计是协议栈在传输时需要检查的
            snprintf(tmp,4096,
                      "v=0\r\n"
                      "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
                      "t=1 10\r\n"
                      "a=username:rainfish\r\n"
                      "a=password:123\r\n");

            osip_message_set_body(invite,tmp,strlen(tmp));
            osip_message_set_content_type(invite,"application/sdp");

            eXosip_lock(context_eXosip);
            i=eXosip_call_send_initial_invite(context_eXosip, invite); //invite SIP INVITE message to send
            eXosip_unlock(context_eXosip);

            //发送了INVITE消息，等待应答
            flag1=1;
            while(flag1)
            {
                vTaskDelay(100/portTICK_PERIOD_MS);
                je=eXosip_event_wait(context_eXosip, 0,200); //Wait for an eXosip event
                //(超时时间秒，超时时间毫秒)
                if(je==NULL)
                {
                    printf("UAC: No response or the time is over!\n");
                    break;
                }
                switch(je->type)   //可能会到来的事件类型
                {
                case EXOSIP_CALL_INVITE:   //收到一个INVITE请求
                    printf("UAC: a new invite received!\n");
                    break;
                case EXOSIP_CALL_PROCEEDING: //收到100 trying消息，表示请求正在处理中
                    printf("proceeding!\n");
                    break;
                case EXOSIP_CALL_RINGING:   //收到180 Ringing应答，表示接收到INVITE请求的UAS正在向被叫用户振铃
                    printf("UAC: ringing!\n");
                    printf("UAC: call_id is %d,dialog_id is %d \n",je->cid,je->did);
                    break;
                case EXOSIP_CALL_ANSWERED: //收到200 OK，表示请求已经被成功接受，用户应答
                    printf("UAC: ok!connected!\n");
                    call_id=je->cid;
                    dialog_id=je->did;
                    printf("UAC: call_id is %d,dialog_id is %d \n",je->cid,je->did);

                    //回送ack应答消息
                    eXosip_call_build_ack(context_eXosip, je->did,&ack);
                    eXosip_call_send_ack(context_eXosip, je->did,ack);
                    flag1=0; //推出While循环
                    break;
                case EXOSIP_CALL_CLOSED: //a BYE was received for this call
                    printf("UAC: the other sid closed!\n");
                    break;
                case EXOSIP_CALL_ACK: //ACK received for 200ok to INVITE
                    printf("UAC: ACK received!\n");
                    break;
                default: //收到其他应答
                    printf("UAC: other response!\n");
                    break;
                }
                eXosip_event_free(je); //Free ressource in an eXosip event
            }
            break;

        case 'h':   //挂断
            printf("UAC: Holded!\n");

            eXosip_lock(context_eXosip);
            eXosip_call_terminate(context_eXosip, call_id,dialog_id);
            eXosip_unlock(context_eXosip);
            break;

        case 'c':
            printf("UAC: This modal is not commpleted!\n");
            break;

        case 's': //传输INFO方法
            eXosip_call_build_info(context_eXosip, dialog_id,&info);
            snprintf(tmp,4096,"\nThis is a sip message(Method:INFO)");
            osip_message_set_body(info,tmp,strlen(tmp));
            //格式可以任意设定，text/plain代表文本信息;
            osip_message_set_content_type(info,"text/plain");
            eXosip_call_send_request(context_eXosip, dialog_id,info);
            break;

        case 'm':
            //传输MESSAGE方法，也就是即时消息，和INFO方法相比，我认为主要区别是：
            //MESSAGE不用建立连接，直接传输信息，而INFO消息必须在建立INVITE的基础上传输
            printf("UAC: the method : MESSAGE\n");
            eXosip_message_build_request(context_eXosip, &message,"MESSAGE",dest_call,source_call,NULL);
            //内容，方法，      to       ，from      ，route
            snprintf(tmp,4096,"This is a sip message(Method:MESSAGE)");
            osip_message_set_body(message,tmp,strlen(tmp));
            //假设格式是xml
            osip_message_set_content_type(message,"text/xml");
            eXosip_message_send_request(context_eXosip, message);
            break;

        case 'q':
            eXosip_quit(context_eXosip);
            printf("UAC: Exit the setup!\n");
            flag=0;
            break;
        }
    }

    return(0);
}

static void uas(void *par)
{
    const char *tag = "UAS: ";
    vTaskDelay(1000/portTICK_PERIOD_MS);
    struct eXosip_t *context_eXosip;
    eXosip_event_t *je = NULL;
    osip_message_t *ack = NULL;
    osip_message_t *invite = NULL;
    osip_message_t *answer = NULL;
    sdp_message_t *remote_sdp = NULL;
    int call_id, dialog_id;
    int i,j;
    int id;
    char *sour_call = "sip:140@192.168.200.90";
    char *dest_call = "sip:133@192.168.200.90:5001";//client ip
    char command;
    char *tmp = malloc(4096);
    int pos = 0;
    //初始化sip
    printf("4 free heap: %d\n", (int)heap_caps_get_free_size(1));
    context_eXosip = eXosip_malloc();
    //初始化
    printf("5 free heap: %d\n", (int)heap_caps_get_free_size(1));
    i=eXosip_init(context_eXosip);
    if(i != 0)
    {
        printf("UAS: Can't initialize eXosip!\n");
        vTaskDelete(NULL);
    }
    else
    {
        printf("UAS: eXosip_init successfully!\n");
    }
    vTaskDelay(1000/portTICK_PERIOD_MS);
    printf("6 free heap: %d\n", (int)heap_caps_get_free_size(1));
    i = eXosip_listen_addr(context_eXosip, IPPROTO_UDP, "192.168.200.90", 5000, AF_INET, 0);
    printf("7 free heap: %d\n", (int)heap_caps_get_free_size(1));
    if(i != 0)
    {
        eXosip_quit(context_eXosip);
        printf("UAS: uas: eXosip_listen_addr error!\nCouldn't initialize transport layer!\n");
        vTaskDelete(NULL);
    }
    printf("UAS: listen suc\n");
    for(;;)
    {
        vTaskDelay(100/portTICK_PERIOD_MS);
        //侦听是否有消息到来
        je = eXosip_event_wait(context_eXosip, 0,50);
        //协议栈带有此语句,具体作用未知
        eXosip_lock(context_eXosip);
        eXosip_default_action(context_eXosip,je);
        eXosip_automatic_refresh(context_eXosip);
        eXosip_unlock(context_eXosip);
        if(je == NULL)//没有接收到消息
            continue;
        // printf("the cid is %s, did is %s/n", je->did, je->cid);
        switch(je->type)
        {
        case EXOSIP_MESSAGE_NEW://新的消息到来
            printf("UAS: EXOSIP_MESSAGE_NEW!\n");
            if(MSG_IS_MESSAGE(je->request))//如果接受到的消息类型是MESSAGE
            {
                {
                    osip_body_t *body;
                    osip_message_get_body(je->request, 0, &body);
                    printf("UAS: I get the msg is: %s\n", body->body);
                    //printf("the cid is %s, did is %s/n", je->did, je->cid);
                }
                //按照规则，需要回复OK信息
                eXosip_message_build_answer(context_eXosip, je->tid, 200,&answer);
                eXosip_message_send_answer(context_eXosip, je->tid, 200,answer);
            }
            break;
        case EXOSIP_CALL_INVITE:
            //得到接收到消息的具体信息
            printf("UAS: Received a INVITE msg from %s:%s, UserName is %s, password is %s\n",je->request->req_uri->host,
                    je->request->req_uri->port, je->request->req_uri->username, je->request->req_uri->password);
            //得到消息体,认为该消息就是SDP格式.
            remote_sdp = eXosip_get_remote_sdp(context_eXosip, je->did);
            call_id = je->cid;
            dialog_id = je->did;

            eXosip_lock(context_eXosip);
            eXosip_call_send_answer(context_eXosip, je->tid, 180, NULL);
            i = eXosip_call_build_answer(context_eXosip, je->tid, 200, &answer);
            if(i != 0)
            {
                printf("UAS: This request msg is invalid!Cann't response!\n");
                eXosip_call_send_answer(context_eXosip, je->tid, 400, NULL);
            }
            else
            {
                snprintf(tmp, 4096,
                          "v=0\r\n"
                          "o=anonymous 0 0 IN IP4 0.0.0.0\r\n"
                          "t=1 10\r\n"
                          "a=username:rainfish\r\n"
                          "a=password:123\r\n");

                //设置回复的SDP消息体,下一步计划分析消息体
                //没有分析消息体，直接回复原来的消息，这一块做的不好。
                osip_message_set_body(answer, tmp, strlen(tmp));
                osip_message_set_content_type(answer, "application/sdp");

                eXosip_call_send_answer(context_eXosip, je->tid, 200, answer);
                printf("UAS: send 200 over!\n");
            }
            eXosip_unlock(context_eXosip);

            //显示出在sdp消息体中的attribute 的内容,里面计划存放我们的信息
            printf("UAS: the INFO is :\n");
            while(!osip_list_eol( &(remote_sdp->a_attributes), pos))
            {
                sdp_attribute_t *at;

                at =(sdp_attribute_t *) osip_list_get( &remote_sdp->a_attributes, pos);
                printf("%s : %s\n", at->a_att_field, at->a_att_value);//这里解释了为什么在SDP消息体中属性a里面存放必须是两列

                pos ++;
            }
            break;
        case EXOSIP_CALL_ACK:
            printf("UAS: ACK recieved!\n");
            // printf("the cid is %s, did is %s/n", je->did, je->cid);
            break;
        case EXOSIP_CALL_CLOSED:
            printf("UAS: the remote hold the session!\n");
            // eXosip_call_build_ack(dialog_id, &ack);
            //eXosip_call_send_ack(dialog_id, ack);
            i = eXosip_call_build_answer(context_eXosip, je->tid, 200, &answer);
            if(i != 0)
            {
                printf("UAS: This request msg is invalid!Cann't response!\n");
                eXosip_call_send_answer(context_eXosip, je->tid, 400, NULL);

            }
            else
            {
                eXosip_call_send_answer(context_eXosip, je->tid, 200, answer);
                printf("UAS: bye send 200 over!\n");
            }
            break;
        case EXOSIP_CALL_MESSAGE_NEW://至于该类型和EXOSIP_MESSAGE_NEW的区别，源代码这么解释的
            /*
            // request related events within calls(except INVITE)
                  EXOSIP_CALL_MESSAGE_NEW,          < announce new incoming request.
            // response received for request outside calls
                     EXOSIP_MESSAGE_NEW,          < announce new incoming request.
                     我也不是很明白，理解是：EXOSIP_CALL_MESSAGE_NEW是一个呼叫中的新的消息到来，比如ring trying都算，所以在接受到后必须判断
                     该消息类型，EXOSIP_MESSAGE_NEW而是表示不是呼叫内的消息到来。
                     该解释有不妥地方，仅供参考。
            */
            printf("UAS: EXOSIP_CALL_MESSAGE_NEW\n");
            if(MSG_IS_INFO(je->request) ) //如果传输的是INFO方法
            {
                eXosip_lock(context_eXosip);
                i = eXosip_call_build_answer(context_eXosip, je->tid, 200, &answer);
                if(i == 0)
                {
                    eXosip_call_send_answer(context_eXosip, je->tid, 200, answer);
                }
                eXosip_unlock(context_eXosip);
                {
                    osip_body_t *body;
                    osip_message_get_body(je->request, 0, &body);
                    printf("UAS: the body is %s\n", body->body);
                }
            }
            break;
        default:
            printf("UAS: Could not parse the msg!\n");
        }
    }
}

 #define IP_DST "192.168.43.6"
//#define IP_DST "192.168.200.90"

void udp_client_task(void *param)
{
    vTaskDelay(1000/portTICK_PERIOD_MS);
    struct sockaddr_in servaddr;
    int client_socket = -1;
    const int port = 15061;
    memset(&servaddr, 0, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(IP_DST);
    client_socket = socket(PF_INET, SOCK_DGRAM, 0);
    if (client_socket < 0) {
        printf("socket fail\n");
        return;
    }
    printf("client init done\n");
    char buf[50] = "hello world\0";
    while(1)
    {
        //printf("sync\n");
        sendto(client_socket, buf, 11, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}


void server_task(void *param)
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
                 printf("socket error\n");
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
                 printf("bind error\n");
        } else{
                printf("bind sucess\n");
        }
        while(1){
                printf("begin recv:\n");
                recv_num = recvfrom(sock_fd,recv_buf,sizeof(recv_buf),0,(struct sockaddr *)&addr_client,&client_len);
                if(recv_num <  0){
                        printf("bad\n");
                } else{
                        recv_buf[recv_num]='\0';
                        printf("recv sucess:%s\n",recv_buf);
                }
                printf("begin send:\n");
                send_num = sendto(sock_fd,recv_buf,recv_num,0,(struct sockaddr *)&addr_client,client_len);
                if(send_num < 0){
                        printf("sendto error");
                } else{
                        printf("send sucessful\n");
                }
        }
        close(sock_fd);
        return 0;
}

void app_main(void)
{
    printf("free heap: %d\n", (int)heap_caps_get_free_size(1));
    esp_netif_init();
    wifi_sta_start_up();
    printf("free heap: %d\n", (int)heap_caps_get_free_size(1));
    xTaskCreate(uac, "uac", 1024*6, NULL, 8, NULL);
    // xTaskCreate(uas, "uas", 1024*6, NULL, 8, NULL);
    // xTaskCreate(udp_client_task, "udp_client_task", 1024*6, NULL, 8, NULL);
    // xTaskCreate(server_task, "server_task", 1024*6, NULL, 8, NULL);
}
//find . -name "*" -exec touch '{}' \;
//sudo /lib/ufw/ufw-init stop