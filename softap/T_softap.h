/*
 * @Author: your name
 * @Date: 2022-02-03 15:50:10
 * @LastEditTime: 2022-04-06 23:57:37
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: /module/softap/T_softap.h
 */

#ifndef T_SOFTAP_H
#define T_SOFTAP_H

#include "hal/T_type.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int (*EVENT_CB)(struct ap_object *obj, int event_id, void *data);

typedef struct ap_object
{
    int (*config)(struct ap_object *obj, int cmd, void *data);
    int (*handle)(struct ap_object *obj);
    int (*event_reg)(struct ap_object *obj, EVENT_CB cb);

    void *p;
}ap_object;

typedef int ap_eventid;

#define GET_SSID_PASS_EVENT     1
typedef struct net_info
{
    char ssid[65];
    char password[65];
}net_info;


ap_object *softap_service_creat(char *devId);
int softap_service_exit(struct ap_object *obj);


#ifdef __cplusplus
}
#endif

#endif // T_SOFTAP_H
