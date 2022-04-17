#include "softap/T_softap.h"

static int softap_event(struct ap_object *obj, ap_eventid id, void *data)
{
    switch (id)
    {
        case GET_SSID_PASS_EVENT:
            if (data == NULL)
            {
                return -1;
            }
            net_info *net = (net_info *)data;
            printf("get ssid:%s, password:%s\n", net->ssid, net->password);
            break;

        default:
            break;
    }

    return 0;
}

int main(void)
{
    printf("into main\n");

    ap_object *obj = softap_service_creat("TuuIotDev0001");
    if (NULL == obj)
    {
        return -1;
    }

    obj->event_reg(obj, softap_event);
    // obj->config

    int ret = obj->handle(obj);
    if (ret < 0)
    {
        printf("handle error:%d\n", ret);
    }

    softap_service_exit(obj);

    return 0;
}