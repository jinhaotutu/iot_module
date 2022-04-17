/*
 * @Author: your name
 * @Date: 2022-02-03 15:49:56
 * @LastEditTime: 2022-04-16 12:07:32
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /module/softap/T_softap.c
 */

#include "T_softap.h"
#include "thirdlib/cJSON/cJSON.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


typedef struct ap_priv
{
    int fd;

    char devId[64];

    uint8_t data[512];
    int len;

    EVENT_CB event;
}ap_priv;

static int __check_crc(uint8_t *data, int len)
{
    uint8_t crc8=0;
    int i=0;
    for(i=0;i<len-1;i++)
    {
        crc8 += data[i];
    }

    if (crc8 != data[len-1])
    {
        printf("crc error:[%02x][%02x]\n", crc8, data[len-1]);
        return -1;
    }

    return 0;
}

static int __msgpack_parse(ap_object *obj, uint8_t *data, int len)
{
    ap_priv *priv = (ap_priv *)obj->p;

    printf("recv data:%s\n", data);

    int ret = 0;
    cJSON *json = cJSON_Parse(data);
    memset(priv->data, 0, sizeof(priv->data));
    if (NULL == json)
    {
        return -1;
    }

    cJSON *url = cJSON_GetObjectItem(json, "url");
    if (url == NULL)
    {
        return -1;
    }

    cJSON *res = NULL;
    cJSON *content = cJSON_GetObjectItem(json, "content");
    if (0 == strcasecmp(url->valuestring, "iot/wifi/config"))
    {
        net_info info={0};
        cJSON *subjs = cJSON_GetObjectItem(content, "ssid");
        if (subjs != NULL)
        {
            memcpy(info.ssid, subjs->valuestring, strlen(subjs->valuestring));
        }

        subjs = cJSON_GetObjectItem(content, "password");
        if (subjs != NULL)
        {
            memcpy(info.password, subjs->valuestring, strlen(subjs->valuestring));
        }

        if (NULL != priv->event)
        {
            priv->event(obj, GET_SSID_PASS_EVENT, &info);
        }
    }
    else if (0 == strcasecmp(url->valuestring, "iot/devid/get"))
    {
        res = cJSON_CreateObject();
        if (res != NULL)
        {
            cJSON_AddStringToObject(res, "devid", priv->devId);
        }
    }

    cJSON *result = cJSON_CreateObject();
    if (result != NULL)
    {
        cJSON_AddStringToObject(result, "url", url->valuestring);
        cJSON_AddNumberToObject(result, "result", ret);
        if (res)
        {
            cJSON_AddItemToObject(result, "content", res);
        }
        char *buf = cJSON_PrintUnformatted(result);
        cJSON_Delete(result);
        if (buf != NULL)
        {
            priv->len = strlen(buf);
            memcpy(priv->data, buf, priv->len);
            printf("send data[%d]:%s\n", priv->len, priv->data);
            free(buf);
        }
    }

    cJSON_Delete(json);

    return 0;
}

static int __data_parse(ap_object *obj, uint8_t *data, int len)
{
#if 0
    if (!(data[0] == 0xaa && data[1] == 0x55))
    {
        printf("head error:[%02x][%02x]\n", data[0], data[1]);
        return -1;
    }

    int datalen = data[2]<<8 | data[3];

    if (0 != __check_crc(data, datalen+5))
    {
        return -1;
    }

    printf("datalen:%d\n", datalen);
    int ret = __msgpack_parse(obj, cmd, data+4, datalen);
#else
    // 直接解析 json
    int ret = __msgpack_parse(obj, data, len);
#endif
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}


static int softap_config(struct ap_object *obj, int cmd, void *data)
{
    return 0;
}

static int softap_handle(struct ap_object *obj)
{
    ap_priv *priv = (ap_priv *)obj->p;

    priv->fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (priv->fd < 0)
    {
        return -1;
    }

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(50005);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(priv->fd, (struct sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0)
    {
        printf("bind error\n");
    }

    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr("224.0.1.205");
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);

    if(setsockopt(priv->fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq)) != 0) {
        printf("udpServer_serverSockInit: setsockopt IPPROTO_IP");
        return -1;
    }

    struct sockaddr_in from;
    socklen_t from_len = sizeof(from);
    while (1)
    {
        memset(priv->data, 0, 256);
        priv->len = recvfrom(priv->fd, priv->data, 256, 0, (struct sockaddr *)&from, &from_len);
        if (priv->len <= 0)
        {
            close(priv->fd);
            return priv->len;
        }

        __data_parse(obj, priv->data, priv->len);

        sendto(priv->fd, priv->data, priv->len, 0, (struct sockaddr *)&from, sizeof(from));
    }

    close(priv->fd);

    return 0;
}

static int softap_event_reg(struct ap_object *obj, EVENT_CB cb)
{
    ap_priv *priv = (ap_priv *)obj->p;

    priv->event = cb;

    return 0;
}

ap_object *softap_service_creat(char *devId)
{
    ap_object *obj = malloc(sizeof(ap_object));
    if (NULL == obj)
    {
        return NULL;
    }
    memset(obj, 0, sizeof(ap_object));

    obj->config = softap_config;
    obj->handle = softap_handle;
    obj->event_reg = softap_event_reg;

    ap_priv *priv = malloc(sizeof(ap_priv));
    if (NULL == priv)
    {
        free(obj);
        return NULL;
    }
    memset(priv, 0, sizeof(ap_priv));

    memcpy(priv->devId, devId, strlen(devId));
    obj->p = priv;

    printf("softap service creat success\n");

    return obj;
}

int softap_service_exit(struct ap_object *obj)
{
    if (obj)
    {
        ap_priv *priv = (ap_priv *)obj->p;
        if (priv)
        {
            free(priv);
        }

        free(obj);
    }

    return 0;
}
