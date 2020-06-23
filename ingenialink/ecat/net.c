/*
* MIT License
*
* Copyright (c) 2017-2018 Ingenia-CAT S.L.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*/

#include <winsock2.h>
#include "net.h"
#include "servo.h"
#include "frame.h"

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <windows.h>
#include <inttypes.h>

#include "ingenialink/err.h"
#include "ingenialink/base/net.h"

#include "soem/soem/ethercat.h"
#include "lwip/netif.h"
#include "lwip/err.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "lwip/udp.h"
#include "lwip/netif/ethernet.h"
#include "lwip/netif/etharp.h"

/*******************************************************************************
* ECAT Master
******************************************************************************/

#define EC_TIMEOUTMON 500
#define UDP_OPEN_PORT           (uint16_t)1061U

/* Network instance */
struct netif tNetif;

/* Global reply data buffer */
uint8_t pReplyData[1024U];

ecx_contextt *context;

char IOmap[4096];

int expectedWKC;
boolean needlf;
volatile int globalwkc;
boolean inOP;
uint8 currentgroup = 0;
OSAL_THREAD_HANDLE thread1;
OSAL_THREAD_HANDLE thread2;
uint8 txbuf[1024];

/** Current RX fragment number */
uint8_t rxfragmentno = 0;
/** Complete RX frame size of current frame */
uint16_t rxframesize = 0;
/** Current RX data offset in frame */
uint16_t rxframeoffset = 0;
/** Current RX frame number */
uint16_t rxframeno = 0;
uint8 rxbuf[1024];
int size_of_rx = sizeof(rxbuf);

struct udp_pcb *ptUdpPcb;

ip_addr_t dstaddr;
err_t error;

uint8_t frame_received[1024];

/*******************************************************************************/

// FoE
#define FWBUFSIZE (8 * 1024 * 1024)

uint8 ob;
uint16 ow;
uint32 data;
char filename[256];
char filebuffer[FWBUFSIZE]; // 8MB buffer
int filesize;
int j;
uint16 argslave;


/*******************************************************************************
* Private
******************************************************************************/

/**
* Destroy ECAT network.
*
* @param [in] ctx
*	Context (il_net_t *).
*/
static void ecat_net_destroy(void *ctx)
{
	il_ecat_net_t *this = ctx;

	il_net_base__deinit(&this->net);

	free(this);
}

static int not_supported(void)
{
	ilerr__set("Functionality not supported");

	return IL_ENOTSUP;
}

bool crc_tabccitt_init_ecat = false;
uint16_t crc_tabccitt_ecat[256];

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
static void init_crcccitt_tab_ecat(void) {

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
		crc_tabccitt_ecat[i] = crc;
	}
	crc_tabccitt_init_ecat = true;
}

static uint16_t update_crc_ccitt_ecat(uint16_t crc, unsigned char c) {

	if (!crc_tabccitt_init_ecat) init_crcccitt_tab_ecat();
	return (crc << 8) ^ crc_tabccitt_ecat[((crc >> 8) ^ (uint16_t)c) & 0x00FF];

}

static uint16_t crc_calc_ecat(const uint16_t *buf, uint16_t u16Sz)
{

	uint16_t crc = 0x0000;
	uint8_t* pu8In = (uint8_t*)buf;

	for (uint16_t u16Idx = 0; u16Idx < u16Sz * 2; u16Idx++)
	{
		crc = update_crc_ccitt_ecat(crc, pu8In[u16Idx]);
	}
	return crc;
}

/**
* Process asynchronous statusword messages.
*
* @param [in] this
*	ECAT Network.
* @param [in] frame
*	IngeniaLink frame.
*/
static void process_statusword(il_ecat_net_t *this, uint8_t subnode, uint16_t *data)
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
*	ECAT Network (il_ecat_net_t *).
*/
int listener_ecat(void *args)
{
	int r;
	uint64_t buf;

	return 0;
}

void SignalHandlerECAT(int signal)
{
	if (signal == SIGINT || signal == SIGTERM || signal == SIGABRT) {
		// abort signal handler code
		il_ecat_net_t *this;
		this = calloc(1, sizeof(*this));
		if (!this) {
			ilerr__set("Network allocation failed");
			return NULL;
		}
		int r = il_net_master_stop(&this->net);
		r = il_net_ecat_close_socket(&this->net);
		printf("Unexpected termination: %i\n", signal);

		exit(-1);
	}
	else {
		// ...  
		printf("Unhandled signal exception: %i\n", signal);
	}
}

static il_net_t *il_ecat_net_create(const il_ecat_net_opts_t *opts)
{
	il_ecat_net_t *this;
	int r;

	typedef void(*SignalHandlerPointer)(int);

	/* Adding signal to catch exceptions */
	SignalHandlerPointer exc_handler_sigint, exc_handler_sigterm, exc_handler_sigabrt;
	exc_handler_sigint = signal(SIGINT, SignalHandlerECAT);
	exc_handler_sigterm = signal(SIGTERM, SignalHandlerECAT);
	exc_handler_sigabrt = signal(SIGABRT, SignalHandlerECAT);

	this = calloc(1, sizeof(*this));
	if (!this) {
		ilerr__set("Network allocation failed");
		return NULL;
	}

	/* initialize parent */
	r = il_net_base__init(&this->net, opts);
	if (r < 0)
		goto cleanup_this;
	this->net.ops = &il_ecat_net_ops;
	this->net.prot = IL_NET_PROT_ECAT;
	this->address_ip = opts->address_ip;
	this->port_ip = opts->port_ip;

	/* setup refcnt */
	this->refcnt = il_utils__refcnt_create(ecat_net_destroy, this);
	if (!this->refcnt)
		goto cleanup_refcnt;
	if (opts->connect_slave != 0) {
		r = il_net_connect(&this->net);
		if (r < 0)
			goto cleanup_this;
	}

	return &this->net;

cleanup_refcnt:
	il_utils__refcnt_destroy(this->refcnt);

cleanup_this:
	free(this);

	return NULL;
}

static void il_ecat_net_close_socket(il_net_t *net) {
	il_ecat_net_t *this = to_ecat_net(net);
	
	int r = 0;
	r = closesocket(this->server);
	WSACleanup();
	return r;
}

static void il_ecat_net_destroy(il_net_t *net)
{
	il_ecat_net_t *this = to_ecat_net(net);
	il_utils__refcnt_release(this->refcnt);
}

static int il_ecat_net_is_slave_connected(il_net_t *net, const char *ip) {

	il_ecat_net_t *this = to_ecat_net(net);
	int r = 0;
	int result = 0;


	return result;
}

static int il_ecat_net_reconnect(il_net_t *net)
{
	il_ecat_net_t *this = to_ecat_net(net);
	this->stop = 1;
	int r = -1;
    
	r = this->stop_reconnect;
	return r;
}

static int il_ecat_net_connect(il_net_t *net, const char *ip)
{
	il_ecat_net_t *this = to_ecat_net(net);

	int r = 0;

	return 0;
}

static void il_ecat_net__release(il_net_t *net)
{
	il_ecat_net_t *this = to_ecat_net(net);

	il_utils__refcnt_release(this->refcnt);
}

il_ecat_net_dev_list_t *il_ecat_net_dev_list_get()
{
	// TODO: Get slaves scanned
	il_ecat_net_dev_list_t *lst = NULL;
	il_ecat_net_dev_list_t *prev;

	return lst;
}

static int il_ecat_status_get(il_net_t *net)
{
	il_ecat_net_t *this = to_ecat_net(net);
	return this->stop;
}

static int il_ecat_mon_stop(il_net_t *net)
{
	il_ecat_net_t *this = to_ecat_net(net);
	this->stop_reconnect = 1;
	osal_thread_join(this->listener, NULL);
}

static il_net_servos_list_t *il_ecat_net_servos_list_get(
	il_net_t *net, il_net_servos_on_found_t on_found, void *ctx)
{
	int r;
	uint64_t vid;
	il_net_servos_list_t *lst;

	/* Check if there are slave in the network*/
	if (ec_slavecount > 0) {
		/* try to read the vendor id register to see if a servo is alive */
		r = il_net__read(net, 1, 1, VENDOR_ID_ADDR, &vid, sizeof(vid));
		if (r < 0) {
			printf("First try fail\n");
			r = il_net__read(net, 1, 1, VENDOR_ID_ADDR, &vid, sizeof(vid));
			if (r < 0) {
				printf("Second try fail\n");
				return NULL;
			}
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
	}
	else {
		return NULL;
	}

	return lst;
}

// Monitoring ECAT
/**
* Monitoring remove all mapped registers
*/
static int *il_ecat_net_remove_all_mapped_registers(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t remove_val = 1;

	r = il_net__write(&this->net, 1, 0, 0x00E2, &remove_val, 2, 1, 0);
	if (r < 0) {

	}

	net->monitoring_number_mapped_registers = 0;
	return r;
}

/**
* Monitoring set mapped registers
*/
static int *il_ecat_net_set_mapped_register(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

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

/**
* Disturbance remove all mapped registers
*/
static int *il_ecat_net_disturbance_remove_all_mapped_registers(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t remove_val = 1;

	r = il_net__write(&this->net, 1, 0, 0x00E7, &remove_val, 2, 1, 0);
	if (r < 0) {

	}

	return r;
}

/**
* Disturbance set mapped reg
*/
static int *il_ecat_net_disturbance_set_mapped_register(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	// Always 0 for the moment
	net->disturbance_data_channels[channel].type = dtype;

	// Map address
	r = il_net__write(&this->net, 1, 0, 0x00E5, &address, 2, 1, 0);
	if (r < 0) {

	}

	return r;
}


/**
* Monitoring enable
*/
static int *il_ecat_net_enable_monitoring(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t enable_monitoring_val = 1;
	r = il_net__write(&this->net, 1, 0, 0x00C0, &enable_monitoring_val, 2, 1, 0);
	if (r < 0) {

	}
	return r;
}

static int *il_ecat_net_disable_monitoring(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t disable_monitoring_val = 0;
	r = il_net__write(&this->net, 1, 0, 0x00C0, &disable_monitoring_val, 2, 1, 0);
	if (r < 0) {

	}
	return r;
}

static int *il_ecat_net_read_monitoring_data(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint64_t vid;

	r = il_net__read(&this->net, 1, 0, 0x00B2, &vid, sizeof(vid));
	if (r < 0) {

	}
}

/**
* Monitor event callback.
*/
static void on_ser_evt(void *ctx, ser_dev_evt_t evt, const ser_dev_t *dev)
{
	il_ecat_net_dev_mon_t *this = ctx;

	if (evt == SER_DEV_EVT_ADDED)
		this->on_evt(this->ctx, IL_NET_DEV_EVT_ADDED, dev->path);
	else
		this->on_evt(this->ctx, IL_NET_DEV_EVT_REMOVED, dev->path);
}

static il_net_dev_mon_t *il_ecat_net_dev_mon_create(void)
{
	il_ecat_net_dev_mon_t *this;

	this = malloc(sizeof(*this));
	if (!this) {
		ilerr__set("Monitor allocation failed");
		return NULL;
	}

	this->mon.ops = &il_ecat_net_dev_mon_ops;
	this->running = 0;

	return &this->mon;
}

static void il_ecat_net_dev_mon_destroy(il_net_dev_mon_t *mon)
{
	il_ecat_net_dev_mon_t *this = to_ecat_mon(mon);

	il_net_dev_mon_stop(mon);

	free(this);
}

static int il_ecat_net_dev_mon_start(il_net_dev_mon_t *mon,
	il_net_dev_on_evt_t on_evt,
	void *ctx)
{
	il_ecat_net_dev_mon_t *this = to_ecat_mon(mon);

	if (this->running) {
		ilerr__set("Monitor already running");
		return IL_EALREADY;
	}

	/* store context and bring up monitor */
	this->ctx = ctx;
	this->on_evt = on_evt;
	this->smon = ser_dev_monitor_init(on_ser_evt, this);
	if (!this->smon) {
		ilerr__set("Network device monitor allocation failed (%s)",
			sererr_last());
		return IL_EFAIL;
	}

	this->running = 1;

	return 0;
}

static void il_ecat_net_dev_mon_stop(il_net_dev_mon_t *mon)
{
	il_ecat_net_dev_mon_t *this = to_ecat_mon(mon);

	if (this->running) {
		ser_dev_monitor_stop(this->smon);
		this->running = 0;
	}
}

static int il_ecat_net__read(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
	void *buf, size_t sz)
{
	il_ecat_net_t *this = to_ecat_net(net);
	int r;
	(void)id;

	osal_mutex_lock(this->net.lock);
	r = net_send(this, subnode, (uint16_t)address, NULL, 0, 0, net);
	if (r < 0) {
		goto unlock;
	}

	int num_retries = 0;
	while (num_retries < NUMBER_OP_RETRIES)
	{
		uint16_t *monitoring_raw_data = NULL;
		r = net_recv(this, subnode, (uint16_t)address, buf, sz, monitoring_raw_data, net);
		if (r == IL_ETIMEDOUT || r == IL_EWRONGREG) 
		{
			++num_retries;
			printf("Frame lost, retry %i\n", num_retries);
		}
		else 
		{
			break;
		}

	}
	
	
	if (r < 0) 
	{
		if (r == IL_ETIMEDOUT || r == IL_EWRONGREG)
		{
			
		}
		goto unlock;
	}
		

unlock:
	osal_mutex_unlock(this->net.lock);

	return r;
}

static int il_ecat_net__write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
	const void *buf, size_t sz, int confirmed, uint16_t extended)
{
	il_ecat_net_t *this = to_ecat_net(net);

	int r;

	(void)id;
	(void)confirmed;

	osal_mutex_lock(this->net.lock);


	r = net_send(this, subnode, (uint16_t)address, buf, sz, extended, net);
	if (r < 0)
		goto unlock;

	int num_retries = 0;
	while (num_retries < NUMBER_OP_RETRIES)
	{
		r = net_recv(this, subnode, (uint16_t)address, NULL, 0, NULL, NULL);
		if (r == IL_ETIMEDOUT || r == IL_EWRONGREG) 
		{
			++num_retries;
			printf("Frame lost, retry %i\n", num_retries);
		}
		else 
		{
			break;
		}
	}

	if (r < 0) 
	{
		if (r == IL_ETIMEDOUT || r == IL_EWRONGREG)
		{

		}
		goto unlock;
	}
	
unlock:
	osal_mutex_unlock(this->net.lock);

	return r;
}

static int il_ecat_net__wait_write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
	const void *buf, size_t sz, int confirmed, uint16_t extended)
{
	il_ecat_net_t *this = to_ecat_net(net);

	int r;

	(void)id;
	(void)confirmed;

	osal_mutex_lock(this->net.lock);


	r = net_send(this, subnode, (uint16_t)address, buf, sz, extended, net);
	if (r < 0)
		goto unlock;

	Sleep(1000);

	int num_retries = 0;
	while (num_retries < NUMBER_OP_RETRIES)
	{
		r = net_recv(this, subnode, (uint16_t)address, NULL, 0, NULL, NULL);
		if (r == IL_ETIMEDOUT || r == IL_EWRONGREG) 
		{
			++num_retries;
			printf("Frame lost, retry %i\n", num_retries);
		}
		else 
		{
			break;
		}
	}
	if (r < 0) 
	{
		goto unlock;
	}

unlock:
	osal_mutex_unlock(this->net.lock);

	return r;
}

typedef union
{
	uint64_t u64;
	uint16_t u16[4];
} UINT_UNION_T;

static int net_send(il_ecat_net_t *this, uint8_t subnode, uint16_t address, const void *data,
	size_t sz, uint16_t extended, il_net_t *net)
{
	int finished = 0;
	uint8_t cmd;

	cmd = sz ? ECAT_MCB_CMD_WRITE : ECAT_MCB_CMD_READ;

	while (!finished) {
		int r;
		uint16_t frame[ECAT_MCB_FRAME_SZ];
		uint16_t hdr_h, hdr_l, crc;
		size_t chunk_sz;

		/* header */
		hdr_h = (ECAT_MCB_NODE_DFLT << 4) | (subnode);
		*(uint16_t *)&frame[ECAT_MCB_HDR_H_POS] = hdr_h;
		hdr_l = (address << 4) | (cmd << 1) | (extended);
		*(uint16_t *)&frame[ECAT_MCB_HDR_L_POS] = hdr_l;

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
		memcpy(&frame[ECAT_MCB_DATA_POS], &u.u16[0], 8);

		/* crc */
		crc = crc_calc_ecat(frame, ECAT_MCB_CRC_POS);
		frame[ECAT_MCB_CRC_POS] = crc;

		/* send frame */
		if (extended == 1) {
			uint16_t frame_size = sizeof(uint16_t) * ECAT_MCB_FRAME_SZ;
			uint8_t extended_frame[1024];

			il_reg_dtype_t type = net->disturbance_data_channels[0].type;
			
			void* pData;
			switch (type) {
				case IL_REG_DTYPE_U16:
					pData = net->disturbance_data_channels[0].value.disturbance_data_u16;
					break;
				case IL_REG_DTYPE_S16:
					pData = net->disturbance_data_channels[0].value.disturbance_data_s16;
					break;
				case IL_REG_DTYPE_U32:
					pData = net->disturbance_data_channels[0].value.disturbance_data_u32;
					break;
				case IL_REG_DTYPE_S32:
					pData = net->disturbance_data_channels[0].value.disturbance_data_s32;
					break;
				case IL_REG_DTYPE_FLOAT:
					pData = net->disturbance_data_channels[0].value.disturbance_data_flt;
					break;
			}

			memcpy(&extended_frame[0], frame, frame_size);
			memcpy(&extended_frame[frame_size], pData, 1024 - frame_size);

			int wkc = 0;
			struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, net->disturbance_data_size + frame_size, PBUF_RAM);
			memcpy(p->payload, extended_frame, net->disturbance_data_size + frame_size);
			error = udp_sendto(ptUdpPcb, p, &dstaddr, 1061);
			pbuf_free(p);

			if (error < 0)
				return ilerr__ser(error);
		}
		else {
			int wkc = 0;
			error = -1;
			struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, 14, PBUF_RAM);
			if (p != NULL) {
				memcpy(p->payload, frame, 14);
				error = udp_sendto(ptUdpPcb, p, &dstaddr, 1061);
			}
			pbuf_free(p);

			// r = send(this->server, (const char*)&frame[0], sizeof(frame), 0);
			// printf("Not extended, result of send: %i\n", r);
			if (error < 0)
				return ilerr__ser(error);
		}
		finished = 1;
		/*if (extended == 1) {
		r = send(server, (const char*)&net->disturbance_data[0], net->disturbance_data_size, 0);
		if (r < 0)
		return ilerr__ser(r);
		}*/
	}
	// printf("End send\n");

	return 0;
}

static int net_recv(il_ecat_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
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
	/* read next frame */
	int r = 0;
	// r = recv(this->server, (char*)&pBuf[0], sizeof(frame), 0);
	int wkc = 0;
	ec_mbxbuft MbxIn;
	wkc = ecx_mbxreceive(context, 1, (ec_mbxbuft *)&MbxIn, EC_TIMEOUTRXM);
	int s32SzRead = 1024;
	wkc = ecx_EOErecv(context, 1, 0, &s32SzRead, rxbuf, EC_TIMEOUTRXM);

	/* Obtain the frame received */
	memcpy(frame, (uint8_t*)frame_received, 1024);

	/* process frame: validate CRC, address, ACK */
	crc = *(uint16_t *)&frame[6];
	uint16_t crc_res = crc_calc_ecat((uint16_t *)frame, 6);
	if (crc_res != crc) {
		ilerr__set("Communications error (CRC mismatch)");
		return IL_EIO;
	}

	/* TODO: Check subnode */

	/* Check ACK */
	hdr_l = *(uint16_t *)&frame[ECAT_MCB_HDR_L_POS];
	int cmd = (hdr_l & ECAT_MCB_CMD_MSK) >> ECAT_MCB_CMD_POS;
	if (cmd != ECAT_MCB_CMD_ACK) {
		uint32_t err;

		err = __swap_be_32(*(uint32_t *)&frame[ECAT_MCB_DATA_POS]);

		ilerr__set("Communications error (NACK -> %08x)", err);
		return IL_EIO;
	}

	/* Check if register received is the same that we asked for.  */
	if ((hdr_l >> 4) != address) 
	{
		return IL_EWRONGREG;
	}


	extended_bit = (hdr_l & ECAT_MCB_PENDING_MSK) >> ECAT_MCB_PENDING_POS;
	if (extended_bit == 1) {
		/* Check if we are reading monitoring data */
		if (address == 0x00B2) {
			/* Monitoring */
			/* Read size of data */
			memcpy(buf, &(frame[ECAT_MCB_DATA_POS]), 2);
			uint16_t size = *(uint16_t*)buf;
			memcpy(net->monitoring_raw_data, (uint8_t*)&frame_received[14], size);

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
			memcpy(buf, &(frame[ECAT_MCB_DATA_POS]), 2);
			uint16_t size = *(uint16_t*)buf;
			memcpy(net->extended_buff, (char*)&frame_received[14], size);
		}
	}
	else {
		memcpy(buf, &(frame[ECAT_MCB_DATA_POS]), sz);
	}

	return 0;
}

// =================================================================================================================
// ECAT
// =================================================================================================================

static err_t LWIP_EthernetifOutput(struct netif *ptNetIfHnd, struct pbuf *ptBuf)
{
	err_t tErr = ERR_OK;

	uint8_t frame6[1024];
	memcpy(frame6, ptBuf->payload, ptBuf->len);

	int i = ecx_EOEsend(context, 1, 0, ptBuf->tot_len, ptBuf->payload, EC_TIMEOUTRXM);



	uint16_t u16Ret = 0;
	if (u16Ret != (uint16_t)0U)
	{
		tErr = ERR_IF;
	}

	if (ptBuf != NULL)
	{
		pbuf_free(ptBuf);
		ptBuf = NULL;
	}

	return tErr;
}

static err_t LWIP_EthernetifInit(struct netif *ptNetIfHnd)
{
	ptNetIfHnd->output = etharp_output;
	ptNetIfHnd->linkoutput = LWIP_EthernetifOutput;

	return ERR_OK;
}

void LWIP_EthernetifInp(void* pData, uint16_t u16SizeBy)
{
	err_t tError;
	struct pbuf* pBuf = NULL;

	/* Allocate data and copy from source */
	pBuf = pbuf_alloc(PBUF_RAW, u16SizeBy, PBUF_POOL);
	if (pBuf != NULL) {
		memcpy((void*)pBuf->payload, (const void*)pData, u16SizeBy);
		pBuf->len = u16SizeBy;

		tError = tNetif.input(pBuf, &tNetif);
		if (tError == ERR_OK)
		{
			pbuf_free(pBuf);
			pBuf = NULL;
		}
	}
}

static void LWIP_UdpReceiveData(void* pArg, struct udp_pcb* ptUdpPcb, struct pbuf* ptBuf,
	const ip_addr_t* ptAddr, u16_t u16Port)
{
	memcpy(frame_received, ptBuf->payload, ptBuf->len);
}

OSAL_THREAD_FUNC mailbox_reader(void *lpParam)
{
	context = (ecx_contextt *)lpParam;
	int wkc = 0;
	ec_mbxbuft MbxIn;
	ec_mbxheadert * MbxHdr = (ec_mbxheadert *)MbxIn;

	IP4_ADDR(&dstaddr, 192, 168, 2, 22);

	ip4_addr_t tIpAddr, tNetmask, tGwIpAddr;

	/* Initilialize the LwIP stack without RTOS */
	lwip_init();

	/* IP addresses initialization */
	IP4_ADDR(&tIpAddr, 192, 168,
		2, 22);
	IP4_ADDR(&tNetmask, 255, 255,
		255, 0);
	IP4_ADDR(&tGwIpAddr, 192, 168,
		2, 1);

	/* IP addresses initialization */
	IP4_ADDR(&tIpAddr, 192, 168,
		2, 22);
	IP4_ADDR(&tNetmask, 255, 255,
		255, 0);
	IP4_ADDR(&tGwIpAddr, 192, 168,
		2, 1);

	/* Add the network interface */
	netif_add(&tNetif, &tGwIpAddr, &tNetmask, &tIpAddr, NULL,
		&LWIP_EthernetifInit, &ethernet_input);
	netif_set_default(&tNetif);
	netif_set_up(&tNetif);
	tNetif.flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

	/* Open the Upd port and link receive callback */
	ptUdpPcb = udp_new();
	/* Link UDP callback */
	udp_recv(ptUdpPcb, LWIP_UdpReceiveData, (void*)NULL);
	/* UDP connect */
	error = udp_connect(ptUdpPcb, &tIpAddr, 1061);

	//osal_usleep(100000);
}

/** registered EoE hook */
int eoe_hook(ecx_contextt * context, uint16 slave, void * eoembx)
{
	//printf("EoE Hook!\n");
	int wkc;
	/* 
	* 	Pass received Mbx data to EoE recevive fragment function that
	* 	that will start/continue fill an Ethernet frame buffer
	*/
	size_of_rx = sizeof(rxbuf);
	//printf("rxfragmentno: %d", rxfragmentno);
	wkc = ecx_EOEreadfragment(eoembx,
		&rxfragmentno,
		&rxframesize,
		&rxframeoffset,
		&rxframeno,
		&size_of_rx,
		rxbuf);
	int r = rxframesize;

	LWIP_EthernetifInp((uint16_t*)rxbuf, sizeof(rxbuf));
	/* wkc == 1 would mean a frame is complete , last fragment flag have been set and all
	* other checks must have past
	*/
	if (wkc > 0)
	{
		ec_etherheadert *bp = (ec_etherheadert *)rxbuf;
		uint16 type = ntohs(bp->etype);
		if (type == ETH_P_ECAT)
		{
			/* Check that the TX and RX frames are EQ */
			if (memcmp(rxbuf, txbuf, size_of_rx))
			{
				//printf("memcmp result != 0\n");
			}
			else
			{
				//printf("memcmp result == 0\n");
			}
			/* Send a new frame */
			int ixme;
			for (ixme = ETH_HEADERSIZE; ixme < sizeof(txbuf); ixme++)
			{
				txbuf[ixme] = (uint8)rand();
			}

			ecx_EOEsend(context, 1, 0, sizeof(txbuf), txbuf, EC_TIMEOUTRXM);
		}
		else
		{
			//printf("Skip type 0x%x\n", type);
		}
	}

	/* No point in returning as unhandled */
	return ec_slavecount;
}

void init_eoe(ecx_contextt * context)
{
	printf("Init EoE\n");
	/* Set the HOOK */
	ecx_EOEdefinehook(context, eoe_hook);

	eoe_param_t ipsettings, re_ipsettings;
	memset(&ipsettings, 0, sizeof(ipsettings));
	memset(&re_ipsettings, 0, sizeof(re_ipsettings));

	printf("IP configuration\n");
	ipsettings.ip_set = 1;
	ipsettings.subnet_set = 1;
	ipsettings.default_gateway_set = 1;

	EOE_IP4_ADDR_TO_U32(&ipsettings.ip, 192, 168, 2, 22);
	EOE_IP4_ADDR_TO_U32(&ipsettings.subnet, 255, 255, 255, 0);
	EOE_IP4_ADDR_TO_U32(&ipsettings.default_gateway, 192, 168, 2, 1);

	printf("IP configured\n");

	/* Send a set IP request */
	ecx_EOEsetIp(context, 1, 0, &ipsettings, EC_TIMEOUTRXM);

	/* Send a get IP request, should return the expected IP back */
	ecx_EOEgetIp(context, 1, 0, &re_ipsettings, EC_TIMEOUTRXM);

	/* Create a asyncronous EoE reader */
	osal_thread_create(&thread2, 128000, &mailbox_reader, &ecx_context);
}

int *il_ecat_net_master_startup(il_net_t **net, char *ifname, char *if_address_ip)
{
	int i, oloop, iloop, chk;
	needlf = FALSE;
	inOP = FALSE;

	il_ecat_net_opts_t opts;
	opts.address_ip = if_address_ip;
	opts.timeout_rd = IL_NET_TIMEOUT_RD_DEF;
	opts.timeout_wr = IL_NET_TIMEOUT_WR_DEF;
	opts.connect_slave = 1;
	opts.port_ip = 1061;
	opts.port = "";

	*net = il_ecat_net_create(&opts);
	if (!*net) {
		printf("FAIL");
		return IL_EFAIL;
	}

	printf("Starting EtherCAT Master\n");
	/* initialise SOEM, bind socket to ifname */
	if (ec_init(ifname))
	{
		printf("ec_init on %s succeeded.\n", ifname);
		/* find and auto-config slaves */
		if (ec_config_init(FALSE) > 0)
		{
			printf("%d slaves found and configured.\n", ec_slavecount);

			printf("Slaves mapped, state to PRE_OP.\n");
			/* wait for all slaves to reach SAFE_OP state */
			ec_statecheck(0, EC_STATE_PRE_OP, EC_TIMEOUTSTATE * 4);

			printf("Calculated workcounter %d\n", expectedWKC);
			ec_slave[0].state = EC_STATE_PRE_OP;

			/* request OP state for all slaves */
			ec_writestate(0);
			chk = 200;

			/* wait for all slaves to reach OP state */
			do
			{
				ec_statecheck(0, EC_STATE_PRE_OP, 50000);
			} while (chk-- && (ec_slave[0].state != EC_STATE_PRE_OP));
			if (ec_slave[0].state == EC_STATE_PRE_OP)
			{
				printf("Pre-Operational state reached for all slaves.\n");
			} else
			{
				printf("Not all slaves reached operational state.\n");
				ec_readstate();
				for (i = 1; i <= ec_slavecount; i++)
				{
					if (ec_slave[i].state != EC_STATE_PRE_OP)
					{
						printf("Not all slaves are in PRE-OP\n");
						return -1;
					}
				}
			}
		}
		else
		{
			printf("No slaves found!\n");
		}

		init_eoe(&ecx_context);
		
	}
	else
	{
		printf("No socket connection on %s\nExcecute as root\n", ifname);
	}

	return ec_slavecount;
}

static int *il_ecat_net_master_stop(il_net_t **net)
{
    printf("Closing Socket\n");
    ec_slavecount = 0;

	/* Disconnecting and removing udp interface */
	udp_disconnect(ptUdpPcb);
	udp_remove(ptUdpPcb);
    
	/* Remove the network interface */
	netif_remove(&tNetif);
	ec_close();
}

int input_bin(char *fname, int *length)
{
    FILE *fp;

	int cc = 0, c;

    fp = fopen(fname, "rb");
    if(fp == NULL)
        return 0;
	while (((c = fgetc(fp)) != EOF) && (cc < FWBUFSIZE))
		filebuffer[cc++] = (uint8)c;
	*length = cc;
	fclose(fp);
	return 1;
}


enum update_error
{
	UP_NOERROR = 0,
	UP_STATEMACHINE_ERROR = -2,
	UP_NOT_IN_BOOT_ERROR = -3,
	UP_EEPROM_PDI_ERROR = -4,
	UP_EEPROM_FILE_ERROR = -6,
	UP_NOT_FOUND_ERROR = -7,
	UP_NO_SOCKET = -8,
	UP_FORCE_BOOT_ERROR = -9
};

int *il_ecat_net_change_state(uint16_t slave, ec_state state) 
{
	ec_slave[slave].state = state;
	ec_writestate(slave);
	
	if (ec_statecheck(slave, state, EC_TIMEOUTSTATE) != state) {
		return UP_STATEMACHINE_ERROR;
	}
	return UP_NOERROR;
}

/**
 * Update Firmware using FoE
*/
static int *il_ecat_net_update_firmware(il_net_t **net, char *ifname, uint16_t slave, char *filename, bool is_summit) 
{
	printf(filename);
	printf("Starting firmware update example\n");
	int r = 0;
	/* initialise SOEM, bind socket to ifname */
	if (ec_init(ifname))
	{
		printf("ec_init on %s succeeded.\n", ifname);
		/* find and auto-config slaves */

		if (ec_config_init(FALSE) > 0)
		{
			printf("%d slaves found and configured.\n", ec_slavecount);

			printf("Request init state for slave %d\n", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_INIT) != UP_NOERROR) {
				printf("Slave %d cannot enter into state INIT.\n", slave);
				return UP_STATEMACHINE_ERROR;
			}

			printf("Slave %d state to INIT.\n", slave);

			printf("Request pre-op state for slave %d\n", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_PRE_OP) != UP_NOERROR) {
				printf("Slave %d cannot enter into state PRE-OP.\n", slave);
				printf("Application not detected. Trying Bootloader process..\n");
			}
			else
			{
				if (!is_summit) 
				{
					printf("Writing COCO FORCE BOOT password through SDO\n");
					uint32 u32val = 0x424F4F54;
					if (ec_SDOwrite(slave, 0x5EDE, 0x00, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTTXM) <= 0) 
					{
						printf("Force Boot error\n");
						return UP_FORCE_BOOT_ERROR;
					}
				}

				printf("Request init state for slave %d\n", slave);
				if (il_ecat_net_change_state(slave, EC_STATE_INIT) != UP_NOERROR) {
					printf("Slave %d cannot enter into state INIT.\n", slave);
					return UP_STATEMACHINE_ERROR;
				}
				printf("Slave %d state to INIT.\n", slave);

				printf("Request BOOT state for slave %d\n", slave);
				if (il_ecat_net_change_state(slave, EC_STATE_BOOT) == UP_NOERROR && !is_summit) {
					printf("Slave %d entered into state BOOT. \n", slave);
					printf("Force COCO Boot not applied correctly.\n");
					return UP_STATEMACHINE_ERROR;
				}
				printf("As expected, Slave %d cannot enter into state BOOT the first time.\n", slave);

				ec_close();
				ec_init(ifname);
				ec_config_init(FALSE);
			}
			
			printf("Request init state for slave %d\n", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_INIT) != UP_NOERROR) {
				printf("Slave %d cannot enter into state INIT.\n", slave);
				return UP_STATEMACHINE_ERROR;
			}
			printf("Slave %d state to INIT.\n", slave);
			
			// MAGIC
			/* read BOOT mailbox data, master -> slave */
			data = ec_readeeprom(slave, ECT_SII_BOOTRXMBX, EC_TIMEOUTEEP);
			ec_slave[slave].SM[0].StartAddr = (uint16)LO_WORD(data);
			ec_slave[slave].SM[0].SMlength = (uint16)HI_WORD(data);
			/* store boot write mailbox address */
			ec_slave[slave].mbx_wo = (uint16)LO_WORD(data);
			/* store boot write mailbox size */
			ec_slave[slave].mbx_l = (uint16)HI_WORD(data);

			/* read BOOT mailbox data, slave -> master */
			data = ec_readeeprom(slave, ECT_SII_BOOTTXMBX, EC_TIMEOUTEEP);
			ec_slave[slave].SM[1].StartAddr = (uint16)LO_WORD(data);
			ec_slave[slave].SM[1].SMlength = (uint16)HI_WORD(data);
			/* store boot read mailbox address */
			ec_slave[slave].mbx_ro = (uint16)LO_WORD(data);
			/* store boot read mailbox size */
			ec_slave[slave].mbx_rl = (uint16)HI_WORD(data);

			printf(" SM0 A:%4.4x L:%4d F:%8.8x\n", ec_slave[slave].SM[0].StartAddr, ec_slave[slave].SM[0].SMlength,
				(int)ec_slave[slave].SM[0].SMflags);
			printf(" SM1 A:%4.4x L:%4d F:%8.8x\n", ec_slave[slave].SM[1].StartAddr, ec_slave[slave].SM[1].SMlength,
				(int)ec_slave[slave].SM[1].SMflags);
			/* program SM0 mailbox in for slave */
			ec_FPWR(ec_slave[slave].configadr, ECT_REG_SM0, sizeof(ec_smt), &ec_slave[slave].SM[0], EC_TIMEOUTRET);
			/* program SM1 mailbox out for slave */
			ec_FPWR(ec_slave[slave].configadr, ECT_REG_SM1, sizeof(ec_smt), &ec_slave[slave].SM[1], EC_TIMEOUTRET);

			printf("Request BOOT state for slave %d\n", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_BOOT) != UP_NOERROR) {
				printf("Slave %d cannot enter into state BOOT.\n", slave);
				return UP_STATEMACHINE_ERROR;
			}
			printf("Slave %d state to BOOT.\n", slave);

			if (ec_eeprom2pdi(slave) <= 0) 
			{
				return UP_EEPROM_PDI_ERROR;
			}
			printf("Slave %d EEPROM set to PDI.\n", slave);

			if (input_bin(filename, &filesize))
			{
				printf("File read OK, %d bytes.\n", filesize);
				printf("FoE write....");
				r = ec_FOEwrite(slave, filename, 0x70636675, filesize, &filebuffer, EC_TIMEOUTSTATE);
				printf("FOE write result %d.\n", r);
				if (r > 0) 
				{
					printf("Request init state for slave %d\n", slave);
					ec_slave[slave].state = EC_STATE_INIT;
					ec_writestate(slave);

					printf("Wait for drive to reset...\n");
					Sleep(4000);

					printf("FOE Process finished succesfully!!.\n");
				}
				else 
				{
					printf("Error during FoE process...");
				}
				
			}
			else
			{
				printf("File not read OK.\n");
				return UP_EEPROM_FILE_ERROR;
			}
			
		}
		else
		{
			printf("No slaves found!\n");
			return UP_NOT_FOUND_ERROR;
		}
		
	}
	else
	{
		printf("No socket connection on %s\nExecute as root\n",ifname);
		return UP_NO_SOCKET;
	}
	printf("End firmware update, close socket\n");
	/* stop SOEM, close socket */
	ec_close();
	return r;
}

/**
	EEPROM Tool
*/

#define MAXBUF 524288
#define STDBUF 2048
#define MINBUF 128
#define CRCBUF 14

#define MODE_NONE         0
#define MODE_READBIN      1
#define MODE_READINTEL    2
#define MODE_WRITEBIN     3
#define MODE_WRITEINTEL   4
#define MODE_WRITEALIAS   5
#define MODE_INFO         6

#define MAXSLENGTH        256

uint8 ebuf[MAXBUF];
uint8 ob;
uint16 ow;
int os;
int slave;
int alias;
ec_timet tstart,tend, tdif;
int wkc_eeprom;
int mode;
char sline[MAXSLENGTH];

#define IHEXLENGTH 0x20

void calc_crc(uint8 *crc, uint8 b)
{
   int j;
   *crc ^= b;
   for(j = 0; j <= 7 ; j++ )
   {
     if(*crc & 0x80)
        *crc = (*crc << 1) ^ 0x07;
     else
        *crc = (*crc << 1);
   }
}

uint16 SIIcrc(uint8 *buf)
{
   int i;
   uint8 crc;

   crc = 0xff;
   for( i = 0 ; i <= 13 ; i++ )
   {
      calc_crc(&crc , *(buf++));
   }
   return (uint16)crc;
}

int input_bin_eeprom(char *fname, int *length)
{
   FILE *fp;

   int cc = 0, c;

   fp = fopen(fname, "rb");
   if(fp == NULL)
      return 0;
   while (((c = fgetc(fp)) != EOF) && (cc < MAXBUF))
      ebuf[cc++] = (uint8)c;
   *length = cc;
   fclose(fp);

   return 1;
}

int input_intelhex(char *fname, int *start, int *length)
{
   FILE *fp;

   int c, sc, retval = 1;
   int ll, ladr, lt, sn, i, lval;
   int hstart, hlength, sum;

   fp = fopen(fname, "r");
   if(fp == NULL)
      return 0;
   hstart = MAXBUF;
   hlength = 0;
   sum = 0;
   do
   {
      memset(sline, 0x00, MAXSLENGTH);
      sc = 0;
      while (((c = fgetc(fp)) != EOF) && (c != 0x0A) && (sc < (MAXSLENGTH -1)))
         sline[sc++] = (uint8)c;
      if ((c != EOF) && ((sc < 11) || (sline[0] != ':')))
      {
         c = EOF;
         retval = 0;
         printf("Invalid Intel Hex format.\n");
      }
      if (c != EOF)
      {
         sn = sscanf(sline , ":%2x%4x%2x", &ll, &ladr, &lt);
         if ((sn == 3) && ((ladr + ll) <= MAXBUF) && (lt == 0))
         {
            sum = ll + (ladr >> 8) + (ladr & 0xff) + lt;
            if(ladr < hstart) hstart = ladr;
            for(i = 0; i < ll ; i++)
            {
               sn = sscanf(&sline[9 + (i << 1)], "%2x", &lval);
               ebuf[ladr + i] = (uint8)lval;
               sum += (uint8)lval;
            }
            if(((ladr + ll) - hstart) > hlength)
               hlength = (ladr + ll) - hstart;
            sum = (0x100 - sum) & 0xff;
            sn = sscanf(&sline[9 + (i << 1)], "%2x", &lval);
            if (!sn || ((sum - lval) != 0))
            {
               c = EOF;
               retval = 0;
               printf("Invalid checksum.\n");
            }
         }
      }
   }
   while (c != EOF);
   if (retval)
   {
      *length = hlength;
      *start = hstart;
   }
   fclose(fp);

   return retval;
}


int output_bin(char *fname, int length)
{
   FILE *fp;

   int cc;

   fp = fopen(fname, "wb");
   if(fp == NULL)
      return 0;
   for (cc = 0 ; cc < length ; cc++)
      fputc( ebuf[cc], fp);
   fclose(fp);

   return 1;
}

int output_intelhex(char *fname, int length)
{
   FILE *fp;

   int cc = 0, ll, sum, i;

   fp = fopen(fname, "w");
   if(fp == NULL)
      return 0;
   while (cc < length)
   {
      ll = length - cc;
      if (ll > IHEXLENGTH) ll = IHEXLENGTH;
      sum = ll + (cc >> 8) + (cc & 0xff);
      fprintf(fp, ":%2.2X%4.4X00", ll, cc);
      for (i = 0; i < ll; i++)
      {
         fprintf(fp, "%2.2X", ebuf[cc + i]);
         sum += ebuf[cc + i];
      }
      fprintf(fp, "%2.2X\n", (0x100 - sum) & 0xff);
      cc += ll;
   }
   fprintf(fp, ":00000001FF\n");
   fclose(fp);

   return 1;
}

int eeprom_read(int slave, int start, int length)
{
   int i, ainc = 4;
   uint16 estat, aiadr;
   uint32 b4;
   uint64 b8;
   uint8 eepctl;

   if((ec_slavecount >= slave) && (slave > 0) && ((start + length) <= MAXBUF))
   {
      aiadr = 1 - slave;
      eepctl = 2;
      ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl , EC_TIMEOUTRET); /* force Eeprom from PDI */
      eepctl = 0;
      ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl , EC_TIMEOUTRET); /* set Eeprom to master */

      estat = 0x0000;
      aiadr = 1 - slave;
      ec_APRD(aiadr, ECT_REG_EEPSTAT, sizeof(estat), &estat, EC_TIMEOUTRET); /* read eeprom status */
      estat = etohs(estat);
      if (estat & EC_ESTAT_R64)
      {
         ainc = 8;
         for (i = start ; i < (start + length) ; i+=ainc)
         {
            b8 = ec_readeepromAP(aiadr, i >> 1 , EC_TIMEOUTEEP);
            ebuf[i] = b8 & 0xFF;
            ebuf[i+1] = (b8 >> 8) & 0xFF;
            ebuf[i+2] = (b8 >> 16) & 0xFF;
            ebuf[i+3] = (b8 >> 24) & 0xFF;
            ebuf[i+4] = (b8 >> 32) & 0xFF;
            ebuf[i+5] = (b8 >> 40) & 0xFF;
            ebuf[i+6] = (b8 >> 48) & 0xFF;
            ebuf[i+7] = (b8 >> 56) & 0xFF;
         }
      }
      else
      {
         for (i = start ; i < (start + length) ; i+=ainc)
         {
            b4 = ec_readeepromAP(aiadr, i >> 1 , EC_TIMEOUTEEP) & 0xFFFFFFFF;
            ebuf[i] = b4 & 0xFF;
            ebuf[i+1] = (b4 >> 8) & 0xFF;
            ebuf[i+2] = (b4 >> 16) & 0xFF;
            ebuf[i+3] = (b4 >> 24) & 0xFF;
         }
      }

      return 1;
   }

   return 0;
}

int eeprom_write(int slave, int start, int length)
{
   int i, dc = 0;
   uint16 aiadr, *wbuf;
   uint8 eepctl;

   if((ec_slavecount >= slave) && (slave > 0) && ((start + length) <= MAXBUF))
   {
      aiadr = 1 - slave;
      eepctl = 2;
      ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl , EC_TIMEOUTRET); /* force Eeprom from PDI */
      eepctl = 0;
      ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl , EC_TIMEOUTRET); /* set Eeprom to master */

      aiadr = 1 - slave;
      wbuf = (uint16 *)&ebuf[0];
      for (i = start ; i < (start + length) ; i+=2)
      {
         ec_writeeepromAP(aiadr, i >> 1 , *(wbuf + (i >> 1)), EC_TIMEOUTEEP);
         if (++dc >= 100)
         {
            dc = 0;
            printf(".");
            fflush(stdout);
         }
      }

      return 1;
   }

   return 0;
}

int eeprom_writealias(int slave, int alias, uint16 crc)
{
   uint16 aiadr;
   uint8 eepctl;
   int ret;

   if((ec_slavecount >= slave) && (slave > 0) && (alias <= 0xffff))
   {
      aiadr = 1 - slave;
      eepctl = 2;
      ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl , EC_TIMEOUTRET); /* force Eeprom from PDI */
      eepctl = 0;
      ec_APWR(aiadr, ECT_REG_EEPCFG, sizeof(eepctl), &eepctl , EC_TIMEOUTRET); /* set Eeprom to master */

      ret = ec_writeeepromAP(aiadr, 0x04 , alias, EC_TIMEOUTEEP);
      if (ret)
        ret = ec_writeeepromAP(aiadr, 0x07 , crc, EC_TIMEOUTEEP);

      return ret;
   }

   return 0;
}

static int *il_ecat_net_eeprom_tool(il_net_t **net, char *ifname, int slave, int mode, char *fname) 
{
	int w, rc = 0, estart, esize;
	int r = 0;
	uint16 *wbuf;

	/* initialise SOEM, bind socket to ifname */
	if (ec_init(ifname))
	{
		printf("ec_init on %s succeeded.\n",ifname);

		w = 0x0000;
		wkc_eeprom = ec_BRD(0x0000, ECT_REG_TYPE, sizeof(w), &w, EC_TIMEOUTSAFE);      /* detect number of slaves */
		if (wkc_eeprom > 0)
		{
			ec_slavecount = wkc_eeprom;

			printf("%d slaves found.\n",ec_slavecount);
			if((ec_slavecount >= slave) && (slave > 0))
			{
				if ((mode == MODE_INFO) || (mode == MODE_READBIN) || (mode == MODE_READINTEL))
				{
					tstart = osal_current_time();
					eeprom_read(slave, 0x0000, MINBUF); // read first 128 bytes

					wbuf = (uint16 *)&ebuf[0];
					printf("Slave %d data\n", slave);
					printf(" PDI Control      : %4.4X\n",*(wbuf + 0x00));
					printf(" PDI Config       : %4.4X\n",*(wbuf + 0x01));
					printf(" Config Alias     : %4.4X\n",*(wbuf + 0x04));
					printf(" Checksum         : %4.4X\n",*(wbuf + 0x07));
					printf("   calculated     : %4.4X\n",SIIcrc(&ebuf[0]));
					printf(" Vendor ID        : %8.8X\n",*(uint32 *)(wbuf + 0x08));
					printf(" Product Code     : %8.8X\n",*(uint32 *)(wbuf + 0x0A));
					printf(" Revision Number  : %8.8X\n",*(uint32 *)(wbuf + 0x0C));
					printf(" Serial Number    : %8.8X\n",*(uint32 *)(wbuf + 0x0E));
					printf(" Mailbox Protocol : %4.4X\n",*(wbuf + 0x1C));
					esize = (*(wbuf + 0x3E) + 1) * 128;
					if (esize > MAXBUF) esize = MAXBUF;
					printf(" Size             : %4.4X = %d bytes\n",*(wbuf + 0x3E), esize);
					printf(" Version          : %4.4X\n",*(wbuf + 0x3F));
				}
				if ((mode == MODE_READBIN) || (mode == MODE_READINTEL))
				{
					if (esize > MINBUF) 
					{
						eeprom_read(slave, MINBUF, esize - MINBUF); // read reminder
					}
						

					tend = osal_current_time();
					osal_time_diff(&tstart, &tend, &tdif);
					if (mode == MODE_READINTEL) output_intelhex(fname, esize);
					if (mode == MODE_READBIN)   output_bin(fname, esize);

					printf("\nTotal EEPROM read time :%ldms\n", (tdif.usec+(tdif.sec*1000000L)) / 1000);
				}
				if ((mode == MODE_WRITEBIN) || (mode == MODE_WRITEINTEL))
				{
					estart = 0;
					if (mode == MODE_WRITEINTEL) rc = input_intelhex(fname, &estart, &esize);
					if (mode == MODE_WRITEBIN)   rc = input_bin_eeprom(fname, &esize);

					if (rc > 0)
					{
						wbuf = (uint16 *)&ebuf[0];
						printf("Slave %d\n", slave);
						printf(" Vendor ID        : %8.8X\n", *(uint32 *)(wbuf + 0x08));
						printf(" Product Code     : %8.8X\n", *(uint32 *)(wbuf + 0x0A));
						printf(" Revision Number  : %8.8X\n", *(uint32 *)(wbuf + 0x0C));
						printf(" Serial Number    : %8.8X\n", *(uint32 *)(wbuf + 0x0E));

						printf("Busy");
						fflush(stdout);
						tstart = osal_current_time();
						eeprom_write(slave, estart, esize);
						tend = osal_current_time();
						osal_time_diff(&tstart, &tend, &tdif);

						printf("\nTotal EEPROM write time :%ldms\n", (tdif.usec + (tdif.sec * 1000000L)) / 1000);
					}
					else
					{
						printf("Error reading file, abort.\n");
						r = -1;
					}
				}
				if (mode == MODE_WRITEALIAS)
				{
					if( eeprom_read(slave, 0x0000, CRCBUF) ) // read first 14 bytes
					{
						wbuf = (uint16 *)&ebuf[0];
						*(wbuf + 0x04) = alias;
						if(eeprom_writealias(slave, alias, SIIcrc(&ebuf[0])))
						{
							printf("Alias %4.4X written successfully to slave %d\n", alias, slave);
						}
						else
						{
							printf("Alias not written\n");
						}
					}
					else
					{
						printf("Could not read slave EEPROM");
					}
				}
			}
			else
			{
				printf("Slave number outside range.\n");
			}
		}
		else
		{
			printf("No slaves found!\n");
			r = -1;
		}
		printf("End, close socket\n");
		/* stop SOEM, close socket */
		ec_close();
	}
	else
	{
		printf("No socket connection on %s\nExcecute as root\n",ifname);
	}
	return r;
}

int Everestsetup(uint16 slave)
{
	int retval;
	uint8 u8val;
	uint16 u16val;
	int8 i8val;

	retval = 0;

	/*u8val = 0;
	retval += ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	uint32 u32Map = 0x60400010;
	retval += ec_SDOwrite(slave, 0x1600, 0x01, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x607A0020;
	retval += ec_SDOwrite(slave, 0x1600, 0x02, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u8val = 2;
	retval += ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1A00, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	u32Map = 0x60410010;
	retval += ec_SDOwrite(slave, 0x1A00, 0x01, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x60640020;
	retval += ec_SDOwrite(slave, 0x1A00, 0x02, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x20780020;
	retval += ec_SDOwrite(slave, 0x1A00, 0x03, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u8val = 3;
	retval += ec_SDOwrite(slave, 0x1A00, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1601, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	u32Map = 0x60400010;
	retval += ec_SDOwrite(slave, 0x1601, 0x01, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x607A0020;
	retval += ec_SDOwrite(slave, 0x1601, 0x02, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u8val = 2;
	retval += ec_SDOwrite(slave, 0x1601, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1A01, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	u32Map = 0x60410010;
	retval += ec_SDOwrite(slave, 0x1A01, 0x01, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x60640020;
	retval += ec_SDOwrite(slave, 0x1A01, 0x02, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x20780020;
	retval += ec_SDOwrite(slave, 0x1A01, 0x03, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u8val = 3;
	retval += ec_SDOwrite(slave, 0x1A01, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1602, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	u32Map = 0x60400010;
	retval += ec_SDOwrite(slave, 0x1602, 0x01, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x607A0020;
	retval += ec_SDOwrite(slave, 0x1602, 0x02, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u8val = 2;
	retval += ec_SDOwrite(slave, 0x1602, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1A02, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	u32Map = 0x60410010;
	retval += ec_SDOwrite(slave, 0x1A02, 0x01, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x60640020;
	retval += ec_SDOwrite(slave, 0x1A02, 0x02, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u32Map = 0x20780020;
	retval += ec_SDOwrite(slave, 0x1A02, 0x03, FALSE, sizeof(u32Map), &u32Map, EC_TIMEOUTRXM);
	u8val = 3;
	retval += ec_SDOwrite(slave, 0x1A02, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);*/

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	//printf("Set reg 0x1c1200 to 0 = %d\n", retval);
	u16val = 0x1600;
	retval += ec_SDOwrite(slave, 0x1c12, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTRXM);
	//printf("Set reg 0x1c1201 to 0x1600 = %d\n", retval);
	u8val = 1;
	retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	//printf("Set reg 0x1c1200 to 1 = %d\n", retval);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	//printf("Set reg 0x1c1300 to 0 = %d\n", retval);
	u16val = 0x1a00;
	retval += ec_SDOwrite(slave, 0x1c13, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTRXM);
	//printf("Set reg 0x1c1301 to 0x1a00 = %d\n", retval);
	u8val = 1;
	retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	//printf("Set reg 0x1c1300 to 1 = %d\n", retval);


	//u8val = 8;
	//retval += ec_SDOwrite(slave, 0x6060, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	//i8val = -4;
	//retval += ec_SDOwrite(slave, 0x60C2, 0x01, FALSE, sizeof(u8val), &i8val, EC_TIMEOUTRXM);
	//u8val = 5;
	//retval += ec_SDOwrite(slave, 0x60C2, 0x02, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	// set some motor parameters, just as example
	//u16val = 1200; // max motor current in mA
	//    retval += ec_SDOwrite(slave, 0x8010, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);
	//u16val = 150; // motor coil resistance in 0.01ohm
	//    retval += ec_SDOwrite(slave, 0x8010, 0x04, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTSAFE);

	// set other nescessary parameters as needed
	// .....

	while (EcatError) printf("%s", ec_elist2string());

	printf("Everest slave %d set, retval = %d\n", slave, retval);
	return 1;
}

int *il_ecat_net_force_error(il_net_t **net, char *ifname, char *if_address_ip)
{
	int i, j, oloop, iloop, wkc_count, chk, slc;
    UINT mmResult;

    needlf = FALSE;
    inOP = FALSE;

   	printf("Slave force error\n");

	/* initialise SOEM, bind socket to ifname */
   	if (ec_init(ifname)) 
	{
		printf("ec_init on %s succeeded.\n",ifname);
      	/* find and auto-config slaves */

		if ( ec_config_init(FALSE) > 0 )
      	{

			printf("%d slaves found and configured.\n",ec_slavecount);
			if (ec_slavecount > 0) 
			{
				int slave = 1;
				ec_slave[slave].PO2SOconfig = &Everestsetup;

				ec_config_map(&IOmap);
						
				ec_configdc();
				ec_slave[slave].state = EC_STATE_PRE_OP;

				/* request PRE-OP state for all slaves */
				ec_writestate(slave);
				chk = 200;
				/* wait for all slaves to reach OP state */
				do
				{
					ec_statecheck(slave, EC_STATE_PRE_OP, 50000);
				}
				while (chk-- && (ec_slave[slave].state != EC_STATE_PRE_OP));
				Sleep(2000);
				int retval = 0;
				
				uint16_t objectValue = 0x10;
				retval += ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(objectValue), &objectValue, EC_TIMEOUTSAFE);
				printf("retval = %i\n", retval);
				Sleep(1000);
				ec_slave[slave].state = EC_STATE_SAFE_OP;
				/* request SAFE_OP state for all slaves */
				int r = ec_writestate(slave);
				Sleep(1000);
				objectValue = 0x04;
				retval += ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(objectValue), &objectValue, EC_TIMEOUTSAFE);

			}
			else 
			{
				// No slaves found!
				return -2;
			}
		}
		else 
		{
			return -1;
		}
	}
	else 
	{
		return -1;
	}
	return 0;
}


/** ECAT network operations. */
const il_ecat_net_ops_t il_ecat_net_ops = {
	/* internal */
	._read = il_ecat_net__read,
	._write = il_ecat_net__write,
	._release = il_ecat_net__release,
	._wait_write = il_ecat_net__wait_write,
	._sw_subscribe = il_net_base__sw_subscribe,
	._sw_unsubscribe = il_net_base__sw_unsubscribe,
	._emcy_subscribe = il_net_base__emcy_subscribe,
	._emcy_unsubscribe = il_net_base__emcy_unsubscribe,
	/* public */
	.create = il_ecat_net_create,
	.destroy = il_ecat_net_destroy,
	.close_socket = il_ecat_net_close_socket,
	.connect = il_ecat_net_connect,
	.is_slave_connected = il_ecat_net_is_slave_connected,
	// .devs_list_get = il_eth_net_dev_list_get,
	.servos_list_get = il_ecat_net_servos_list_get,
	.status_get = il_ecat_status_get,
	.mon_stop = il_ecat_mon_stop,
	/* Monitornig */
	.remove_all_mapped_registers = il_ecat_net_remove_all_mapped_registers,
	.set_mapped_register = il_ecat_net_set_mapped_register,
	.enable_monitoring = il_ecat_net_enable_monitoring,
	.disable_monitoring = il_ecat_net_disable_monitoring,
	.read_monitoring_data = il_ecat_net_read_monitoring_data,
	/* Disturbance */
	.disturbance_remove_all_mapped_registers = il_ecat_net_disturbance_remove_all_mapped_registers,
	.disturbance_set_mapped_register = il_ecat_net_disturbance_set_mapped_register,
	/* Master EtherCAT */
	.master_startup = il_ecat_net_master_startup,
	.master_stop = il_ecat_net_master_stop,
	.update_firmware = il_ecat_net_update_firmware,
	.eeprom_tool = il_ecat_net_eeprom_tool,

	.force_error = il_ecat_net_force_error
};

/** MCB network device monitor operations. */
const il_net_dev_mon_ops_t il_ecat_net_dev_mon_ops = {
	.create = il_ecat_net_dev_mon_create,
	.destroy = il_ecat_net_dev_mon_destroy,
	.start = il_ecat_net_dev_mon_start,
	.stop = il_ecat_net_dev_mon_stop
};