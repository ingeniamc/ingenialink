#ifdef _WIN32
	#include <windows.h>
#else
	#include <unistd.h>
#define Sleep(x) usleep((x)*1000)
#endif
#include "net.h"
#include "servo.h"
#include "frame.h"

#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <stdbool.h>
#include <inttypes.h>

#include "ingenialink/err.h"
#include "ingenialink/base/net.h"
#include "external/log.c/src/log.h"

#include "external/SOEM/soem/ethercat.h"
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
#define MAX_FOE_TIMEOUT 2000000

/* Network instance */
struct netif tNetif;

ecx_contextt *context;

char IOmap[4096];

bool is_lwip_on = FALSE;

OSAL_THREAD_HANDLE mailbox_reader_thread;

struct udp_pcb *ptUdpPcb;

ip_addr_t dstaddr;
err_t error;

char *Ifname;
char *If_address_ip;
uint16_t slave_number;

/** Current RX fragment number */
uint8_t rxfragmentno = 0;
/** Complete RX frame size of current frame */
uint16_t rxframesize = 0;
/** Current RX data offset in frame */
uint16_t rxframeoffset = 0;
/** Current RX frame number */
uint16_t rxframeno = 0;
uint8 rxbuf[1024];
int size_of_rx;

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
static int il_ecat_net__read(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address, void *buf, size_t sz);
static int il_ecat_net__read_monitoring(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
	void *buf, size_t sz);
int il_ecat_net_SDO_read(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, int size, void *buf);
static int net_send(il_ecat_net_t *this, uint8_t subnode, uint16_t address, const void *data,
	size_t sz, uint16_t extended, il_net_t *net);
int il_ecat_net_SDO_write(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, int size, void *buf);
static int il_ecat_net_recv_monitoring(il_ecat_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
 	size_t sz, uint16_t *monitoring_raw_data, il_net_t *net, int num_bytes);
static int net_recv(il_ecat_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
	size_t sz, uint16_t *monitoring_raw_data, il_net_t *net);
static int process_monitoring_data(il_ecat_net_t *this, il_net_t *net);
static int il_ecat_net_remove_all_mapped_registers_v1(il_net_t *net);
static int il_ecat_net_remove_all_mapped_registers_v2(il_net_t *net);
static int il_ecat_net_disturbance_remove_all_mapped_registers_v1(il_net_t *net);
static int il_ecat_net_disturbance_remove_all_mapped_registers_v2(il_net_t *net);
static int il_ecat_net_disturbance_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address,
															uint8_t subnode, il_reg_dtype_t dtype,
															uint8_t size);
static int il_ecat_net_disturbance_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
														uint8_t subnode, il_reg_dtype_t dtype,
														uint8_t size);
static int il_ecat_net_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype);
static int il_ecat_net_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
											uint8_t subnode, il_reg_dtype_t dtype,
											uint8_t size);

int il_net_ecat_monitoring_mapping_registers[16] = {
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

int il_net_ecat_disturbance_mapping_registers[16] = {
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

restart:
{
	int error_count = 0;
	il_ecat_net_t *this = args;

	while (error_count < 3 && this != NULL && this->stop_reconnect == 0 ) {
		uint16_t sw;

		/* try to read the status word register to see if a servo is alive */
		if (this != NULL && this->status_check_stop == 0) {
			r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
			if (r < 0) {
				if (il_net_status_get(this) != IL_NET_STATE_DISCONNECTED) {
					error_count = error_count + 1;
				}
			}
			else {
				if (il_net_status_get(this) == IL_NET_STATE_DISCONNECTED) {
					log_info("DEVICE CONNECTED");
					il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);
				}
				error_count = 0;
				this->stop = IL_NET_STATE_CONNECTED;
				process_statusword(this, 1, sw);
			}

		}
		Sleep(100);
	}
	if (error_count == 3 && this != NULL && this->stop_reconnect == 0) {
		goto err;
	}
	else if (error_count < 3 && this != NULL && this->stop_reconnect != 0) {
		goto stop;
	}
	return 0;

err:
	if(this != NULL) {
		log_info("DEVICE DISCONNECTED");
		ilerr__set("Slave %i disconnected\n", this->slave);
		this->stop = IL_NET_STATE_DISCONNECTED;
		il_net__state_set(&this->net, IL_NET_STATE_DISCONNECTED);
		goto restart;
	}
	return 0;
stop:
	if (this != NULL) {
		log_debug("Stop reconnection thread");
		osal_mutex_unlock(this->net.lock);
		log_debug("Net unlocked");
	}
	return 0;
}
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
		log_error("Unexpected termination: %i", signal);

		exit(-1);
	}
	else {
		log_warn("Unhandled signal exception: %i", signal);
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
	this->ifname = opts->ifname;
	this->if_address_ip = opts->if_address_ip;
	this->slave = opts->slave;
	this->recv_timeout = EC_TIMEOUTRET;
	this->status_check_stop = 1;
	this->use_eoe_comms = opts->use_eoe_comms;

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
	#ifdef _WIN32
		r = shutdown(this->server, SD_BOTH);
		if (r == 0) { r = closesocket(this->server); }
	#else
		r = shutdown(this->server, SHUT_RDWR);
		if (r == 0) { r = close(this->server); }
	#endif

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

static int il_ecat_net_reconnect(il_ecat_net_t *this)
{
	this->stop = 1;
	int r = -1;
	int r2 = 0;
	uint16_t sw;

	while (r < 0 && this->stop_reconnect == 0) {
		log_debug("Disconnecting interface");
		ec_slavecount = 0;
		/* Disconnecting and removing udp interface */
		if (ptUdpPcb != NULL) {
			udp_disconnect(ptUdpPcb);
			udp_remove(ptUdpPcb);
		}

		/* Remove the network interface */
		log_debug("Removing network interface");
		netif_remove(&tNetif);
		ec_mbxempty(this->slave, 100000);
		context->EOEhook = NULL;

		log_debug("Closing EtherCAT interface");
		/* Close EtherCAT interface */
		ec_close();
		Sleep(1000);

		r2 = il_net_master_startup(&this->net, this->ifname, this->slave, 1);

		if (r2 > 0) {
			/* Try to read */
			Sleep(2000);
			r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
			if (r >= 0) {
				this->stop = 0;
				this->stop_reconnect = 0;
				log_info("DEVICE RECONNECTED");
				il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);
			}
		}
		Sleep(2000);
	}

	r = this->stop_reconnect;
	this->stop = 0;
	this->stop_reconnect = 0;
	return r;
}

static int il_ecat_net_connect(il_net_t *net, const char *ip)
{
	il_ecat_net_t *this = to_ecat_net(net);
	int r = 0;

	il_net__state_set(&this->net, IL_NET_STATE_CONNECTED);

	/* Start listener thread */
	this->stop = 0;
	this->stop_reconnect = 0;

	this->listener = osal_thread_create_(listener_ecat, this);
	if (!this->listener) {
		ilerr__set("Listener thread creation failed");
	}

	return 0;
}

static void il_ecat_net__release(il_net_t *net)
{
	il_ecat_net_t *this = to_ecat_net(net);

	il_utils__refcnt_release(this->refcnt);
}

il_ecat_net_dev_list_t *il_ecat_net_dev_list_get()
{
	/* TODO: Get slaves scanned */
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
	log_debug("Join listener thread");
	if (this->listener != NULL) {
		osal_thread_join(this->listener, NULL);
	}
	log_debug("Join mailbox thread");
	Sleep(1000);
}


static il_net_servos_list_t *il_ecat_net_servos_list_get(
	il_net_t *net, il_net_servos_on_found_t on_found, void *ctx)
{
	int r;
	uint64_t vid;
	il_net_servos_list_t *lst;
	il_ecat_net_t *this = to_ecat_net(net);

	/* Check if there are slave in the network*/
	if (ec_slavecount > 0) {
		/* try to read the vendor id register to see if a servo is alive */
		r = il_ecat_net__read(net, this->slave, 1, VENDOR_ID_ADDR, &vid, sizeof(vid));
		if (r < 0) {
			il_net_master_stop(net);
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
static int il_ecat_net_remove_all_mapped_registers(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	// Check the Monitoring/Disturbance version
	uint32_t mon_dist_version = 0;
	r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
	if (r < 0) {
		// Old monitoring implementation
		r = il_ecat_net_remove_all_mapped_registers_v1(net);
	}
	else {
		r = il_ecat_net_remove_all_mapped_registers_v2(net);
	}

	return r;
}

static int il_ecat_net_remove_all_mapped_registers_v1(il_net_t *net)
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

static int il_ecat_net_remove_all_mapped_registers_v2(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t remove_val = 0;
	r = il_net__write(&this->net, 1, 0, 0x00E3, &remove_val, sizeof(uint16_t), 1, 0);
	if (r < 0) {

	}

	net->monitoring_number_mapped_registers = 0;
	return r;
}

/**
* Monitoring set mapped registers
*/
static int il_ecat_net_set_mapped_register(il_net_t *net, int channel, uint32_t address,
											uint8_t subnode, il_reg_dtype_t dtype,
											uint8_t size)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	// Check the Monitoring/Disturbance version
	uint32_t mon_dist_version = 0;
	r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
	if (r < 0) {
		// Old monitoring implementation
		r = il_ecat_net_set_mapped_register_v1(net, channel, address, dtype);
	}
	else {
		il_ecat_net_set_mapped_register_v2(net, channel,
											address, subnode,
											dtype, size);
	}

	return r;
}

static int il_ecat_net_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address, il_reg_dtype_t dtype)
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

static int il_ecat_net_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
											uint8_t subnode, il_reg_dtype_t dtype,
											uint8_t size)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

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
	r = il_net__write(&this->net, 1, 0, il_net_ecat_monitoring_mapping_registers[net->monitoring_number_mapped_registers], &entire_frame, 4, 1, 0);

	// Update number of mapped registers & monitoring bytes per block
	net->monitoring_number_mapped_registers = net->monitoring_number_mapped_registers + 1;
	r = il_net__write(&this->net, 1, 0, 0x00E3, &net->monitoring_number_mapped_registers, 2, 1, 0);
	r = il_net__read(&this->net, 1, 0, 0x00E4, &net->monitoring_bytes_per_block, sizeof(net->monitoring_bytes_per_block));
	if (r < 0) {

	}

	return r;
}

/**
* Disturbance remove all mapped registers
*/
static int il_ecat_net_disturbance_remove_all_mapped_registers(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	// Check the Monitoring/Disturbance version
	uint32_t mon_dist_version = 0;
	r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
	if (r < 0) {
		// Old monitoring implementation
		r = il_ecat_net_disturbance_remove_all_mapped_registers_v1(net);
	}
	else {
		r = il_ecat_net_disturbance_remove_all_mapped_registers_v2(net);
	}

	return r;
}

static int il_ecat_net_disturbance_remove_all_mapped_registers_v1(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t remove_val = 1;
	r = il_net__write(&this->net, 1, 0, 0x00E7, &remove_val, 2, 1, 0);
	if (r < 0) {

	}

	net->disturbance_number_mapped_registers = 0;
	return r;
}

static int il_ecat_net_disturbance_remove_all_mapped_registers_v2(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

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
static int il_ecat_net_disturbance_set_mapped_register(il_net_t *net, int channel, uint32_t address,
														uint8_t subnode, il_reg_dtype_t dtype,
														uint8_t size)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	// Check the Monitoring/Disturbance version
	uint32_t mon_dist_version = 0;
	r = il_net__read(&this->net, 1, 0, 0x00BA, &mon_dist_version, sizeof(uint32_t));
	if (r < 0) {
		// Old monitoring implementation
		il_ecat_net_disturbance_set_mapped_register_v1(net, channel, address, subnode, dtype, size);
	}
	else {
		il_ecat_net_disturbance_set_mapped_register_v2(net, channel, address, subnode, dtype, size);
	}

	return r;
}

static int il_ecat_net_disturbance_set_mapped_register_v1(il_net_t *net, int channel, uint32_t address,
															uint8_t subnode, il_reg_dtype_t dtype,
															uint8_t size)
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

static int il_ecat_net_disturbance_set_mapped_register_v2(il_net_t *net, int channel, uint32_t address,
														uint8_t subnode, il_reg_dtype_t dtype,
														uint8_t size)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

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
	r = il_net__write(&this->net, 1, 0, il_net_ecat_disturbance_mapping_registers[net->disturbance_number_mapped_registers], &entire_frame, 4, 1, 0);

	// Update number of mapped registers
	net->disturbance_number_mapped_registers = net->disturbance_number_mapped_registers + 1;
	r = il_net__write(&this->net, 1, 0, 0x0E8, &net->disturbance_number_mapped_registers, 2, 1, 0);

	return r;
}


/**
* Monitoring enable
*/


static int il_ecat_net_enable_disturbance(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

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

static int il_ecat_net_disable_disturbance(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

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


static int il_ecat_net_disable_monitoring(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t disable_monitoring_val = 0;
	r = il_net__write(&this->net, 1, 0, 0x00C0, &disable_monitoring_val, 2, 1, 0);
	if (r < 0) {

	}

	return r;
}

static int il_ecat_net_enable_monitoring(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint16_t enable_monitoring_val = 1;
	r = il_net__write(&this->net, 1, 0, 0x00C0, &enable_monitoring_val, 2, 1, 0);
	if (r < 0) {

	}
	return r;
}

static int il_ecat_net_monitoring_remove_data(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);
	uint16_t remove_data_val = 1;

	r = il_net__write(&this->net, 1, 0, 0x0E0, &remove_data_val, 2, 1, 0);
	if (r < 0) {

	}
	return r;
}

static int il_ecat_net_disturbance_remove_data(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);
	uint16_t remove_data_val = 1;

	r = il_net__write(&this->net, 1, 0, 0x0E1, &remove_data_val, 2, 1, 0);
	if (r < 0) {

	}
	return r;
}


static int il_ecat_net_read_monitoring_data(il_net_t *net)
{
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	uint64_t vid;


	osal_mutex_lock(this->net.lock);
	r = il_ecat_net__read_monitoring(&this->net, 1, 0, 0x00B2, &vid, sizeof(vid));
	if (r < 0) {

	}
	osal_mutex_unlock(this->net.lock);
	return r;
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

	osal_mutex_lock(this->net.lock);
	if (this->use_eoe_comms)
	{
		int num_retries = 0;
		while (num_retries < NUMBER_OP_RETRIES_DEF)
		{
			r = net_send(this, subnode, (uint16_t)address, NULL, 0, 0, net);
			if (r < 0) {
				goto unlock;
			}

			uint16_t *monitoring_raw_data = NULL;
			r = net_recv(this, subnode, (uint16_t)address, buf, sz, monitoring_raw_data, net);
			if (r == IL_ETIMEDOUT || r == IL_EWRONGREG || r == IL_EFAIL)
			{
				++num_retries;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// SDOs
		int addr;
		if (subnode >= 1) {
			addr = address + (0x2000 + ((subnode - 1) * 0x800));
		}
		else
		{
			addr = address + 0x5800;
		}
		r = il_ecat_net_SDO_read(this, id, addr, 0x00, sz, buf);
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

static int il_ecat_net__read_monitoring(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
	void *buf, size_t sz)
{
	il_ecat_net_t *this = to_ecat_net(net);
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
			r = il_ecat_net_recv_monitoring(this, subnode, (uint16_t)address, buf, sz, monitoring_raw_data, net, num_bytes);
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

static int il_ecat_net__write(il_net_t *net, uint16_t id, uint8_t subnode, uint32_t address,
	const void *buf, size_t sz, int confirmed, uint16_t extended)
{
	il_ecat_net_t *this = to_ecat_net(net);

	int r;

	(void)id;
	(void)confirmed;

	osal_mutex_lock(this->net.lock);

	if (this->use_eoe_comms)
	{
		r = net_send(this, subnode, (uint16_t)address, buf, sz, extended, net);
		if (r < 0)
			goto unlock;

		int num_retries = 0;
		while (num_retries < NUMBER_OP_RETRIES_DEF)
		{
			r = net_recv(this, subnode, (uint16_t)address, NULL, 0, NULL, NULL);
			if (r == IL_ETIMEDOUT || r == IL_EWRONGREG)
			{
				++num_retries;
			}
			else
			{
				break;
			}
		}
	}
	else
	{
		// SDOs
		int addr;
		if (subnode >= 1) {
			addr = address + (0x2000 + ((subnode - 1) * 0x800));
		}
		else
		{
			addr = address + 0x5800;
		}
		r = il_ecat_net_SDO_write(this, id, addr, 0x00, sz, buf);
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
	while (num_retries < NUMBER_OP_RETRIES_DEF)
	{
		r = net_recv(this, subnode, (uint16_t)address, NULL, 0, NULL, NULL);
		if (r == IL_ETIMEDOUT || r == IL_EWRONGREG)
		{
			++num_retries;
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

int il_ecat_net_SDO_read(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, int size, void *buf) {
	int wkc = 0;
	int r = 0;
	int num_retries = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	while (num_retries < NUMBER_OP_RETRIES_DEF)
	{
		wkc = ec_SDOread(slave, index, subindex, FALSE, &size, buf, EC_TIMEOUTRXM);
		if (wkc <= 0)
		{
			++num_retries;
			Sleep(100);
		}
		else
		{
			break;
		}
	}
	if (wkc <= 0)
	{
		r = IL_EFAIL;
	}
	return r;
}

int il_ecat_net_SDO_read_complete_access(il_net_t *net, uint8_t slave, uint16_t index, int size, void *buf) {
	int wkc = 0;
	int r = 0;
	int num_retries = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	while (num_retries < NUMBER_OP_RETRIES_DEF)
	{
		wkc = ec_SDOread(slave, index, 1, TRUE, &size, buf, EC_TIMEOUTRXM);
		if (wkc <= 0)
		{
			++num_retries;
			Sleep(100);
		}
		else
		{
			break;
		}
	}
	if (wkc <= 0)
	{
		r = IL_EFAIL;
	}
	return r;
}

int il_ecat_net_SDO_write(il_net_t *net, uint8_t slave, uint16_t index, uint8_t subindex, int size, void *buf) {
	int wkc = 0;
	int r = 0;
	int num_retries = 0;
	il_ecat_net_t *this = to_ecat_net(net);

	while (num_retries < NUMBER_OP_RETRIES_DEF)
	{
		wkc += ec_SDOwrite(slave, index, subindex, FALSE, size, buf, EC_TIMEOUTRXM);
		if (wkc <= 0)
		{
			++num_retries;
			Sleep(100);
		}
		else
		{
			break;
		}
	}
	if (wkc <= 0)
	{
		r = IL_EFAIL;
	}
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
				return ilerr__ecat(error);
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

			if (error < 0)
				return ilerr__ecat(error);
		}
		finished = 1;
	}

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

	int r = osal_cond_wait(this->mailbox_check, this->lock_mailbox, this->recv_timeout/1000);

	if (r == -2){
		return IL_ETIMEDOUT;
	}
	if (r < 0) {
		return IL_EFAIL;
	}

	int s32SzRead = 1024;
	/* Obtain the frame received */
	memcpy(frame, (uint8_t*)&(this->frame_received), 1024);
	/* process frame: validate CRC, address, ACK */
	crc = *(uint16_t *)&frame[6];
	uint16_t crc_res = crc_calc_ecat((uint16_t *)frame, 6);
	if (crc_res != crc) {
		ilerr__set("Communications error (CRC mismatch)");
		return IL_EWRONGCRC;
	}

	/* TODO: Check subnode */

	/* Check ACK */
	hdr_l = *(uint16_t *)&frame[ECAT_MCB_HDR_L_POS];
	int cmd = (hdr_l & ECAT_MCB_CMD_MSK) >> ECAT_MCB_CMD_POS;
	if (cmd != ECAT_MCB_CMD_ACK) {
		uint32_t err;

		err = __swap_be_32(*(uint32_t *)&frame[ECAT_MCB_DATA_POS]);

		ilerr__set("Communications error (NACK -> %08x)", err);
		return IL_ENACK;
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
			memcpy(net->monitoring_raw_data, (uint8_t*)&(this->frame_received[14]), size);

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
			memcpy(net->extended_buff, (char*)&(this->frame_received[14]), size);
		}
	}
	else {
		memcpy(buf, &(frame[ECAT_MCB_DATA_POS]), sz);
	}

	return 0;
}

 static int il_ecat_net_recv_monitoring(il_ecat_net_t *this, uint8_t subnode, uint16_t address, uint8_t *buf,
 	size_t sz, uint16_t *monitoring_raw_data, il_net_t *net, int num_bytes)
 {
 	int finished = 0;
 	size_t pending_sz = sz;

 	uint16_t frame[1024];
 	size_t block_sz = 0;
 	uint16_t crc, hdr_l;
 	uint8_t *pBuf = (uint8_t*)&frame;
 	uint8_t extended_bit = 0;

	int r = osal_cond_wait(this->mailbox_check, this->lock_mailbox, this->recv_timeout/1000);

	if (r == -2) {
		return IL_ETIMEDOUT;
	}
	if (r < 0) {
		return IL_EFAIL;
	}

 	/* Obtain the frame received */
 	memcpy(frame, (uint8_t*)&(this->frame_received), 1024);

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
 	if (extended_bit == 1)
	{
 		/* Check if we are reading monitoring data */
 		if (address == 0x00B2)
		{
 			/* Monitoring */
 			/* Read size of data */
 			memcpy(buf, &(frame[ECAT_MCB_DATA_POS]), 2);
 			uint16_t size = *(uint16_t*)buf;
			if (num_bytes < size)
			{
				size = num_bytes;
			}
			uint32_t start_addr = net->monitoring_data_size;
			memcpy((uint16_t*)&net->monitoring_raw_data[start_addr], (uint16_t*)&(this->frame_received[14]), size);
			net->monitoring_data_size += (uint32_t)size;
 		}
 		else
		{
 			memcpy(buf, &(frame[ECAT_MCB_DATA_POS]), 2);
 			uint16_t size = *(uint16_t*)buf;
 			memcpy(net->extended_buff, (char*)&(this->frame_received[14]), size);
 		}
 	}
 	else
	{
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

	int i = ecx_EOEsend(context, slave_number, 0, ptBuf->tot_len, ptBuf->payload, EC_TIMEOUTTXM);

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

		pbuf_free(pBuf);
		pBuf = NULL;
	}
}

static void LWIP_UdpReceiveData(void* net, struct udp_pcb* ptUdpPcb, struct pbuf* ptBuf,
	const ip_addr_t* ptAddr, u16_t u16Port)
{
	il_ecat_net_t *this = to_ecat_net((il_net_t*)net);
	memcpy(this->frame_received, ptBuf->payload, ptBuf->len);
	osal_cond_signal(this->mailbox_check);
	pbuf_free(ptBuf);
}

OSAL_THREAD_FUNC configure_udp(il_net_t *net)
{
	int wkc = 0;
	ec_mbxbuft MbxIn;
	ec_mbxheadert * MbxHdr = (ec_mbxheadert *)MbxIn;

	IP4_ADDR(&dstaddr, 192, 168, 2, 22);

	ip4_addr_t tIpAddr, tNetmask, tGwIpAddr;

	/* Initilialize the LwIP stack without RTOS */
	if (!is_lwip_on){
		lwip_init();
		is_lwip_on = TRUE;
	}
	
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
	udp_recv(ptUdpPcb, LWIP_UdpReceiveData, net);
	/* UDP connect */
	error = udp_connect(ptUdpPcb, &tIpAddr, 1061);

	//osal_usleep(100000);
}

OSAL_THREAD_FUNC mailbox_reader(il_ecat_net_t *this)
{
	int s32SzRead = 1024;
	int wkc;
	while (!(this->stop_mailbox))
	{
		if (context != NULL){
			wkc = ecx_EOErecv(context, this->slave, 0, &s32SzRead, rxbuf, EC_TIMEOUTRXM);
			LWIP_ProcessTimeouts();
		}
	}
}

/** registered EoE hook */
int eoe_hook(ecx_contextt * context, uint16 slave, void * eoembx)
{
	int wkc;

	/*
	* 	Pass received Mbx data to EoE recevive fragment function that
	* 	that will start/continue fill an Ethernet frame buffer
	*/

	size_of_rx = sizeof(rxbuf);
	wkc = ecx_EOEreadfragment(eoembx,
		&rxfragmentno,
		&rxframesize,
		&rxframeoffset,
		&rxframeno,
		&size_of_rx,
		rxbuf);
	int r = rxframesize;
	
	if (wkc > 0) {
		LWIP_EthernetifInp((uint16_t*)rxbuf, size_of_rx);
	}

	/* No point in returning as unhandled */
	return 0;
}

void init_eoe(il_net_t *net, ecx_contextt * context, uint16_t slave)
{
	log_debug("Init EoE");
	/* Set the HOOK */
	il_ecat_net_t *this = to_ecat_net(net);

	ecx_EOEdefinehook(context, eoe_hook);

	eoe_param_t ipsettings, re_ipsettings;
	memset(&ipsettings, 0, sizeof(ipsettings));
	memset(&re_ipsettings, 0, sizeof(re_ipsettings));

	log_debug("IP configuration");
	ipsettings.ip_set = 1;
	ipsettings.subnet_set = 1;
	ipsettings.default_gateway_set = 1;
	ipsettings.mac_set = 1;

	EOE_IP4_ADDR_TO_U32(&ipsettings.ip, 192, 168, 2, 22);
	EOE_IP4_ADDR_TO_U32(&ipsettings.subnet, 255, 255, 255, 0);
	EOE_IP4_ADDR_TO_U32(&ipsettings.default_gateway, 192, 168, 2, 1);
	ipsettings.mac.addr[0] = 0;
	ipsettings.mac.addr[1] = 1;
	ipsettings.mac.addr[2] = 2;
	ipsettings.mac.addr[3] = 3;
	ipsettings.mac.addr[4] = 4;
	ipsettings.mac.addr[5] = 5;

	log_debug("IP configured");

	/* Send a set IP request */
	ecx_EOEsetIp(context, slave, 0, &ipsettings, EC_TIMEOUTRXM);

	/* Send a get IP request, should return the expected IP back */
	ecx_EOEgetIp(context, slave, 0, &re_ipsettings, EC_TIMEOUTRXM);

	/* Configure UDP */
	configure_udp(net);

	/* Create a asyncronous EoE reader */
	this->mailbox_check = osal_cond_create();
	this->lock_mailbox = osal_mutex_create();
	this->stop_mailbox = 0;
	osal_thread_create(&mailbox_reader_thread, 128000, &mailbox_reader, this);
}

int *il_ecat_net_set_if_params(il_net_t *net, char *ifname, char *if_address_ip)
{
	il_ecat_net_t *this = to_ecat_net(net);
	this->ifname = ifname;
	this->if_address_ip = if_address_ip;
}

int *il_ecat_net_master_startup(il_net_t *net, char *ifname, uint16_t slave, uint8_t use_eoe_comms)
{

	int i, oloop, iloop, chk;
	
	slave_number = slave;

	log_debug("Starting EtherCAT Master");
	/* Initialise SOEM, bind socket to ifname */
	if (ec_init(ifname)) {
		log_debug("ec_init on %s succeeded.", ifname);
		/* Find and auto-config slaves */
		if (ec_config_init(FALSE) > 0) {
			log_debug("%d slaves found and configured.", ec_slavecount);
			context = &ecx_context;
			if (slave <= ec_slavecount) {
				log_debug("Slaves mapped, state to PRE_OP.");
				/* Wait for all slaves to reach SAFE_OP state */
				ec_statecheck(slave, EC_STATE_PRE_OP, EC_TIMEOUTSTATE * 4);

				ec_slave[slave].state = EC_STATE_PRE_OP;

				/* Request OP state for all slaves */
				ec_writestate(slave);
				chk = 200;

				/* Wait for all slaves to reach OP state */
				do{
					ec_statecheck(slave, EC_STATE_PRE_OP, 50000);
				} while (chk-- && (ec_slave[slave].state != EC_STATE_PRE_OP));
				if (ec_slave[slave].state == EC_STATE_PRE_OP) {
					log_info("Pre-Operational state reached for all slaves");
				} else {
					log_warn("Not all slaves reached operational state.");
					ec_readstate();
					if (ec_slave[slave].state != EC_STATE_PRE_OP) {
						log_error("Not all slaves are in PRE-OP");
						return -1;
					}
				}

				if (ec_slavecount > 0 && use_eoe_comms) {
					init_eoe(net, context, slave);
				}
			} else {
				log_error("Slave number not found.");
				return -1;
			}
		} else {
			log_info("No slaves found!");
		}
	} else {
		log_warn("No socket connection on %s. Excecute as root", ifname);
	}

	return ec_slavecount;
}

int *il_ecat_net_test(il_net_t *net) {
	int r = 0;
	il_ecat_net_t *this = to_ecat_net(net);
	while(true) {
		uint16_t sw;
		r = il_net__read(&this->net, 1, 1, STATUSWORD_ADDRESS, &sw, sizeof(sw));
		if (r < 0) {
			log_warn("FAIL READING SW!");
		}
		else {
			log_debug("SW -> %i", sw);
		}
		Sleep(1000);
	}
	return 0;
}

int *il_ecat_net_num_slaves_get(char *ifname)
{
	int i, oloop, iloop, chk;

	log_debug("Starting EtherCAT Master");
	/* Initialise SOEM, bind socket to ifname */
	if (ec_init(ifname)) {
		log_debug("ec_init on %s succeeded.", ifname);
		/* Find and auto-config slaves */
		if (ec_config_init(FALSE) > 0) {
			log_debug("%d slaves found.", ec_slavecount);
		} else {
			log_warn("No slaves found!");
		}
	} else {
		log_warn("No socket connection on %s. Excecute as root", ifname);
	}
	ec_close();
	return ec_slavecount;
}

int *il_ecat_net_change_state(uint16_t slave, ec_state state)
{
	ec_slave[slave].state = state;
	ec_writestate(slave);

	if (ec_statecheck(slave, state, EC_TIMEOUTSTATE) != state) {
		return UP_STATEMACHINE_ERROR;
	}
	return UP_NOERROR;
}

static int *il_ecat_net_master_stop(il_net_t *net)
{

	log_debug("Close listener ecat");
	il_ecat_net_t *this = to_ecat_net(net);
	this->stop_mailbox = 1;
	il_ecat_mon_stop(this);

	// Wait for mailbox stop
	Sleep(1000);
	if (this->use_eoe_comms == 1){
		osal_mutex_destroy(this->lock_mailbox);
		osal_cond_destroy(this->mailbox_check);
	}

	log_debug("Disconnecting interface");
	ec_slavecount = 0;
	/* Disconnecting and removing udp interface */
	if (ptUdpPcb != NULL) {
		udp_disconnect(ptUdpPcb);
		udp_recv(ptUdpPcb, NULL, (void*)NULL);
		udp_remove(ptUdpPcb);
	}

	/* Remove the network interface */
	log_debug("Removing network interface");
	netif_set_down(&tNetif);
	netif_remove(&tNetif);
	ec_mbxempty(this->slave, 100000);
	context->EOEhook = NULL;

	log_debug("Setting state to INIT");
	if (il_ecat_net_change_state(this->slave, EC_STATE_INIT) != UP_NOERROR) {
		log_warn("Slave %d cannot enter into state INIT.", 0);
	}

	log_debug("Closing EtherCAT interface");
	/* Close EtherCAT interface */
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


/**
 * Update Firmware using FoE
*/
static int *il_ecat_net_update_firmware(il_net_t **net, char *ifname, uint16_t slave, char *filename, bool is_summit)
{
	log_debug(filename);
	log_debug("Starting firmware update example");
	int r = 0;
	/* initialise SOEM, bind socket to ifname */
	if (ec_init(ifname)) {
		log_debug("ec_init on %s succeeded.", ifname);
		/* find and auto-config slaves */

		if (ec_config_init(FALSE) > 0) {
			log_debug("%d slaves found and configured.", ec_slavecount);

			log_debug("Request init state for slave %d", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_INIT) != UP_NOERROR) {
				log_error("Slave %d cannot enter into state INIT.", slave);
				return UP_STATEMACHINE_ERROR;
			}

			log_debug("Slave %d state to INIT.", slave);

			log_debug("Request pre-op state for slave %d", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_PRE_OP) != UP_NOERROR) {
				log_warn("Slave %d cannot enter into state PRE-OP.", slave);
				log_debug("Application not detected. Trying Bootloader process..");
			} else {
				if (!is_summit) {
					log_debug("Writing COCO FORCE BOOT password through SDO");
					uint32 u32val = 0x424F4F54;
					if (ec_SDOwrite(slave, 0x5EDE, 0x00, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTRXM) <= 0) {
						log_debug("SDO write error");
						log_debug("Retrying...");
						if (ec_SDOwrite(slave, 0x5EDE, 0x00, FALSE, sizeof(u32val), &u32val, EC_TIMEOUTRXM) <= 0)  {
							log_error("Force Boot error");
							return UP_FORCE_BOOT_ERROR;
						}
					}
				}

				log_debug("Request init state for slave %d", slave);
				if (il_ecat_net_change_state(slave, EC_STATE_INIT) != UP_NOERROR) {
					log_error("Slave %d cannot enter into state INIT.", slave);
					return UP_STATEMACHINE_ERROR;
				}
				log_debug("Slave %d state to INIT.", slave);

				log_debug("Request BOOT state for slave %d", slave);
				if (il_ecat_net_change_state(slave, EC_STATE_BOOT) == UP_NOERROR && !is_summit) {
					log_debug("Slave %d entered into state BOOT.", slave);
					log_error("Force COCO Boot not applied correctly.");
					return UP_STATEMACHINE_ERROR;
				}
				log_debug("As expected, Slave %d cannot enter into state BOOT the first time.", slave);

				ec_close();
				ec_init(ifname);
				ec_config_init(FALSE);
			}

			log_debug("Request init state for slave %d", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_INIT) != UP_NOERROR) {
				log_error("Slave %d cannot enter into state INIT.", slave);
				return UP_STATEMACHINE_ERROR;
			}
			log_debug("Slave %d state to INIT.", slave);

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

			log_debug(" SM0 A:%4.4x L:%4d F:%8.8x", ec_slave[slave].SM[0].StartAddr, ec_slave[slave].SM[0].SMlength,
				(int)ec_slave[slave].SM[0].SMflags);
			log_debug(" SM1 A:%4.4x L:%4d F:%8.8x", ec_slave[slave].SM[1].StartAddr, ec_slave[slave].SM[1].SMlength,
				(int)ec_slave[slave].SM[1].SMflags);
			/* program SM0 mailbox in for slave */
			ec_FPWR(ec_slave[slave].configadr, ECT_REG_SM0, sizeof(ec_smt), &ec_slave[slave].SM[0], EC_TIMEOUTRET);
			/* program SM1 mailbox out for slave */
			ec_FPWR(ec_slave[slave].configadr, ECT_REG_SM1, sizeof(ec_smt), &ec_slave[slave].SM[1], EC_TIMEOUTRET);

			log_debug("Request BOOT state for slave %d", slave);
			if (il_ecat_net_change_state(slave, EC_STATE_BOOT) != UP_NOERROR) {
				log_error("Slave %d cannot enter into state BOOT.", slave);
				return UP_STATEMACHINE_ERROR;
			}
			log_debug("Slave %d state to BOOT.", slave);

			if (ec_eeprom2pdi(slave) <= 0) {
				return UP_EEPROM_PDI_ERROR;
			}
			log_debug("Slave %d EEPROM set to PDI.", slave);

			if (input_bin(filename, &filesize)){
				// Get filename of absolute path
				int len = strlen(filename);
				while (len >= 0) {
					if (filename[len] == '/' || filename[len] == '\\') {
						break;
					}
					--len;
				}
				char* file_id = &filename[len + 1];

				log_debug("File read OK, %d bytes.", filesize);
				log_debug("FoE write....");
				r = ec_FOEwrite(slave, file_id, 0x70636675, filesize, &filebuffer, MAX_FOE_TIMEOUT);
				log_debug("FOE write result %d.", r);
				if (r > 0) {
					log_debug("Request init state for slave %d", slave);
					if (!is_summit) {
						ec_slave[slave].state = EC_STATE_INIT;
						ec_writestate(slave);

						log_debug("Wait for drive to reset...");
						Sleep(4000);
					} else {
						ec_slave[slave].state = EC_STATE_INIT;
						ec_writestate(slave);

						log_debug("Wait for drive to reset...");
						Sleep(60000);
					}
					log_info("FOE Process finished succesfully!!!.");
				} else  {
					if (r == 0) {
						r = SOEM_EC_ERR_TYPE_SDO_ERROR;
					}
					log_warn("Error during FoE process...");
				}

			} else {
				log_error("File not read OK.");
				return UP_EEPROM_FILE_ERROR;
			}

		} else {
			log_error("No slaves found!");
			return UP_NOT_FOUND_ERROR;
		}

	} else {
		log_error("No socket connection on %s. Execute as root",ifname);
		return UP_NO_SOCKET;
	}
	log_debug("End firmware update, close socket");
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
         log_warn("Invalid Intel Hex format.");
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
               log_warn("Invalid checksum.");
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
            log_debug(".");
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
		log_debug("ec_init on %s succeeded.",ifname);

		w = 0x0000;
		wkc_eeprom = ec_BRD(0x0000, ECT_REG_TYPE, sizeof(w), &w, EC_TIMEOUTSAFE);      /* detect number of slaves */
		if (wkc_eeprom > 0)
		{
			ec_slavecount = wkc_eeprom;

			log_debug("%d slaves found.",ec_slavecount);
			if((ec_slavecount >= slave) && (slave > 0))
			{
				if ((mode == MODE_INFO) || (mode == MODE_READBIN) || (mode == MODE_READINTEL))
				{
					tstart = osal_current_time();
					eeprom_read(slave, 0x0000, MINBUF); // read first 128 bytes

					wbuf = (uint16 *)&ebuf[0];
					log_debug("Slave %d data", slave);
					log_debug(" PDI Control      : %4.4X",*(wbuf + 0x00));
					log_debug(" PDI Config       : %4.4X",*(wbuf + 0x01));
					log_debug(" Config Alias     : %4.4X",*(wbuf + 0x04));
					log_debug(" Checksum         : %4.4X",*(wbuf + 0x07));
					log_debug("   calculated     : %4.4X",SIIcrc(&ebuf[0]));
					log_debug(" Vendor ID        : %8.8X",*(uint32 *)(wbuf + 0x08));
					log_debug(" Product Code     : %8.8X",*(uint32 *)(wbuf + 0x0A));
					log_debug(" Revision Number  : %8.8X",*(uint32 *)(wbuf + 0x0C));
					log_debug(" Serial Number    : %8.8X",*(uint32 *)(wbuf + 0x0E));
					log_debug(" Mailbox Protocol : %4.4X",*(wbuf + 0x1C));
					esize = (*(wbuf + 0x3E) + 1) * 128;
					if (esize > MAXBUF) esize = MAXBUF;
					log_debug(" Size             : %4.4X = %d bytes",*(wbuf + 0x3E), esize);
					log_debug(" Version          : %4.4X",*(wbuf + 0x3F));
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

					log_debug("Total EEPROM read time :%ldms", (tdif.usec+(tdif.sec*1000000L)) / 1000);
				}
				if ((mode == MODE_WRITEBIN) || (mode == MODE_WRITEINTEL))
				{
					estart = 0;
					if (mode == MODE_WRITEINTEL) rc = input_intelhex(fname, &estart, &esize);
					if (mode == MODE_WRITEBIN)   rc = input_bin_eeprom(fname, &esize);

					if (rc > 0)
					{
						wbuf = (uint16 *)&ebuf[0];
						log_debug("Slave %d", slave);
						log_debug(" Vendor ID        : %8.8X", *(uint32 *)(wbuf + 0x08));
						log_debug(" Product Code     : %8.8X", *(uint32 *)(wbuf + 0x0A));
						log_debug(" Revision Number  : %8.8X", *(uint32 *)(wbuf + 0x0C));
						log_debug(" Serial Number    : %8.8X", *(uint32 *)(wbuf + 0x0E));

						log_debug("Busy");
						fflush(stdout);
						tstart = osal_current_time();
						eeprom_write(slave, estart, esize);
						tend = osal_current_time();
						osal_time_diff(&tstart, &tend, &tdif);

						log_debug("Total EEPROM write time :%ldms", (tdif.usec + (tdif.sec * 1000000L)) / 1000);
					}
					else
					{
						log_error("Error reading file, abort.");
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
							log_debug("Alias %4.4X written successfully to slave %d", alias, slave);
						}
						else
						{
							log_debug("Alias not written");
						}
					}
					else
					{
						log_debug("Could not read slave EEPROM");
					}
				}
			}
			else
			{
				log_debug("Slave number outside range.");
			}
		}
		else
		{
			log_error("No slaves found!");
			r = -1;
		}
		log_debug("End, close socket");
		/* stop SOEM, close socket */
		ec_close();
	}
	else
	{
		log_warn("No socket connection on %s. Excecute as root",ifname);
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
	u16val = 0x1600;
	retval += ec_SDOwrite(slave, 0x1c12, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTRXM);
	u8val = 1;
	retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);

	u8val = 0;
	retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);
	u16val = 0x1a00;
	retval += ec_SDOwrite(slave, 0x1c13, 0x01, FALSE, sizeof(u16val), &u16val, EC_TIMEOUTRXM);
	u8val = 1;
	retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(u8val), &u8val, EC_TIMEOUTRXM);


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

	while (EcatError) log_debug("%s", ec_elist2string());

	log_debug("Everest slave %d set, retval = %d", slave, retval);
	return 1;
}

int *il_ecat_net_force_error(il_net_t **net, char *ifname, char *if_address_ip)
{
	int i, j, oloop, iloop, wkc_count, chk, slc;


   	log_debug("Slave force error");

	/* initialise SOEM, bind socket to ifname */
   	if (ec_init(ifname))
	{
		log_debug("ec_init on %s succeeded.",ifname);
      	/* find and auto-config slaves */

		if ( ec_config_init(FALSE) > 0 )
      	{

			log_debug("%d slaves found and configured.",ec_slavecount);
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
				log_debug("retval = %i", retval);
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

static int process_monitoring_data(il_ecat_net_t *this, il_net_t *net)
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

	log_debug("Data Processed");
	return 0;
}

int il_ecat_set_reconnection_retries(il_net_t *net, uint8_t retries)
{
	il_ecat_net_t *this = to_ecat_net(net);
	this->reconnection_retries = retries;
	return 0;
}

int il_ecat_set_recv_timeout(il_net_t *net, uint32_t timeout)
{
	il_ecat_net_t *this = to_ecat_net(net);
	this->recv_timeout = timeout;
	return 0;
}

int il_ecat_set_status_check_stop(il_net_t *net, int stop)
{
	il_ecat_net_t *this = to_ecat_net(net);
	this->status_check_stop = stop;
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
	._state_set = il_net_base__state_set,
	.mon_stop = il_ecat_mon_stop,
	/* Monitornig */
	.remove_all_mapped_registers = il_ecat_net_remove_all_mapped_registers,
	.set_mapped_register = il_ecat_net_set_mapped_register,
	.enable_monitoring = il_ecat_net_enable_monitoring,
	.disable_monitoring = il_ecat_net_disable_monitoring,

	/* Disturbance */
	.disturbance_remove_all_mapped_registers = il_ecat_net_disturbance_remove_all_mapped_registers,
	.disturbance_set_mapped_register = il_ecat_net_disturbance_set_mapped_register,
	.enable_disturbance = il_ecat_net_enable_disturbance,
	.disable_disturbance = il_ecat_net_disable_disturbance,
	.monitoring_remove_data = il_ecat_net_monitoring_remove_data,
	.disturbance_remove_data = il_ecat_net_disturbance_remove_data,
	.read_monitoring_data = il_ecat_net_read_monitoring_data,
	.recv_monitoring = il_ecat_net_recv_monitoring,
	/* Master EtherCAT */
	.master_startup = il_ecat_net_master_startup,
	.num_slaves_get = il_ecat_net_num_slaves_get,
	.master_stop = il_ecat_net_master_stop,
	.update_firmware = il_ecat_net_update_firmware,
	.eeprom_tool = il_ecat_net_eeprom_tool,

	.force_error = il_ecat_net_force_error,

	.set_if_params = il_ecat_net_set_if_params,

	.set_reconnection_retries = il_ecat_set_reconnection_retries,
	.set_recv_timeout = il_ecat_set_recv_timeout,
	.set_status_check_stop = il_ecat_set_status_check_stop,
	.net_test = il_ecat_net_test,
	.SDO_read = il_ecat_net_SDO_read,
	.SDO_read_complete_access = il_ecat_net_SDO_read_complete_access,
	.SDO_write = il_ecat_net_SDO_write
};

/** MCB network device monitor operations. */
const il_net_dev_mon_ops_t il_ecat_net_dev_mon_ops = {
	.create = il_ecat_net_dev_mon_create,
	.destroy = il_ecat_net_dev_mon_destroy,
	.start = il_ecat_net_dev_mon_start,
	.stop = il_ecat_net_dev_mon_stop
};