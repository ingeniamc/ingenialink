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

#ifndef MCB_FRAME_H_
#define MCB_FRAME_H_

#include "ingenialink/mcb/frame.h"

/** MCB frame CRC polynomial (16-CCITT). */
#define MCB_CRC_POLY	0x1021

/** MCB frame size (words). */
#define MCB_FRAME_SZ	7

/** MCB Header */
/** MCB frame header high word position. */
#define MCB_HDR_H_POS	0
/** MCB frame header low word position. */
#define MCB_HDR_L_POS	1
/** MCB frame header high size (words). */
#define MCB_HDR_H_SZ	1
/** MCB frame header high size (words). */
#define MCB_HDR_L_SZ	1
/** MCB frame header size (words). */
#define MCB_HDR_SZ	2

/** MCB Config Data */
/** MCB frame config data first word position. */
#define MCB_CFG_DATA_POS	2
/** MCB frame config data size (words). */
#define MCB_CFG_DATA_SZ 4

/* MCB CRC */
/** MCB frame crc first word position. */
#define MCB_CRC_POS 6
/** MCB frame CRC size (words). */
#define MCB_CRC_SZ 1

/** MCB COCO Subnode value */
#define MCB_SUBNODE_COCO    0
/** MCB MOCO Subnode value */
#define MCB_SUBNODE_MOCO    2   

/** MCB default node */
#define MCB_NODE_DFLT   0x00A


/** MCB frame header, pending bit */
#define MCB_PENDING	1
/** MCB frame header, pending bit position */
#define MCB_PENDING_POS	0
/** MCB frame header, pending bit mask */
#define MCB_PENDING_MSK	0x0001

/** MCB frame header command, request info (master). */
#define MCB_CMD_INFO	0
/** MCB frame header command, read access (master). */
#define MCB_CMD_READ	1
/** MCB frame header command, write access (master). */
#define MCB_CMD_WRITE	2
/** MCB frame header command, ACK (slave). */
#define MCB_CMD_ACK	3
/** MCB frame header command, read access error (slave). */
#define MCB_CMD_RERR	5
/** MCB frame header command, write access error (slave). */
#define MCB_CMD_WERR	6
/** MCB frame header command, request info error (slave). */
#define MCB_CMD_IERR	4
/** MCB frame header command position. */
#define MCB_CMD_POS	1
/** MCB frame header command mask. */
#define MCB_CMD_MSK	0x000E

/** MCB frame header address position. */
#define MCB_ADDR_POS	4
/** MCB frame header address mask. */
#define MCB_ADDR_MSK	0xFFF0

/** MCB frame data position (first byte). */
#define MCB_DATA_POS	2
/** MCB frame data size. */
#define MCB_DATA_SZ	8

/** MCB frame payload size (header + data). */
#define MCB_PAYLOAD_SZ	10

/** MCB frame CRC high byte position. */
#define MCB_CRC_H	10
/** MCB frame CRC low byte position. */
#define MCB_CRC_L	11
/** MCB frame CRC size (bytes). */
#define MCB_CRC_SZ	2

#endif
