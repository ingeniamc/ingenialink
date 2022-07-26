#ifdef _WIN32
    #define _WINSOCKAPI_ 
    #include <windows.h>
#else
    #include <unistd.h>
    #include <errno.h>
    #include <sys/time.h>
    #define Sleep(x) usleep((x)*1000)
#endif
#include "net.h"
#include "frame.h"

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <fcntl.h>

#include "ingenialink/err.h"
#include "ingenialink/base/net.h"
#include "external/log.c/src/log.h"

/*******************************************************************************
* Private
******************************************************************************/
static int il_net_reconnect(il_net_t *net);
static int process_monitoring_data(il_eth_net_t *this, il_net_t *net);
static int il_eth_net_recv_monitoring(il_eth_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
                                      size_t sz, uint8_t *monitoring_raw_data, il_net_t *net, int num_bytes);
static int il_eth_net_remove_all_mapped_registers_v1(il_net_t *net);
static int il_eth_net_remove_all_mapped_registers_v2(il_net_t *net);
static int il_eth_net_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype);
static int il_eth_net_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
                                             uint8_t subnode, il_reg_dtype_t dtype, uint8_t size);
static int il_eth_net_disturbance_remove_all_mapped_registers_v1(il_net_t *net);
static int il_eth_net_disturbance_remove_all_mapped_registers_v2(il_net_t *net);
static int il_eth_net_disturbance_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address,
                                                         uint8_t subnode, il_reg_dtype_t dtype, uint8_t size);
static int il_eth_net_disturbance_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
                                                         uint8_t subnode, il_reg_dtype_t dtype, uint8_t size);
static int net_recv(il_eth_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
                    size_t sz, uint16_t *monitoring_raw_data, il_net_t *net);
static int net_send(il_eth_net_t *this, uint8_t subnode, uint16_t address, const void *data,
                    size_t sz, uint16_t extended, il_net_t *net);
static int il_eth_net__read_monitoring(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
                                       void *buf, size_t sz);
static void il_eth_net_close_socket(il_net_t *net);																														

int il_net_monitoring_mapping_registers[16] = {
    0x0D0,
    0x0D1,
    0x0D2,
    0x0D3,
    0x0D4,
    0x0D5,
    0x0D6,
    0x0D7,
    0x0D8,
    0x0D9,
    0x0DA,
    0x0DB,
    0x0DC,
    0x0DD,
    0x0DE,
    0x0DF
};

int il_net_disturbance_mapping_registers[16] = {
    0x090,
    0x091,
    0x092,
    0x093,
    0x094,
    0x095,
    0x096,
    0x097,
    0x098,
    0x099,
    0x09A,
    0x09B,
    0x09C,
    0x09D,
    0x09E,
    0x09F
};

/**
* Destroy VIRTUAL network.
*
* @param [in] ctx
*	Context (il_net_t *).
*/
static void eth_net_destroy(void *ctx)
{
    il_eth_net_t *this = ctx;

    il_net_base__deinit(&this->net);

    free(this);
}

static int not_supported(void)
{
    return ilerr__eth(IL_ENOTSUP);
}

bool crc_tabccitt_init_eth = false;
uint16_t crc_tabccitt_eth[256];

/**
* Compute CRC of the given buffer.
*
* @param [in] buf
*	Buffer.
* @param [in] sz
*	Buffer size (bytes).
*
* @return
*	CRC.
*/
static void init_crcccitt_tab_eth(void) {

    uint16_t i;
    uint16_t j;
    uint16_t crc;
    uint16_t c;

    for (i = 0; i<256; i++) {
        crc = 0;
        c = i << 8;
        for (j = 0; j<8; j++) {
            if ((crc ^ c) & 0x8000) crc = (crc << 1) ^ 0x1021;
            else crc = crc << 1;
            c = c << 1;
        }
        crc_tabccitt_eth[i] = crc;
    }
    crc_tabccitt_init_eth = true;
}

static uint16_t update_crc_ccitt_eth(uint16_t crc, unsigned char c) {

    if (!crc_tabccitt_init_eth) init_crcccitt_tab_eth();
    return (crc << 8) ^ crc_tabccitt_eth[((crc >> 8) ^ (uint16_t)c) & 0x00FF];

}

static uint16_t crc_calc_eth(const uint16_t *buf, uint16_t u16Sz)
{

    uint16_t crc = 0x0000;
    uint8_t* pu8In = (uint8_t*)buf;

    for (uint16_t u16Idx = 0; u16Idx < u16Sz * 2; u16Idx++)
    {
        crc = update_crc_ccitt_eth(crc, pu8In[u16Idx]);
    }
    return crc;
}

/**
* Process asynchronous statusword messages.
*
* @param [in] this
*	ETH Network.
* @param [in] frame
*	IngeniaLink frame.
*/
static void process_statusword(il_eth_net_t *this, uint8_t subnode, uint16_t *data)
{
    il_net_sw_subscriber_lst_t *subs;
    int i;
    uint8_t id;
    uint16_t sw;

    subs = &this->net.sw_subs;

    id = subnode;
    sw = data;

    osal_mutex_lock(subs->lock);

    for (i = 0; i < subs->sz; i++) {
        if (subs->subs[i].id == id && subs->subs[i].cb) {
            void *ctx;

            ctx = subs->subs[i].ctx;
            subs->subs[i].cb(ctx, sw);

            break;
        }
    }

    osal_mutex_unlock(subs->lock);
}

/**
* Listener thread.
*
* @param [in] args
*	MCB Network (il_eth_net_t *).
*/
int listener_eth(void *args)
{
    int r;
    uint64_t buf;

restart:
{
    int error_count = 0;
    il_eth_net_t *this = to_eth_net(args);
    while (error_count < this->reconnection_retries && this != NULL && this->stop_reconnect == 0 ) {
        uint16_t sw;

        /* try to read the status word register to see if a servo is alive */
        if (this != NULL && this->status_check_stop == 0) {
            r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
            if (r < 0) {
                error_count = error_count + 1;
            }
            else {
                error_count = 0;
                this->stop = 0;
                process_statusword(this, 1, sw);
            }
        }
        Sleep(500);
    }
    if (error_count == this->reconnection_retries && this != NULL && this->stop_reconnect == 0) {
        goto err;
    }
    return 0;

err:
    if(this != NULL) {
        log_info("DEVICE DISCONNECTED!");
        ilerr__set("Device at %s disconnected\n", this->address_ip);
        il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);
        il_eth_net_close_socket(&this->net);
        r = il_net_reconnect(this);
        if (r == 0) goto restart;
    }
    return 0;
}
}

void SignalHandler(int signal)
{
    if (signal == SIGINT || signal == SIGTERM || signal == SIGABRT) {
        // abort signal handler code
        il_eth_net_t *this;
        this = calloc(1, sizeof(*this));
        if (!this) {
            ilerr__set("Network allocation failed");
            return NULL;
        }
        int r = il_net_close_socket(&this->net);
        log_error("Unexpected termination: %i", signal);

        exit(-1);
    }
    else {
        // ...
        log_warn("Unhandled signal exception: %i", signal);
    }
}

static il_net_t *il_eth_net_create(const il_net_opts_t *opts)
{
    il_eth_net_t *this;
    int r;

    typedef void(*SignalHandlerPointer)(int);

    /* Adding signal to catch exceptions */
    SignalHandlerPointer exc_handler_sigint, exc_handler_sigterm, exc_handler_sigabrt;
    exc_handler_sigint = signal(SIGINT, SignalHandler);
    exc_handler_sigterm = signal(SIGTERM, SignalHandler);
    exc_handler_sigabrt = signal(SIGABRT, SignalHandler);

    this = calloc(1, sizeof(*this));
    if (!this) {
        ilerr__set("Network allocation failed");
        return NULL;
    }

    /* initialize parent */
    r = il_net_base__init(&this->net, opts);
    if (r < 0)
        goto cleanup_this;
    this->net.ops = &il_eth_net_ops;
    this->net.prot = IL_NET_PROT_ETH;
    this->address_ip = opts->address_ip;
    this->port_ip = opts->port_ip;
    this->protocol = opts->protocol;
    this->reconnection_retries = RECONNECTION_RETRIES_DEF;
    this->stop_reconnect = 1;
    this->status_check_stop = 1;
    this->recv_timeout = READ_TIMEOUT_DEF;

    /* setup refcnt */
    this->refcnt = il_utils__refcnt_create(eth_net_destroy, this);
    if (!this->refcnt)
        goto cleanup_refcnt;
    if (opts->connect_slave != 0) {
        r = il_net_connect(&this->net);
        if (r < 0)
            goto cleanup_this;
    }

    this->listener = NULL;

    if (this->reconnection_retries > 0 && this->listener == NULL)
    {
        /* start listener thread */
        this->stop = 0;
        this->stop_reconnect = 0;

        this->listener = osal_thread_create_(listener_eth, this);
        if (!this->listener) {
            ilerr__set("Listener thread creation failed");
        }
    }

    return &this->net;

cleanup_refcnt:
    il_utils__refcnt_destroy(this->refcnt);

cleanup_this:
    free(this);

    return NULL;
}

static void il_eth_net_close_socket(il_net_t *net) {
    il_eth_net_t *this = to_eth_net(net);

    int r = 0;
    #ifdef _WIN32
        r = shutdown(this->server, SD_BOTH);
        if (r == 0) { r = closesocket(this->server); }
    #else
        r = shutdown(this->server, SHUT_RDWR);
        if (r == 0) { r = close(this->server); }
    #endif

    return r;	
}

static int il_eth_net_init_socket(void)
{
    #ifdef _WIN32
        /* Use Winsocks 2.2 (0x202) */
        WSADATA wsa_data;
        return WSAStartup(0x202, &wsa_data);
    #else
        return 0;
    #endif
}


static int il_eth_net_quit_socket(void)
{
    #ifdef _WIN32
        return WSACleanup();
    #else
        return 0;
    #endif
}

static int il_eth_get_last_socket_error(void)
{
    #ifdef _WIN32
        return WSAGetLastError();
    #else
        return errno;
    #endif
};

static void il_eth_net_destroy(il_net_t *net)
{
    il_eth_net_t *this = to_eth_net(net);
    il_utils__refcnt_release(this->refcnt);
}

static int il_eth_net_is_slave_connected(il_net_t *net, const char *ip) {

    il_eth_net_t *this = to_eth_net(net);
    int r = 0;
    int result = 0;
    uint16_t sw;

    if ((r = il_eth_net_init_socket()) != 0)
    {
        log_error("Server: WSAStartup() failed with error %d\n", r);
        r = il_eth_net_quit_socket();
        return -1;
    }
    else log_debug("Server: WSAStartup() is OK.\n");
    if (this != NULL) {
        if (this->protocol == 1)
        {
            this->server = socket(AF_INET, SOCK_STREAM, 0);
        }
        else
        {
            this->server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        }
        this->addr.sin_addr.s_addr = inet_addr(this->address_ip);
        this->addr.sin_family = AF_INET;
        this->addr.sin_port = htons(this->port_ip);

        unsigned long iMode = 1;
        #ifdef _WIN32
            r = ioctlsocket(this->server, FIONBIO, &iMode);		
            long no_err = NO_ERROR;
        #else
            r = ioctl(this->server, FIONBIO, &iMode);	
            int no_err = 0;
        #endif
        if (r != no_err)
        {
            log_warn("ioctlsocket failed with error: %ld", r);
        }

        r = connect(this->server, (struct sockaddr *)&this->addr, sizeof(this->addr));
        if (r < 0) {

            int err = il_eth_get_last_socket_error();

            // check if error was WSAEWOULDBLOCK, where we'll wait					
            if (
                #ifdef WINDOWS
                    err == WSAEWOULDBLOCK
                #else
                    err == EWOULDBLOCK
                #endif
                ){
                log_debug("Attempting to connect.\n");
                fd_set Write, Err;
                struct timeval Timeout;
                Timeout.tv_sec = 0;
                Timeout.tv_usec = 50000;

                FD_ZERO(&Write);
                FD_ZERO(&Err);
                FD_SET(this->server, &Write);
                FD_SET(this->server, &Err);

                #ifdef WINDOWS
                    r = select(this->server, NULL, &Write, &Err, &Timeout);
                #else
                    r = select(this->server + 1, NULL, &Write, &Err, &Timeout);
                #endif
                if (r == 0) {
                    log_warn("Timeout during connection");
                    result = 0;
                }
                    if (FD_ISSET(this->server, &Write)) {
                        log_info("Connected to the Server");
                        result = 1;
                    }
                    if (FD_ISSET(this->server, &Err)) {
                }
            }
            else {
                int last_error = il_eth_get_last_socket_error();
                log_warn("Fail connecting to server");
                result = 0;
            }

        }
        else
        {
            log_info("Connected to the Server");
            r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
            if (r < 0)
            {
                log_warn("Fail connecting to server");
                result = 0;
            }
            else
            {
                result = 1;
            }
        }


        iMode = 0;
        #ifdef _WIN32
            r = ioctlsocket(this->server, FIONBIO, &iMode);		
        #else
            r = ioctl(this->server, FIONBIO, &iMode);
        #endif		
        if (r != no_err)
        {
            log_warn("ioctlsocket failed with error: %ld", r);
        }
    }
    else result = 0;

    // Closing socket
    //closesocket(this->server);

    return result;

}

static int il_net_reconnect(il_net_t *net)
{
    il_eth_net_t *this = to_eth_net(net);
    this->stop = 1;
    int r = -1;
    uint16_t sw;
    while (r < 0 && this->stop_reconnect == 0)
    {
        log_debug("Reconnecting...");
        if (this->protocol == 1)
        {
            this->server = socket(AF_INET, SOCK_STREAM, 0);
        }
        else {
            this->server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        }

        //set the socket in non-blocking
        unsigned long iMode = 1;
        #ifdef _WIN32
            r = ioctlsocket(this->server, FIONBIO, &iMode);		
            long no_err = NO_ERROR;
        #else
            r = ioctl(this->server, FIONBIO, &iMode);	
            int no_err = 0;
        #endif
        if (r != no_err)
        {
            log_warn("ioctlsocket failed with error: %ld", r);
        }
        r = connect(this->server, (struct sockaddr *)&this->addr, sizeof(this->addr));
        if (r < 0) {
            int err = il_eth_get_last_socket_error();
            // check if error was WSAEWOULDBLOCK, where we'll wait
            if (
                #ifdef WINDOWS
                    err == WSAEWOULDBLOCK
                #else
                    err == EWOULDBLOCK
                #endif
                ){
                log_debug("Attempting to connect.\n");
                fd_set Write, Err;
                struct timeval Timeout;
                Timeout.tv_sec = 2;
                Timeout.tv_usec = 0;

                FD_ZERO(&Write);
                FD_ZERO(&Err);
                FD_SET(this->server, &Write);
                FD_SET(this->server, &Err);
                r = select(0, NULL, &Write, &Err, &Timeout);
                if (r == 0) {
                    log_warn("Timeout during connection");
                }
                else {
                    if (FD_ISSET(this->server, &Write)) {
                        log_info("Reconnected to the Server");
                        this->stop = 0;
                    }
                    if (FD_ISSET(this->server, &Err)) {
                        log_warn("Error reconnecting");
                    }
                }
            }
            else {
                int last_error = il_eth_get_last_socket_error();
                log_warn("Fail connecting to server");
            }
        }
        else {
            log_info("Connected to the Server");
            r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
            if (r < 0)
            {
                log_warn("Fail connecting to server");
            }
            else
            {
                this->stop = 0;
                this->stop_reconnect = 0;
                log_info("DEVICE RECONNECTED!");
                il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);
            }
        }
        iMode = 0;
        int r = -1;
        #ifdef _WIN32
            r = ioctlsocket(this->server, FIONBIO, &iMode);		
        #else
            r = ioctl(this->server, FIONBIO, &iMode);	
        #endif
        if (r != no_err)
        {
            log_warn("ioctlsocket failed with error: %ld", r);
        }
        Sleep(1000);
    }
    r = this->stop_reconnect;
    return r;
}

static int il_eth_net_connect(il_net_t *net, const char *ip)
{
    il_eth_net_t *this = to_eth_net(net);

    int r = 0;

    if ((r = il_eth_net_init_socket()) != 0)
    {
        log_error("Server: WSAStartup() failed with error %d\n", r);
        r = il_eth_net_quit_socket();
        return -1;
    }
    else log_debug("Server: WSAStartup() is OK.");
    int gas = this->protocol;
    // Initialize socket with the protocol choosen
    if (this->protocol == 1)
    {
        this->server = socket(AF_INET, SOCK_STREAM, 0);
    }
    else
    {
        this->server = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    this->addr.sin_addr.s_addr = inet_addr(this->address_ip);
    this->addr.sin_family = AF_INET;
    this->addr.sin_port = htons(this->port_ip);

    //set the socket in non-blocking
    unsigned long iMode = 1;
    #ifdef _WIN32
        r = ioctlsocket(this->server, FIONBIO, &iMode);		
        long no_err = NO_ERROR;
    #else
        r = ioctl(this->server, FIONBIO, &iMode);	
        int no_err = 0;
    #endif
    if (r != no_err)
    {
        log_warn("ioctlsocket failed with error: %ld", r);
    }

    r = connect(this->server, (struct sockaddr *)&this->addr, sizeof(this->addr));
    if (r < 0) {
        int err = il_eth_get_last_socket_error();
        // check if error was WSAEWOULDBLOCK, where we'll wait
        if (
            #ifdef WINDOWS
                err == WSAEWOULDBLOCK
            #else
                err == EWOULDBLOCK
            #endif
            ){
            log_debug("Attempting to connect.\n");
            fd_set Write, Err;
            struct timeval Timeout;
            Timeout.tv_sec = 2;
            Timeout.tv_usec = 0;

            FD_ZERO(&Write);
            FD_ZERO(&Err);
            FD_SET(this->server, &Write);
            FD_SET(this->server, &Err);
            r = select(0, NULL, &Write, &Err, &Timeout);
            if (r == 0) {
                log_warn("Timeout during connection");
                il_eth_net_close_socket(&this->net);
                return -1;
            }
            else {
                if (FD_ISSET(this->server, &Write)) {
                    log_info("Connected to the Server");
                }
                if (FD_ISSET(this->server, &Err)) {
                    log_error("Error connecting");
                    il_eth_net_close_socket(&this->net);
                    return -1;
                }
            }
        }
        else {
            int last_error = il_eth_get_last_socket_error();
            log_error("Fail connecting to server");
            il_eth_net_close_socket(&this->net);
            return -1;
        }
    }
    else {
        log_info("Connected to the Server");
    }
    iMode = 0;
    #ifdef _WIN32
        r = ioctlsocket(this->server, FIONBIO, &iMode);		
    #else
        r = ioctl(this->server, FIONBIO, &iMode);	
    #endif
    if (r != no_err)
    {
        log_warn("ioctlsocket failed with error: %ld", r);
    }

    /*
        Due to restriction of sockets connected to the slave, it's necessary to check that
        we can communicate with the slave.
    */
    uint32_t product_code_coco;
    r = il_net__read(&this->net, 1, 0, PRODUCT_CODE_COCO, &product_code_coco, sizeof(product_code_coco));
    if (r < 0) {
        uint32_t sw;
        r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
        if (r < 0) {
            log_error("Cannot connect to the slave.");
            il_eth_net_close_socket(&this->net);
            return r;
        }
    }
    else {
        uint32_t sw;
        r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
        if (r < 0) {
            log_error("Failed connecting. MOCO did not respond.");
            il_eth_net_close_socket(&this->net);
            return r;
        }
    }

    log_info("Connected to the Server!");
    il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);

    return 0;
}

static void il_eth_net__release(il_net_t *net)
{
    il_eth_net_t *this = to_eth_net(net);

    il_utils__refcnt_release(this->refcnt);
}

il_eth_net_dev_list_t *il_eth_net_dev_list_get()
{
    // TODO: Get slaves scanned
    il_eth_net_dev_list_t *lst = NULL;
    il_eth_net_dev_list_t *prev;


    prev = NULL;
    lst = malloc(sizeof(*lst));
    char *address_ip = "150.1.1.1";
    lst->address_ip = (char *)address_ip;

    return lst;


}

static int il_eth_status_get(il_net_t *net)
{
    il_eth_net_t *this = to_eth_net(net);
    return this->stop;
}

static int il_eth_mon_stop(il_net_t *net)
{
    il_eth_net_t *this = to_eth_net(net);
    this->stop_reconnect = 1;
    if (this->listener) {
        osal_thread_join(this->listener, NULL);
    }
}

static il_net_servos_list_t *il_eth_net_servos_list_get(
    il_net_t *net, il_net_servos_on_found_t on_found, void *ctx)
{
    int r;
    uint64_t vid;
    il_net_servos_list_t *lst;

    Sleep(2);
    /* try to read the vendor id register to see if a servo is alive */
    r = il_net__read(net, 1, 1, VENDOR_ID_ADDR, &vid, sizeof(vid));
    if (r < 0) {
        return NULL;
    }
    /* create list with one element (id=1) */
    lst = malloc(sizeof(*lst));
    if (!lst) {
        return NULL;
    }
    lst->next = NULL;
    lst->id = 1;

    if (on_found) {
        on_found(ctx, 1);
    }

    return lst;
}

// Monitoring ETH
/**
* Monitoring remove all mapped registers
*/
static int il_eth_net_remove_all_mapped_registers(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    // Check the Monitoring/Disturbance version
    uint32_t mon_dist_version = 0;
    r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
    if (r < 0) {
        // Old monitoring implementation
        r = il_eth_net_remove_all_mapped_registers_v1(net);
    }
    else {
        r = il_eth_net_remove_all_mapped_registers_v2(net);
    }

    return r;
}

static int il_eth_net_remove_all_mapped_registers_v1(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t remove_val = 1;
    r = il_net__write(&this->net, 1, 0, 0x00E2, &remove_val, 2, 1, 0);
    if (r < 0) {

    }

    net->monitoring_number_mapped_registers = 0;
    return r;
}

static int il_eth_net_remove_all_mapped_registers_v2(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t remove_val = 0;
    r = il_net__write(&this->net, 1, 0, 0x00E3, &remove_val, sizeof(uint16_t), 1, 0);
    if (r < 0) {

    }

    net->monitoring_number_mapped_registers = 0;
    return r;
}


#pragma Set_Mapped_Register
/**
* Monitoring set mapped registers
*/
static int *il_eth_net_set_mapped_register(il_net_t *net, int channel, uint32_t address,
                                            uint8_t subnode, il_reg_dtype_t dtype,
                                            uint8_t size)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    // Check the Monitoring/Disturbance version
    uint32_t mon_dist_version = 0;
    r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
    if (r < 0) {
        // Old monitoring implementation
        r = il_eth_net_set_mapped_register_v1(net, channel, address, dtype);
    }
    else {
        r = il_eth_net_set_mapped_register_v2(net, channel,
                                            address, subnode,
                                            dtype, size);
    }

    return r;
}

static int il_eth_net_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    net->monitoring_data_channels[channel].type = dtype;

    // Map address
    r = il_net__write(&this->net, 1, 0, 0x00E0, &address, 2, 1, 0);
    if (r < 0) {

    }
    // Update number of mapped registers & monitoring bytes per block
    net->monitoring_number_mapped_registers = net->monitoring_number_mapped_registers + 1;
    r = il_net__read(&this->net, 1, 0, 0x00E4, &net->monitoring_bytes_per_block, sizeof(net->monitoring_bytes_per_block));
    if (r < 0) {

    }

    return r;
}

static int il_eth_net_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
                                            uint8_t subnode, il_reg_dtype_t dtype,
                                            uint8_t size)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    net->monitoring_data_channels[channel].type = dtype;

    uint16_t frame[2];
    uint16_t hdr_h, hdr_l;
    uint16_t reg_address = address;
    uint8_t reg_subnode = subnode;
    uint8_t data_type = dtype;
    uint8_t data_size = size;

    hdr_l = ((uint32_t)data_type << 8) | (data_size);
    *(uint16_t *)&frame[0] = hdr_l;

    hdr_h = ((uint32_t) (subnode) << 12) | (address);
    *(uint16_t *)&frame[1] = hdr_h;

    uint32_t entire_frame = *(uint32_t*)frame;
    r = il_net__write(&this->net, 1, 0, il_net_monitoring_mapping_registers[net->monitoring_number_mapped_registers], &entire_frame, 4, 1, 0);

    // Update number of mapped registers & monitoring bytes per block
    net->monitoring_number_mapped_registers = net->monitoring_number_mapped_registers + 1;
    r = il_net__write(&this->net, 1, 0, 0x00E3, &net->monitoring_number_mapped_registers, 2, 1, 0);
    r = il_net__read(&this->net, 1, 0, 0x00E4, &net->monitoring_bytes_per_block, sizeof(net->monitoring_bytes_per_block));
    if (r < 0) {

    }

    return r;
}
#pragma endregion Set_Mapped_Register


/**
* Disturbance remove all mapped registers
*/
static int il_eth_net_disturbance_remove_all_mapped_registers(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    // Check the Monitoring/Disturbance version
    uint32_t mon_dist_version = 0;
    r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
    if (r < 0) {
        // Old monitoring implementation
        r = il_eth_net_disturbance_remove_all_mapped_registers_v1(net);
    }
    else {
        r = il_eth_net_disturbance_remove_all_mapped_registers_v2(net);
    }
    return r;
}

static int il_eth_net_disturbance_remove_all_mapped_registers_v1(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t remove_val = 1;
    r = il_net__write(&this->net, 1, 0, 0x00E7, &remove_val, 2, 1, 0);
    if (r < 0) {

    }

    net->disturbance_number_mapped_registers = 0;
    return r;
}

static int il_eth_net_disturbance_remove_all_mapped_registers_v2(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t remove_val = 0;
    r = il_net__write(&this->net, 1, 0, 0x00E8, &remove_val, 2, 1, 0);
    if (r < 0) {

    }

    net->disturbance_number_mapped_registers = 0;
    return r;
}

/**
* Disturbance set mapped reg
*/
static int il_eth_net_disturbance_set_mapped_register(il_net_t *net, int channel, uint32_t address,
                                                        uint8_t subnode, il_reg_dtype_t dtype,
                                                        uint8_t size)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    // Check the Monitoring/Disturbance version
    uint32_t mon_dist_version = 0;
    r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
    if (r < 0) {
        // Old monitoring implementation
        r = il_eth_net_disturbance_set_mapped_register_v1(net, channel, address, subnode, dtype, size);
    }
    else {
        r = il_eth_net_disturbance_set_mapped_register_v2(net, channel, address, subnode, dtype, size);
    }

    return r;
}

static int il_eth_net_disturbance_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address,
                                                            uint8_t subnode, il_reg_dtype_t dtype,
                                                            uint8_t size)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    // Always 0 for the moment
    net->disturbance_data_channels[channel].type = dtype;

    // Map address
    r = il_net__write(&this->net, 1, 0, 0x00E5, &address, 2, 1, 0);
    if (r < 0) {

    }

    return r;
}

static int il_eth_net_disturbance_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
                                                        uint8_t subnode, il_reg_dtype_t dtype,
                                                        uint8_t size)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    net->disturbance_data_channels[channel].type = dtype;

    uint16_t frame[2];
    uint16_t hdr_h, hdr_l;
    uint16_t reg_address = address;
    uint8_t reg_subnode = subnode;
    uint8_t data_type = dtype;
    uint8_t data_size = size;

    hdr_l = ((uint32_t)data_type << 8) | (data_size);
    *(uint16_t *)&frame[0] = hdr_l;

    hdr_h = ((uint32_t) (subnode) << 12) | (address);		// subnode | address
    *(uint16_t *)&frame[1] = hdr_h;

    uint32_t entire_frame = *(uint32_t*)frame;
    r = il_net__write(&this->net, 1, 0, il_net_disturbance_mapping_registers[net->disturbance_number_mapped_registers], &entire_frame, 4, 1, 0);

    // Update number of mapped registers
    net->disturbance_number_mapped_registers = net->disturbance_number_mapped_registers + 1;
    r = il_net__write(&this->net, 1, 0, 0x0E8, &net->disturbance_number_mapped_registers, 2, 1, 0);

    return r;
}

/**
* Disturbance enable
*/
static int il_eth_net_enable_disturbance(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t enable_disturbance_val = 1;

    // Check the Monitoring/Disturbance version
    uint32_t mon_dist_version = 0;
    r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
    if (r < 0) {
        // Old version
        r = il_net__write(&this->net, 1, 0, 0x00C0, &enable_disturbance_val, 2, 1, 0);
        if (r < 0) {

        }
    } else {
        // New version
        // Obtaining the bit to know if monitoring and disturbance are detached or not
        int detached_monitoring_bit = 0;
        int mask =  1 << detached_monitoring_bit;
        int masked_n = mon_dist_version & mask;
        int detached_monitoring = masked_n >> detached_monitoring_bit;
        if (detached_monitoring == 0) {
            // Monitoring and distrubance are NOT detached.
            r = il_net__write(&this->net, 1, 0, 0x00C0, &enable_disturbance_val, 2, 1, 0);
            if (r < 0) {

            }
        }
        else {
            // Monitoring and disturbance are detached and we need to write in the new register.
            r = il_net__write(&this->net, 1, 0, 0x00C7, &enable_disturbance_val, 2, 1, 0);
            if (r < 0) {

            }
        }
    }

    return r;
}

static int il_eth_net_disable_disturbance(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t disable_disturbance_val = 0;

    // Check the Monitoring/Disturbance version
    uint32_t mon_dist_version = 0;
    r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
    if (r < 0) {
        // Old version
        r = il_net__write(&this->net, 1, 0, 0x00C0, &disable_disturbance_val, 2, 1, 0);
        if (r < 0) {

        }
    } else {
        // New version
        // Obtaining the bit to know if monitoring and disturbance are detached or not
        int detached_monitoring_bit = 0;
        int mask =  1 << detached_monitoring_bit;
        int masked_n = mon_dist_version & mask;
        int detached_monitoring = masked_n >> detached_monitoring_bit;
        if (detached_monitoring == 0) {
            // Monitoring and distrubance are NOT detached.
            r = il_net__write(&this->net, 1, 0, 0x00C0, &disable_disturbance_val, 2, 1, 0);
            if (r < 0) {

            }
        }
        else {
            // Monitoring and disturbance are detached and we need to write in the new register.
            r = il_net__write(&this->net, 1, 0, 0x00C7, &disable_disturbance_val, 2, 1, 0);
            if (r < 0) {

            }
        }
    }

    return r;
}

static int il_eth_net_monitoring_remove_data(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);
    uint16_t remove_data_val = 1;

    r = il_net__write(&this->net, 1, 0, 0x0EA, &remove_data_val, 2, 1, 0);
    if (r < 0) {

    }
    return r;
}

static int il_eth_net_disturbance_remove_data(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);
    uint16_t remove_data_val = 1;

    r = il_net__write(&this->net, 1, 0, 0x0EB, &remove_data_val, 2, 1, 0);
    if (r < 0) {

    }
    return r;
}

static int il_eth_set_last_channel(il_net_t *net, int channel)
{
    net->last_channel = net->last_channel > channel ? net->last_channel : channel;
    return 0;
}

/**
* Monitoring enable
*/
static int il_eth_net_enable_monitoring(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t enable_monitoring_val = 1;
    r = il_net__write(&this->net, 1, 0, 0x00C0, &enable_monitoring_val, 2, 1, 0);
    if (r < 0) {

    }
    return r;
}

static int il_eth_net_disable_monitoring(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint16_t disable_monitoring_val = 0;
    r = il_net__write(&this->net, 1, 0, 0x00C0, &disable_monitoring_val, 2, 1, 0);
    if (r < 0) {

    }

    return r;
}

static int il_eth_net_read_monitoring_data(il_net_t *net)
{
    int r = 0;
    il_eth_net_t *this = to_eth_net(net);

    uint64_t vid;


    osal_mutex_lock(this->net.lock);
    r = il_eth_net__read_monitoring(&this->net, 1, 0, 0x00B2, &vid, sizeof(vid));
    if (r < 0) {

    }
    osal_mutex_unlock(this->net.lock);
    return r;
}

static il_net_dev_mon_t *il_eth_net_dev_mon_create(void)
{
    il_eth_net_dev_mon_t *this;

    this = malloc(sizeof(*this));
    if (!this) {
        ilerr__set("Monitor allocation failed");
        return NULL;
    }

    this->mon.ops = &il_eth_net_dev_mon_ops;
    this->running = 0;

    return &this->mon;
}

static void il_eth_net_dev_mon_destroy(il_net_dev_mon_t *mon)
{
    il_eth_net_dev_mon_t *this = to_eth_mon(mon);

    il_net_dev_mon_stop(mon);

    free(this);
}

static int il_eth_net_dev_mon_start(il_net_dev_mon_t *mon,
    il_net_dev_on_evt_t on_evt,
    void *ctx)
{
	il_eth_net_dev_mon_t *this = to_eth_mon(mon);

	if (this->running) {
		ilerr__set("Monitor already running");
		return IL_EALREADY;
	}

	/* store context and bring up monitor */
	this->ctx = ctx;
	this->on_evt = on_evt;
	this->running = 1;

	return 0;
}

static void il_eth_net_dev_mon_stop(il_net_dev_mon_t *mon)
{
    il_eth_net_dev_mon_t *this = to_eth_mon(mon);

	if (this->running) {
		this->running = 0;
	}
}

static int il_eth_net__read(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
    void *buf, size_t sz)
{
    il_eth_net_t *this = to_eth_net(net);
    int r;
    (void)id;

    osal_mutex_lock(this->net.lock);
    r = net_send(this, subnode, (uint16_t)address, NULL, 0, 0, net);
    if (r < 0) {
        goto unlock;
    }
    uint16_t *monitoring_raw_data = NULL;
    r = net_recv(this, subnode, (uint16_t)address, buf, sz, monitoring_raw_data, net);
    if (r < 0)
        goto unlock;

unlock:
    osal_mutex_unlock(this->net.lock);

    return r;
}

static int il_eth_net__read_monitoring(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
    void *buf, size_t sz)
{
    il_eth_net_t *this = to_eth_net(net);
    int r;
    (void)id;

    int num_bytes;
    r = il_net__read(&this->net, 1, 0, 0x00B7, &num_bytes, sizeof(num_bytes));
    if (r < 0)
    {
        // Old monitoring method
        uint64_t vid;
        r = il_net__read(&this->net, 1, 0, 0x00B2, &vid, sizeof(vid));
    }
    else
    {
        // Initialize monitoring data size value
        net->monitoring_data_size = 0;

        while (num_bytes > 0)
        {
            // osal_mutex_lock(this->net.lock);
            r = net_send(this, subnode, (uint16_t)address, NULL, 0, 0, net);
            if (r < 0) {
                goto unlock;
            }
            uint8_t *monitoring_raw_data = NULL;
            r = il_eth_net_recv_monitoring(this, subnode, (uint16_t)address, buf, sz, monitoring_raw_data, net, num_bytes);
            if (r < 0)
                goto unlock;
            // osal_mutex_unlock(this->net.lock);

            r = il_net__read(&this->net, 1, 0, 0x00B7, &num_bytes, sizeof(num_bytes));
            if (r < 0) {
                goto unlock;
            }

        }

        if (r >= 0)
        {
            r = process_monitoring_data(this, net);
        }
    }


unlock:
    osal_mutex_unlock(this->net.lock);

    return r;
}

static int il_eth_net__write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
    const void *buf, size_t sz, int confirmed, uint16_t extended)
{
    il_eth_net_t *this = to_eth_net(net);

    int r;

    (void)id;
    (void)confirmed;

    osal_mutex_lock(this->net.lock);

    r = net_send(this, subnode, (uint16_t)address, buf, sz, extended, net);
    if (r < 0)
        goto unlock;

    r = net_recv(this, subnode, (uint16_t)address, NULL, 0, NULL, NULL);
    if (r < 0)
        goto unlock;

unlock:
    osal_mutex_unlock(this->net.lock);

    return r;
}

static int il_eth_net__wait_write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
    const void *buf, size_t sz, int confirmed, uint16_t extended)
{
    il_eth_net_t *this = to_eth_net(net);

    int r;

    (void)id;
    (void)confirmed;

    osal_mutex_lock(this->net.lock);


    r = net_send(this, subnode, (uint16_t)address, buf, sz, extended, net);
    if (r < 0)
        goto unlock;

    Sleep(1000);

    r = net_recv(this, subnode, (uint16_t)address, NULL, 0, NULL, NULL);
    if (r < 0)
        goto unlock;

unlock:
    osal_mutex_unlock(this->net.lock);

    return r;
}

typedef union
{
    uint64_t u64;
    uint16_t u16[4];
} UINT_UNION_T;

static int net_send(il_eth_net_t *this, uint8_t subnode, uint16_t address, const void *data,
    size_t sz, uint16_t extended, il_net_t *net)
{
    int finished = 0;
    uint8_t cmd;

    cmd = sz ? ETH_MCB_CMD_WRITE : ETH_MCB_CMD_READ;


    while (!finished) {
        int r;
        uint16_t frame[ETH_MCB_FRAME_SZ];
        uint16_t hdr_h, hdr_l, crc;
        size_t chunk_sz;

        /* header */
        hdr_h = (ETH_MCB_NODE_DFLT << 4) | (subnode);
        *(uint16_t *)&frame[ETH_MCB_HDR_H_POS] = hdr_h;
        hdr_l = (address << 4) | (cmd << 1) | (extended);
        *(uint16_t *)&frame[ETH_MCB_HDR_L_POS] = hdr_l;

        /* cfg_data */
        uint64_t d = 0;
        /* Check if frame is extended */
        if (extended == 1) {
            d = net->disturbance_data_size;
        }
        else {
            if (sz > 0) {
                memcpy(&d, data, sz);
            }
        }
        UINT_UNION_T u = { .u64 = d };
        memcpy(&frame[ETH_MCB_DATA_POS], &u.u16[0], 8);

        /* crc */
        crc = crc_calc_eth(frame, ETH_MCB_CRC_POS);
        frame[ETH_MCB_CRC_POS] = crc;

        /* send frame */
        if (extended == 1) {
            uint16_t frame_size = sizeof(uint16_t) * ETH_MCB_FRAME_SZ;
            uint8_t extended_frame[1024];

            int n_bytes_sum = 0;
            int sample_position = 0;
            uint8_t* pData;
            pData = malloc(net->disturbance_data_size);
            uint8_t* pDataAux = pData;

            while(n_bytes_sum < net->disturbance_data_size){
                for (int i = 0; i <= net->last_channel; i++){
                    il_reg_dtype_t type = net->disturbance_data_channels[i].type;
                    switch (type) {
                        case IL_REG_DTYPE_U16:
                            n_bytes_sum+=2;
                            memcpy(pData, &net->disturbance_data_channels[i].value.disturbance_data_u16[sample_position], 2);
                            pData += sizeof(uint16_t);
                            break;
                        case IL_REG_DTYPE_S16:
                            n_bytes_sum+=2;
                            memcpy(pData, &net->disturbance_data_channels[i].value.disturbance_data_s16[sample_position], 2);
                            pData += sizeof(int16_t);
                            break;
                        case IL_REG_DTYPE_U32:
                            n_bytes_sum+=4;
                            memcpy(pData, &net->disturbance_data_channels[i].value.disturbance_data_u32[sample_position], 4);
                            pData += sizeof(uint32_t);
                            break;
                        case IL_REG_DTYPE_S32:
                            n_bytes_sum+=4;
                            memcpy(pData, &net->disturbance_data_channels[i].value.disturbance_data_s32[sample_position], 4);
                            pData += sizeof(int32_t);
                            break;
                        case IL_REG_DTYPE_FLOAT:
                            n_bytes_sum+=4;
                            memcpy(pData, &net->disturbance_data_channels[i].value.disturbance_data_flt[sample_position], 4);
                            pData += sizeof(float);
                            break;
                    }
                }
                sample_position++;
            }

            memcpy(&extended_frame[0], frame, frame_size);
            memcpy(&extended_frame[frame_size], pDataAux, 1024 - frame_size);
            free(pDataAux);
            r = send(this->server, (const char*)&extended_frame[0], net->disturbance_data_size + frame_size, 0);
            if (r < 0)
                return ilerr__eth(r);
        }
        else {
            r = send(this->server, (const char*)&frame[0], sizeof(frame), 0);
            if (r < 0)
                return ilerr__eth(r);
        }
        finished = 1;
    }

    return 0;
}

static int net_recv(il_eth_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
    size_t sz, uint16_t *monitoring_raw_data, il_net_t *net)
{
    int finished = 0;
    size_t pending_sz = sz;

    /*while (!finished) {*/
    uint16_t frame[1024];
    size_t block_sz = 0;
    uint16_t crc, hdr_l;
    uint8_t *pBuf = (uint8_t*)&frame;
    uint8_t extended_bit = 0;

    Sleep(5);
    int r = 0;
    // GAS
    fd_set fds;
    int n;
    struct timeval tv;

    // Set up the file descriptor set.
    FD_ZERO(&fds);
    FD_SET(this->server, &fds);

    // Set up the struct timeval for the timeout.
    tv.tv_sec = 0;
    tv.tv_usec = this->recv_timeout;

    // Wait until timeout or data received.
    #ifdef WINDOWS
        n = select(this->server, &fds, NULL, NULL, &tv);
    #else
        n = select(this->server + 1, &fds, NULL, NULL, &tv);
    #endif	
    if (n == 0)
    {
        log_error("Timeout...");
        return ilerr__eth(IL_ETIMEDOUT);
    }
    else if (n == -1)
    {
        log_error("Error..");
        return ilerr__eth(IL_EIO);
    }

    /* read next frame */
    r = recv(this->server, (char*)&pBuf[0], sizeof(frame), 0);

    /* process frame: validate CRC, address, ACK */
    crc = *(uint16_t *)&frame[6];
    uint16_t crc_res = crc_calc_eth((uint16_t *)frame, 6);
    if (crc_res != crc) {
        return ilerr__eth(IL_EWRONGCRC);
    }

    /* TODO: Check subnode */

    /* Check ACK */
    hdr_l = *(uint16_t *)&frame[ETH_MCB_HDR_L_POS];
    int cmd = (hdr_l & ETH_MCB_CMD_MSK) >> ETH_MCB_CMD_POS;
    if (cmd != ETH_MCB_CMD_ACK) {

        uint32_t err;

        err = __swap_be_32(*(uint32_t *)&frame[ETH_MCB_DATA_POS]);

        ilerr__set("Communications error (NACK -> %08x)", err);
        ilerr__ipb_set(err);
        return IL_ENACK;
    }
    /* Check address */

    uint16_t addr = (hdr_l & 0xFFF0) >> 4;
    if (addr != address) {
        uint32_t err;

        err = __swap_be_32(*(uint32_t *)&frame[ETH_MCB_DATA_POS]);
        log_error("Address error (Address asked -> %08x, Address frame -> %08x, err -> %08x)"
                , address, addr, err);
        log_error("Frame -> %04x %04x %04x %04x %04x %04x %04x %04x", frame[0], frame[1], frame[2], frame[3], frame[4], frame[5], frame[6], frame[7]);
        return ilerr__eth(IL_EWRONGREG);
    }

    extended_bit = (hdr_l & ETH_MCB_PENDING_MSK) >> ETH_MCB_PENDING_POS;
    if (extended_bit == 1) {
        /* Check if we are reading monitoring data */
        if (address == 0x00B2) {
            /* Monitoring */
            /* Read size of data */
            memcpy(buf, &(frame[ETH_MCB_DATA_POS]), 2);
            uint16_t size = *(uint16_t*)buf;
            memcpy(net->monitoring_raw_data, (uint8_t*)&pBuf[14], size);
            //r = recv(this->server, (uint8_t*)net->monitoring_raw_data, size, 0);

            net->monitoring_data_size = size;
            int num_mapped = net->monitoring_number_mapped_registers;
            int bytes_per_block = net->monitoring_bytes_per_block;

            int number_blocks = size / bytes_per_block;
            uint8_t* pData = net->monitoring_raw_data;

            for (int i = 0; i < number_blocks; ++i)
            {
                int OffsetIndexIntraBlockInBytes = 0;
                for (int j = 0; j < num_mapped; ++j)
                {
                    il_reg_dtype_t type = net->monitoring_data_channels[j].type;
                    switch (type) {
                        case IL_REG_DTYPE_U16:
                            net->monitoring_data_channels[j].value.monitoring_data_u16[i] = *((uint16_t*)(pData + OffsetIndexIntraBlockInBytes));
                            OffsetIndexIntraBlockInBytes += sizeof(uint16_t);
                            break;
                        case IL_REG_DTYPE_S16:
                            net->monitoring_data_channels[j].value.monitoring_data_s16[i] = *((int16_t*)(pData + OffsetIndexIntraBlockInBytes));
                            OffsetIndexIntraBlockInBytes += sizeof(int16_t);
                            break;
                        case IL_REG_DTYPE_U32:
                            net->monitoring_data_channels[j].value.monitoring_data_u32[i] = *((uint32_t*)(pData + OffsetIndexIntraBlockInBytes));
                            OffsetIndexIntraBlockInBytes += sizeof(uint32_t);
                            break;
                        case IL_REG_DTYPE_S32:
                            net->monitoring_data_channels[j].value.monitoring_data_s32[i] = *((int32_t*)(pData + OffsetIndexIntraBlockInBytes));
                            OffsetIndexIntraBlockInBytes += sizeof(int32_t);
                            break;
                        case IL_REG_DTYPE_FLOAT:
                            net->monitoring_data_channels[j].value.monitoring_data_flt[i] = *((float*)(pData + OffsetIndexIntraBlockInBytes));
                            OffsetIndexIntraBlockInBytes += sizeof(float);
                            break;
                    }
                }
                pData += bytes_per_block;
            }
        }
        else {
            memcpy(buf, &(frame[ETH_MCB_DATA_POS]), 2);
            uint16_t size = *(uint16_t*)buf;
            memcpy(net->extended_buff, (char*)&pBuf[14], size);

        }
    }
    else {
        memcpy(buf, &(frame[ETH_MCB_DATA_POS]), sz);
    }

    return 0;
}

static int il_eth_net_recv_monitoring(il_eth_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
    size_t sz, uint8_t *monitoring_raw_data, il_net_t *net, int num_bytes)
{
    int finished = 0;
    size_t pending_sz = sz;

    /*while (!finished) {*/
    uint16_t frame[1024];
    size_t block_sz = 0;
    uint16_t crc, hdr_l;
    uint8_t *pBuf = (uint8_t*)&frame;
    uint8_t extended_bit = 0;

    Sleep(5);
    int r = 0;
    // GAS
    fd_set fds;
    int n;
    struct timeval tv;

    // Set up the file descriptor set.
    FD_ZERO(&fds);
    FD_SET(this->server, &fds);

    // Set up the struct timeval for the timeout.
    tv.tv_sec = 0;
    tv.tv_usec = this->recv_timeout;

    // Wait until timeout or data received.
    #ifdef WINDOWS
        n = select(this->server, &fds, NULL, NULL, &tv);
    #else
        n = select(this->server + 1, &fds, NULL, NULL, &tv);
    #endif	
    if (n == 0)
    {
        log_error("Timeout..");
        il_eth_net_close_socket(&this->net);
        return -1;
    }
    else if (n == -1)
    {
        log_error("Error..");
        return -1;
    }

    /* read next frame */
    r = recv(this->server, (char*)&pBuf[0], sizeof(frame), 0);

    /* process frame: validate CRC, address, ACK */
    crc = *(uint16_t *)&frame[6];
    uint16_t crc_res = crc_calc_eth((uint16_t *)frame, 6);
    if (crc_res != crc) {
        return ilerr__eth(IL_EWRONGCRC);
    }

    /* TODO: Check subnode */

    /* Check ACK */
    hdr_l = *(uint16_t *)&frame[ETH_MCB_HDR_L_POS];
    int cmd = (hdr_l & ETH_MCB_CMD_MSK) >> ETH_MCB_CMD_POS;
    if (cmd != ETH_MCB_CMD_ACK) {
        uint32_t err;

        err = __swap_be_32(*(uint32_t *)&frame[ETH_MCB_DATA_POS]);

        ilerr__set("Communications error (NACK -> %08x)", err);
        ilerr__ipb_set(err);
        return IL_ENACK;
    }
    extended_bit = (hdr_l & ETH_MCB_PENDING_MSK) >> ETH_MCB_PENDING_POS;
    if (extended_bit == 1) {
        /* Check if we are reading monitoring data */
        if (address == 0x00B2) {
            /* Monitoring */
            /* Read size of data */
            memcpy(buf, &(frame[ETH_MCB_DATA_POS]), 2);
            uint16_t size = *(uint16_t*)buf;
            if (num_bytes < size)
            {
                size = num_bytes;
            }
            uint32_t start_addr = net->monitoring_data_size;
            memcpy((uint8_t*)&net->monitoring_raw_data[start_addr], (uint8_t*)&pBuf[14], size);

            net->monitoring_data_size += size;
        }
        else {
            memcpy(buf, &(frame[ETH_MCB_DATA_POS]), 2);
            uint16_t size = *(uint16_t*)buf;
            memcpy(net->extended_buff, (char*)&pBuf[14], size);

        }
    }
    else {
        memcpy(buf, &(frame[ETH_MCB_DATA_POS]), sz);
    }

    return 0;
}

static int process_monitoring_data(il_eth_net_t *this, il_net_t *net)
{
    log_debug("Process monitoring data: %i", net->monitoring_data_size);

    int num_mapped = net->monitoring_number_mapped_registers;
    int bytes_per_block = net->monitoring_bytes_per_block;

    int number_blocks = net->monitoring_data_size / bytes_per_block;
    uint8_t* pData = net->monitoring_raw_data;

    for (int i = 0; i < number_blocks; ++i)
    {
        int OffsetIndexIntraBlockInBytes = 0;
        for (int j = 0; j < num_mapped; ++j)
        {
            il_reg_dtype_t type = net->monitoring_data_channels[j].type;
            switch (type) {
            case IL_REG_DTYPE_U16:
                net->monitoring_data_channels[j].value.monitoring_data_u16[i] = *((uint16_t*)(pData + OffsetIndexIntraBlockInBytes));
                OffsetIndexIntraBlockInBytes += sizeof(uint16_t);
                break;
            case IL_REG_DTYPE_S16:
                net->monitoring_data_channels[j].value.monitoring_data_s16[i] = *((int16_t*)(pData + OffsetIndexIntraBlockInBytes));
                OffsetIndexIntraBlockInBytes += sizeof(int16_t);
                break;
            case IL_REG_DTYPE_U32:
                net->monitoring_data_channels[j].value.monitoring_data_u32[i] = *((uint32_t*)(pData + OffsetIndexIntraBlockInBytes));
                OffsetIndexIntraBlockInBytes += sizeof(uint32_t);
                break;
            case IL_REG_DTYPE_S32:
                net->monitoring_data_channels[j].value.monitoring_data_s32[i] = *((int32_t*)(pData + OffsetIndexIntraBlockInBytes));
                OffsetIndexIntraBlockInBytes += sizeof(int32_t);
                break;
            case IL_REG_DTYPE_FLOAT:
                net->monitoring_data_channels[j].value.monitoring_data_flt[i] = *((float*)(pData + OffsetIndexIntraBlockInBytes));
                OffsetIndexIntraBlockInBytes += sizeof(float);
                break;
            }
        }
        pData += bytes_per_block;
    }
    return 0;
}

int il_eth_set_reconnection_retries(il_net_t *net, uint8_t retries)
{
    il_eth_net_t *this = to_eth_net(net);
    this->reconnection_retries = retries;

    if (this->reconnection_retries > 0 && this->listener == NULL)
    {
        /* start listener thread */
        this->stop = 0;
        this->stop_reconnect = 0;

        this->listener = osal_thread_create_(listener_eth, this);
        if (!this->listener) {
            ilerr__set("Listener thread creation failed");
        }
    }
    return 0;
}

int il_eth_set_recv_timeout(il_net_t *net, uint32_t timeout)
{
    il_eth_net_t *this = to_eth_net(net);
    this->recv_timeout = timeout;
    return 0;
}

int il_eth_set_status_check_stop(il_net_t *net, int stop)
{
    il_eth_net_t *this = to_eth_net(net);
    this->status_check_stop = stop;
    return 0;
}

/** ETH network operations. */
const il_eth_net_ops_t il_eth_net_ops = {
    /* internal */
    ._read = il_eth_net__read,
    ._write = il_eth_net__write,
    ._release = il_eth_net__release,
    ._wait_write = il_eth_net__wait_write,
    ._sw_subscribe = il_net_base__sw_subscribe,
    ._sw_unsubscribe = il_net_base__sw_unsubscribe,
    ._emcy_subscribe = il_net_base__emcy_subscribe,
    ._emcy_unsubscribe = il_net_base__emcy_unsubscribe,
    /* public */
    .create = il_eth_net_create,
    .destroy = il_eth_net_destroy,
    .close_socket = il_eth_net_close_socket,
    .connect = il_eth_net_connect,
    .is_slave_connected = il_eth_net_is_slave_connected,
    // .devs_list_get = il_eth_net_dev_list_get,
    .servos_list_get = il_eth_net_servos_list_get,
    .status_get = il_eth_status_get,
    ._state_set = il_net_base__state_set,
    .mon_stop = il_eth_mon_stop,
    /* Monitornig */
    .remove_all_mapped_registers = il_eth_net_remove_all_mapped_registers,
    .set_mapped_register = il_eth_net_set_mapped_register,
    .enable_monitoring = il_eth_net_enable_monitoring,
    .disable_monitoring = il_eth_net_disable_monitoring,

    /* Disturbance */
    .disturbance_remove_all_mapped_registers = il_eth_net_disturbance_remove_all_mapped_registers,
    .disturbance_set_mapped_register = il_eth_net_disturbance_set_mapped_register,
    // .set_last_channel = il_eth_set_last_channel,
    .enable_disturbance = il_eth_net_enable_disturbance,
    .disable_disturbance = il_eth_net_disable_disturbance,
    .monitoring_remove_data = il_eth_net_monitoring_remove_data,
    .disturbance_remove_data = il_eth_net_disturbance_remove_data,
    .read_monitoring_data = il_eth_net_read_monitoring_data,
    .recv_monitoring = il_eth_net_recv_monitoring,
    .set_reconnection_retries = il_eth_set_reconnection_retries,
    .set_recv_timeout = il_eth_set_recv_timeout,
    .set_status_check_stop = il_eth_set_status_check_stop
};

/** MCB network device monitor operations. */
const il_net_dev_mon_ops_t il_eth_net_dev_mon_ops = {
    .create = il_eth_net_dev_mon_create,
    .destroy = il_eth_net_dev_mon_destroy,
    .start = il_eth_net_dev_mon_start,
    .stop = il_eth_net_dev_mon_stop
};
