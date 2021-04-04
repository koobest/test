#include <stdio.h>
#include "string.h"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include <osipparser2/internal.h>
#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>

char *callids[] = {
    "#\n",
    "# valid field\n",
    "#\n",
    "qsdlkj@qsdfez\n",
    "qsoidhaeofih\n",
    "oiuh87657653@qsdfqf\n",
    "oiuh876-.!%*_+`'~@oiuh876-.!%*_+`'~\n",
    "#\n",
    0,
};

void osip_global_init(void);

void app_main(void)
{
  osip_call_id_t *callid;
  char *a_callid;
  char *dest;
  char *res;
  osip_global_init();
  a_callid = (char *) osip_malloc(200);
  int i = 0;
  while (callids[i] != 0) {
    // printf("new line\n");
    memset(a_callid, 0, 200);
    memcpy(a_callid, callids[i], strlen(callids[i]));
    int errcode;

    /* remove the last '\n' before parsing */
    strncpy(a_callid + strlen(a_callid) - 1, "\0", 1);

    if (0 != strncmp(a_callid, "#", 1)) {
      /* allocate & init callid */
      osip_call_id_init(&callid);
      printf("=================================================\n");
      printf("CALLID TO PARSE: |%s|\n", a_callid);
      errcode = osip_call_id_parse(callid, a_callid);

      if (errcode != -1) {
        if (osip_call_id_to_str(callid, &dest) != -1) {
          printf("%p\n", dest);
          printf("result:        |%s|\n", dest);
          osip_free(dest);
        }

      } else
        printf("Bad callid format: %s\n", a_callid);

      osip_call_id_free(callid);
      printf("=================================================\n");
    }
    i++;
  }
  osip_free(a_callid);
  return;
}

#include "osip2/osip.h"

osip_t *osip_global = NULL;
osip_transaction_t *transaction_global = NULL; 
osip_message_t *message_global = NULL;
osip_via_t *via_global = NULL;

void via_application_init(void)
{
    osip_via_init(&via_global);
    via_set_version(via_global, "V2.0.1");
    via_set_protocol(via_global, "UDP");
    via_set_host(via_global, "192.168.0.12");
    via_set_port(via_global, "8556");
    via_set_comment(via_global, "this is SIP example");
}

void message_application_init(void)
{
    uint32_t ret = 0;
    osip_uri_t *url;
    osip_uri_init(&url);
    osip_uri_set_scheme(url, "SIP");
    osip_uri_set_host(url, "192.186.0.33");
    osip_uri_set_username(url, "zhengsan");
    osip_uri_set_password(url, "77778888");
    osip_uri_set_port(url, "8321");
    url->string = "ccc";

    osip_message_init(&message_global);
    osip_message_set_version(message_global, "v3.2.1");
    osip_message_set_uri(message_global, url);

    osip_from_init(&(message_global->from));
    osip_from_set_displayname(message_global->from, "kooho");
    osip_from_set_url(message_global->from, url);

    osip_to_init(&(message_global->to));
    osip_to_set_displayname(message_global->to, "suzhou");
    osip_to_set_url(message_global->to, url);

    osip_call_id_init(&(message_global->call_id));
    osip_call_id_set_number(message_global->call_id, "kooho@192.168.0.4");
    via_application_init();
    osip_list_add(&(message_global->vias), via_global, 0);

    osip_cseq_init(&(message_global->cseq));
    osip_cseq_set_number(message_global->cseq, "1512345323");
    osip_cseq_set_method(message_global->cseq, "INVITE");
}

void message_send_cb(int cb_type, osip_transaction_t *trans, osip_message_t *message)
{
    printf("trans message\n");
}

void error_message_cb(int type, osip_transaction_t *trans, int error)
{
    printf("error message\n");
}
const char *c = "hello world";

void osip_global_init(void)
{
    int ret = -1;
    ret = osip_init(&osip_global);
    if (ret != OSIP_SUCCESS) {
        printf("osip init fail %d\n", __LINE__);
        return;
    }
    ret = osip_set_message_callback(osip_global, OSIP_ICT_INVITE_SENT, message_send_cb);
    if (ret != OSIP_SUCCESS) {
        printf("osip init fail %d\n", __LINE__);
        return;
    }
    ret = osip_set_transport_error_callback(osip_global, OSIP_ICT_INVITE_SENT, error_message_cb);
    if (ret != OSIP_SUCCESS) {
        printf("osip init fail %d\n", __LINE__);
        return;
    }
    message_application_init();
    ret = osip_transaction_init(&transaction_global, ICT, osip_global, message_global);
    if (ret != OSIP_SUCCESS) {
        printf("osip init fail %d\n", __LINE__);
        return;
    }

    osip_event_t event;
    memset(&event, 0, sizeof(event));
    ret = osip_transaction_add_event(transaction_global, &event);
    if (ret != OSIP_SUCCESS) {
        printf("osip init fail %d\n", __LINE__);
        return;
    }
}