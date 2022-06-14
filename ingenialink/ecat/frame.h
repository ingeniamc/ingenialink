#ifndef ECAT_MCB_FRAME_H_
#define ECAT_MCB_FRAME_H_

#include "frame.h"

/** Ethernet MCB frame CRC polynomial (16-CCITT). */
#define ECAT_MCB_CRC_POLY	0x1021

/** Ethernet MCB frame size (words). */
#define ECAT_MCB_FRAME_SZ	7

/** Ethernet MCB Header */
/** Ethernet MCB frame header high word position. */
#define ECAT_MCB_HDR_H_POS	0
/** Ethernet MCB frame header low word position. */
#define ECAT_MCB_HDR_L_POS	1
/** Ethernet MCB frame header high size (words). */
#define ECAT_MCB_HDR_H_SZ	1
/** Ethernet MCB frame header high size (words). */
#define ECAT_MCB_HDR_L_SZ	1
/** Ethernet MCB frame header size (words). */
#define ECAT_MCB_HDR_SZ	2

/** Ethernet MCB Config Data */
/** Ethernet MCB frame config data first word position. */
#define ECAT_MCB_CFG_DATA_POS	2
/** Ethernet MCB frame config data size (words). */
#define ECAT_MCB_CFG_DATA_SZ 4

/* Ethernet MCB CRC */
/** Ethernet MCB frame crc first word position. */
#define ECAT_MCB_CRC_POS 6
/** Ethernet MCB frame CRC size (words). */
#define ECAT_MCB_CRC_SZ 1

/** Ethernet MCB COCO Subnode value */
#define ECAT_MCB_SUBNODE_COCO    0
/** Ethernet MCB MOCO Subnode value */
#define ECAT_MCB_SUBNODE_MOCO    1   

/** Ethernet MCB default node */
#define ECAT_MCB_NODE_DFLT   0xA


/** Ethernet MCB frame header, pending bit */
#define ECAT_MCB_PENDING	1
/** Ethernet MCB frame header, pending bit position */
#define ECAT_MCB_PENDING_POS	0
/** Ethernet MCB frame header, pending bit mask */
#define ECAT_MCB_PENDING_MSK	0x0001

/** Ethernet MCB frame header command, request info (master). */
#define ECAT_MCB_CMD_INFO	0
/** Ethernet MCB frame header command, read access (master). */
#define ECAT_MCB_CMD_READ	1
/** Ethernet MCB frame header command, write access (master). */
#define ECAT_MCB_CMD_WRITE	2
/** Ethernet MCB frame header command, ACK (slave). */
#define ECAT_MCB_CMD_ACK	3
/** Ethernet MCB frame header command, read access error (slave). */
#define ECAT_MCB_CMD_RERR	5
/** Ethernet MCB frame header command, write access error (slave). */
#define ECAT_MCB_CMD_WERR	6
/** Ethernet MCB frame header command, request info error (slave). */
#define ECAT_MCB_CMD_IERR	4
/** Ethernet MCB frame header command position. */
#define ECAT_MCB_CMD_POS	1
/** Ethernet MCB frame header command mask. */
#define ECAT_MCB_CMD_MSK	0x000E

/** Ethernet MCB frame header address position. */
#define ECAT_MCB_ADDR_POS	4
/** Ethernet MCB frame header address mask. */
#define ECAT_MCB_ADDR_MSK	0xFFF0

/** Ethernet MCB frame data position (first byte). */
#define ECAT_MCB_DATA_POS	2
/** Ethernet MCB frame data size. */
#define ECAT_MCB_DATA_SZ	8

/** Ethernet MCB frame payload size (header + data). */
#define ECAT_MCB_PAYLOAD_SZ	10

/** Ethernet MCB frame CRC high byte position. */
#define ECAT_MCB_CRC_H	10
/** Ethernet MCB frame CRC low byte position. */
#define ECAT_MCB_CRC_L	11
/** Ethernet MCB frame CRC size (bytes). */
#define ECAT_MCB_CRC_SZ	2

#endif
