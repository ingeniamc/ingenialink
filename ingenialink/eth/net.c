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

#include "net.h"
#include "frame.h"

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <windows.h>

#include "ingenialink/err.h"
#include "ingenialink/base/net.h"

/*******************************************************************************
 * Private
 ******************************************************************************/
static int not_supported(void)
{
	ilerr__set("Functionality not supported");

	return IL_ENOTSUP;
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
static void init_crcccitt_tab_eth( void ) {

	uint16_t i;
	uint16_t j;
	uint16_t crc;
	uint16_t c;

	for (i=0; i<256; i++) {
		crc = 0;
		c   = i << 8;
		for (j=0; j<8; j++) {
			if ( (crc ^ c) & 0x8000 ) crc = ( crc << 1 ) ^ 0x1021;
			else crc =   crc << 1;
			c = c << 1;
		}
		crc_tabccitt_eth[i] = crc;
	}
	crc_tabccitt_init_eth = true;
}

static uint16_t update_crc_ccitt_eth( uint16_t crc, unsigned char c ) {

	if ( ! crc_tabccitt_init_eth ) init_crcccitt_tab_eth();
	return (crc << 8) ^ crc_tabccitt_eth[ ((crc >> 8) ^ (uint16_t) c) & 0x00FF ];

} 

static uint16_t crc_calc_eth(const uint16_t *buf, uint16_t u16Sz)
{
	
	uint16_t crc = 0x0000;
    uint8_t* pu8In = (uint8_t*) buf;
    
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
 *	MCB Network.
 * @param [in] frame
 *	IngeniaLink frame.
 */
static void process_statusword(il_mcb_net_t *this, uint8_t subnode, uint16_t data)
{

}

/**
 * Listener thread.
 *
 * @param [in] args
 *	MCB Network (il_mcb_net_t *).
 */
int listener_mcb(void *args)
{
	return not_supported();
}
