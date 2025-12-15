
/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(_WIN32) || defined(__linux__)

/* Includes ------------------------------------------------------------------*/
#include "mqtt_platform.h"
#include <sys/types.h>
#include <sys/param.h>
#include <sys/time.h>
#if defined(__linux__)
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include "../network/network.h"
#include "../common/mqtt_error.h"
#include "../../../common/elab_log.h"

#if defined(_WIN32)
#include <winsock.h>
#endif

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  MQTT connect port function.
  * @param  host        The Host IP address.
  * @param  port        The Host port.
  * @param  proto       The low level protocol of MQTT.
  * @retval Error ID.
  */
#if defined(_WIN32)
int32_t platform_net_socket_connect(const char *host,
                                    const char *port, int32_t proto)
{
    assert(proto == PLATFORM_NET_PROTO_TCP);

    int32_t ret = MQTT_SUCCESS_ERROR;

    WSADATA wsadata;
    if (WSAStartup(MAKEWORD(1, 1), &wsadata) == SOCKET_ERROR)
    {
        elog_error("WSAStartup() fail.");
        ret = -1;
        goto exit;
    }

    elog_debug("MQTT port connect. host: %s, port: %s.", host, port);

    int32_t fd;
    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        elog_error("create socket error: %s(errno: %d).",
                     strerror(errno), errno);
        ret = -2;
        goto exit;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port));
    server_addr.sin_addr.S_un.S_addr = inet_addr(host);

    if (connect(fd,
                (struct sockaddr*)&server_addr,
                sizeof(server_addr)) < 0)
    {
        elog_error("connect error: %s(errno: %d).", strerror(errno), errno);
        ret = -3;
        goto exit;
    }

    ret = fd;
    elog_debug("MQTT server TCP connect sucess. fd: %d.", fd);

exit:
    return ret;
}
#endif

/**
  * @brief  MQTT connect port function.
  * @param  host        The Host IP address.
  * @param  port        The Host port.
  * @param  proto       The low level protocol of MQTT.
  * @retval Error ID.
  */
#if defined(__linux__)
int32_t platform_net_socket_connect(const char *host,
                                    const char *port, int32_t proto)
{
    int32_t fd, ret = MQTT_SOCKET_UNKNOWN_HOST_ERROR;
    struct addrinfo hints, *addr_list, *cur;
    
    /* Do name resolution with both IPv6 and IPv4. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = (proto == PLATFORM_NET_PROTO_UDP) ? SOCK_DGRAM : SOCK_STREAM;
    hints.ai_protocol = (proto == PLATFORM_NET_PROTO_UDP) ? IPPROTO_UDP : IPPROTO_TCP;
    
    if (getaddrinfo(host, port, &hints, &addr_list) != 0)
    {
        return ret;
    }
    
    for (cur = addr_list; cur != NULL; cur = cur->ai_next)
    {
        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0)
        {
            ret = MQTT_SOCKET_FAILED_ERROR;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0)
        {
            ret = fd;
            break;
        }

        close(fd);
        ret = MQTT_CONNECT_FAILED_ERROR;
    }

    freeaddrinfo(addr_list);
    return ret;
}
#endif

/**
  * @brief  MQTT net recv port function.
  * @param  fd      The socket fd.
  * @param  port    Buffer for receiving data.
  * @param  len     Expected data length.
  * @retval Error ID.
  */
int32_t platform_net_socket_recv(int32_t fd, void *buf, size_t len, int32_t flags)
{
    return recv(fd, buf, len, flags);
}

/**
  * @brief  MQTT net recv port function with timeout parameter.
  * @param  fd      The socket fd.
  * @param  port    Buffer for receiving data.
  * @param  len     Expected data length.
  * @param  timeout Expected time to be timeout.
  * @retval Error ID.
  */
int32_t platform_net_socket_recv_timeout(int32_t fd, uint8_t *buf, 
                                            int32_t len, int32_t timeout)
{
    int32_t nread;
    int32_t nleft = len;
    uint8_t *ptr; 
    ptr = buf;

    struct timeval tv =
    {
        timeout / 1000, 
        (timeout % 1000) * 1000,
    };
    
    if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec <= 0))
    {
        tv.tv_sec = 0;
        tv.tv_usec = 100;
    }

    platform_net_socket_setsockopt(fd,
                                    SOL_SOCKET, SO_RCVTIMEO,
                                    (char *)&tv, sizeof(struct timeval));

    while (nleft > 0)
    {
        nread = platform_net_socket_recv(fd, ptr, nleft, 0);
        if (nread < 0)
        {
            return -1;
        }
        else if (nread == 0)
        {
            break;
        }

        nleft -= nread;
        ptr += nread;
    }

    return len - nleft;
}

/**
  * @brief  MQTT net write port function.
  * @param  fd      The socket fd.
  * @param  port    Buffer for receiving data.
  * @param  len     Expected data length.
  * @retval Error ID.
  */
int32_t platform_net_socket_write(int32_t fd, void *buf, size_t len)
{
#if defined(__linux__)
    return write(fd, buf, len);
#else
    return send(fd, buf, len, 0);
#endif
}

/**
  * @brief  MQTT net write port function with timeout parameter.
  * @param  fd      The socket fd.
  * @param  port    Buffer for receiving data.
  * @param  len     Expected data length.
  * @param  timeout Expected time to be timeout.
  * @retval Error ID.
  */
int32_t platform_net_socket_write_timeout(int32_t fd,
                                        uint8_t *buf, int32_t len,
                                        int32_t timeout)
{
    struct timeval tv =
    {
        timeout / 1000, 
        (timeout % 1000) * 1000
    };
    
    if (tv.tv_sec < 0 || (tv.tv_sec == 0 && tv.tv_usec <= 0))
    {
        tv.tv_sec = 0;
        tv.tv_usec = 100;
    }

    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(struct timeval));
    
#if defined(__linux__)
    return write(fd, buf, len);
#else
    return send(fd, buf, len, 0);
#endif
}

/**
  * @brief  MQTT net socket closing port function.
  * @param  fd      The socket fd.
  * @retval Error ID.
  */
int32_t platform_net_socket_close(int32_t fd)
{
    return close(fd);
}

/**
  * @brief  MQTT net socket blocking mode setting port function.
  * @param  fd      The socket fd.
  * @retval Error ID.
  */
int32_t platform_net_socket_set_block(int32_t fd)
{
#if defined(__linux__)
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, F_GETFL) & ~O_NONBLOCK);
#else
    unsigned long mode = 0;
    return ioctlsocket(fd, FIONBIO, &mode);
#endif
}

/**
  * @brief  MQTT net socket non-blocking mode setting port function.
  * @param  fd      The socket fd.
  * @retval Error ID.
  */
int32_t platform_net_socket_set_nonblock(int32_t fd)
{
#if defined(__linux__)
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, F_GETFL) | O_NONBLOCK);
#else
    unsigned long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode);
#endif
}

/**
  * @brief  MQTT net socket setsockopt port function.
  * @param  fd      The socket fd.
  * @retval Error ID.
  */
#if defined(__linux__)
int32_t platform_net_socket_setsockopt(int32_t fd, int32_t level, int32_t optname,
                                        const void *optval, socklen_t optlen)
#else
int32_t platform_net_socket_setsockopt(int32_t fd, int32_t level, int32_t optname,
                                        const void *optval, uint32_t optlen)
#endif
{
    return setsockopt(fd, level, optname, optval, optlen);
}

#endif

/* ----------------------------- end of file -------------------------------- */
