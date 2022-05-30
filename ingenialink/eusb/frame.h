#ifndef FRAME_H
#define FRAME_H

#include "ingenialink/eusb/frame.h"

/**
 * Swap index value on little-endian systems.
 *
 * @notes
 *      The index field is the only one that uses big endian format...
 */
#ifndef IL_BIG_ENDIAN
#define __swap_index(x) \
	((((uint16_t)(x) & 0xFF00U) >> 8) | \
	 (((uint16_t)(x) & 0x00FFU) << 8))
#else
#define __swap_index(x) (x)
#endif

/*
 * IngeniaLink Frame
 *
 * +===========+=========+=========+============+==========+==========+========+
 * | Net. addr | Func.   | MEI     | Prot. Ctl. |  Res.    |    Node  | Index  |
 * +===========+=========+=========+============+==========+==========+========+
 * | 1         | 1       | 1       | 1          | 1        | 1        | 2 (H-L)|
 * +-----------+---------+---------+------------+----------+----------+--------+
 * | Node      | 43      | 13      | 0/1 (R/W)  | 0        | Node     | Index  |
 * +===========+=========+=========+============+==========+==========+========+
 * | Subindex  | Addr.   | N. data | Data       | CRC      | SYNC     |        |
 * +===========+=========+=========+============+==========+==========+========+
 * | 1         | 2 (H-L) | 2 (H-L) | 0-8 (H..L) | 0-2 (H-L)| 4        |        |
 * +-----------+---------+---------+------------+----------+----------+--------+
 * | Subindex  | 0x0000  | 0-8     | Data       | IBM-16   | All 0x55 |        |
 * +-----------+---------+---------+------------+----------+----------+--------+
 */

/** Address field. */
#define FR_ADDR_FLD	0
/** Function field. */
#define FR_FUNC_FLD	1
/** MEI type field. */
#define FR_MEI_FLD	2
/** Protocol control field. */
#define FR_PROT_FLD	3
/** Reserved field. */
#define FR_RES_FLD	4
/** Node field. */
#define FR_NODE_FLD	5
/** Index field (H). */
#define FR_INDEX_H_FLD	6
/** Index field (L). */
#define FR_INDEX_L_FLD	7
/** Subindex field. */
#define FR_SINDEX_FLD	8
/** Starting address field (H). */
#define FR_SADDR_H_FLD	9
/** Starting address field (L). */
#define FR_SADDR_L_FLD	10
/** Num. of data field (H). */
#define FR_NDATA_H_FLD	11
/** Num. of data field (L). */
#define FR_NDATA_L_FLD	12
/** Data field (Highest). */
#define FR_DATA_FLD	13

/** Frame function code. */
#define FR_FUNC		43U
/** Frame MEI type. */
#define FR_MEI		13U
/** Frame read mode. */
#define FR_RD		0U
/** Frame write mode. */
#define FR_WR		1U
/** Frame SYNC code. */
#define FR_SYNC		0x55U
/** Frame SYNC size. */
#define FR_SYNC_SZ	4U

#endif
