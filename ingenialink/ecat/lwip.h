/**
 * @file lwip.h
 * @brief This file contains the lwip application functions
 *
 * @author  Firmware department.
 * @copyright Ingenia Motion Control (c) 2019. All rights reserved.
 */

#ifndef LWIP_H
#define LWIP_H

// #include "utils.h"
#include <stdint.h>
#include <stdbool.h>
/**
 * Lwip initialization function.
 */
void
LWIP_Init(void);

/**
 * Virtual network interface (EoE), input function.
 *
 * @param[in] pData
 *  Pointer to received data.
 * @param[in] u16SizeBy
 *  Number of bytes received.
 */
void
LWIP_EthernetifIntput(void* pData, uint16_t u16SizeBy);

/**
 * Set an ip address to the network interface.
 *
 * @param[in] pu8IpAddr
 *  New Ip address.
 */
void
LWIP_SetIpAddress(uint8_t* pu8IpAddr);

/**
 * Set submask to network interface.
 *
 * @param[in] pu8Netmask
 *  New submask.
 */
void
LWIP_SetNetmask(uint8_t* pu8Netmask);

/**
 * Set the gateway Ip address to the network interface.
 *
 * @param[in] pu8GwAddr
 *  New gateway Ip address.
 */
void
LWIP_SetGwAddress(uint8_t* pu8GwAddr);

/**
 * Handle lwip timeouts.
 */
void
LWIP_ProcessTimeouts(void);

#endif /* LWIP_H */
