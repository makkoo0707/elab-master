/*
 * @Author: jiejie
 * @Github: https://github.com/jiejieTop
 * @Date: 2020-02-25 03:36:09
 * @LastEditTime: 2020-06-17 19:59:41
 * @Description: the code belongs to jiejie, please keep the author information and source code according to the license.
 */

#ifndef _MQTT_CONFIG_H_
#define _MQTT_CONFIG_H_

#define MQTT_LOG_LEVEL                          MQTT_LOG_DEBUG_LEVEL   //MQTT_LOG_WARN_LEVEL MQTT_LOG_DEBUG_LEVEL
#define MQTT_MAX_PACKET_ID                      (0xFFFF - 1)
#define MQTT_TOPIC_LEN_MAX                      64
#define MQTT_ACK_HANDLER_NUM_MAX                64
#define MQTT_DEFAULT_BUF_SIZE                   2048
#define MQTT_DEFAULT_CMD_TIMEOUT                4000
#define MQTT_MAX_CMD_TIMEOUT                    20000
#define MQTT_MIN_CMD_TIMEOUT                    1000
#define MQTT_KEEP_ALIVE_INTERVAL                100         // unit: second
#define MQTT_VERSION                            4           // 4 is mqtt 3.1.1
#define MQTT_RECONNECT_DEFAULT_DURATION         1000
#define MQTT_THREAD_STACK_SIZE                  4096
#define MQTT_THREAD_PRIO                        5
#define MQTT_THREAD_TICK                        50

#define MQTT_NETWORK_TYPE_NO_TLS

#endif /* _DEFCONFIG_H_ */
