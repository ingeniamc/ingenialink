/*
 * MIT License
 *
 * Copyright (c) 2017 Ingenia-CAT S.L.
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

#ifndef MC_H
#define MC_H

/*******************************************************************************
 * Power Drive System
 ******************************************************************************/

/*
 * Controlword common bits.
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2, Figure 5
 */

#define IL_MC_CW_SO	(1 << 0)
#define IL_MC_CW_EV	(1 << 1)
#define IL_MC_CW_QS	(1 << 2)
#define IL_MC_CW_EO	(1 << 3)
#define IL_MC_CW_FR	(1 << 7)
#define IL_MC_CW_H	(1 << 8)

/*
 * Statusword common bits.
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2, Figure 6
 */

#define IL_MC_SW_RTSO	(1 << 0)
#define IL_MC_SW_SO	(1 << 1)
#define IL_MC_SW_OE	(1 << 2)
#define IL_MC_SW_F	(1 << 3)
#define IL_MC_SW_VE	(1 << 4)
#define IL_MC_SW_QS	(1 << 5)
#define IL_MC_SW_SOD	(1 << 6)
#define IL_MC_SW_W	(1 << 7)
#define IL_MC_SW_RM	(1 << 9)
#define IL_MC_SW_TR	(1 << 10)
#define IL_MC_SW_ILA	(1 << 11)

/*
 * PDS FSA states
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2,
 *      Figure 3, Table 30
 */

/*
 * Masks for PDS FSA states
 */
#define IL_MC_PDS_STA_NRTSO_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_SOD_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_RTSO_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_QS | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_SO_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_QS | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_OE_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_QS | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_QSA_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_QS | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_FRA_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_SOD)
#define IL_MC_PDS_STA_F_MSK	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F | IL_MC_SW_SOD)

/** Not ready to switch on. */
#define IL_MC_PDS_STA_NRTSO	0x0000
/** Switch on disabled. */
#define IL_MC_PDS_STA_SOD	IL_MC_SW_SOD
/** Ready to switch on. */
#define IL_MC_PDS_STA_RTSO	(IL_MC_SW_RTSO | IL_MC_SW_QS)
/** Switched on. */
#define IL_MC_PDS_STA_SO	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_QS)
/** Operation enabled. */
#define IL_MC_PDS_STA_OE	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_QS)
/** Quick stop active. */
#define IL_MC_PDS_STA_QSA	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE)
/** Fault reaction active. */
#define IL_MC_PDS_STA_FRA	(IL_MC_SW_RTSO | IL_MC_SW_SO | IL_MC_SW_OE | \
				 IL_MC_SW_F)
/** Fault. */
#define IL_MC_PDS_STA_F		IL_MC_SW_F
/** Unknown. */
#define IL_MC_PDS_STA_UNKNOWN	0xFFFFU

/*
 * PDS FSA commands
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2,
 *      Figure 3, Table 27
 */

/** Shutdown. */
#define IL_MC_PDS_CMD_SD	(IL_MC_CW_EV | IL_MC_CW_QS)
/** Switch on. */
#define IL_MC_PDS_CMD_SO	(IL_MC_CW_SO | IL_MC_CW_EV | IL_MC_CW_QS)
/** Switch on + enable operation. */
#define IL_MC_PDS_CMD_SOEO	(IL_MC_CW_SO | IL_MC_CW_EV | IL_MC_CW_QS | \
				 IL_MC_CW_EO)
/** Disable voltage. */
#define IL_MC_PDS_CMD_DV	0x0000U
/** Quick stop. */
#define IL_MC_PDS_CMD_QS	IL_MC_CW_EV
/** Disable operation. */
#define IL_MC_PDS_CMD_DO	(IL_MC_CW_SO | IL_MC_CW_EV | IL_MC_CW_QS)
/** Enable operation. */
#define IL_MC_PDS_CMD_EO	(IL_MC_CW_SO | IL_MC_CW_EV | IL_MC_CW_QS | \
				 IL_MC_CW_EO)
/** Fault reset. */
#define IL_MC_PDS_CMD_FR	IL_MC_CW_FR
/** Unknown command. */
#define IL_MC_PDS_CMD_UNKNOWN	0xFFFFU

/*******************************************************************************
 * Homing
 ******************************************************************************/

/*
 * Homing controlword bits
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2,
 *      Figure 34, Table 120
 */

/** Homing operation start. */
#define IL_MC_HOMING_CW_START	(1 << 4)
/** Halt */
#define IL_MC_HOMING_CW_HALT	(1 << 8)

/*
 * Homing statusword bits
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2, Figure 35
 */

/** Homing attained. */
#define IL_MC_HOMING_SW_ATT	(1 << 12)
/** Homing error. */
#define IL_MC_HOMING_SW_ERR	(1 << 13)

/*
 * Homing states
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2, Table 121
 */

/** Homing state mask. */
#define IL_MC_HOMING_STA_MSK		(IL_MC_SW_TR | IL_MC_HOMING_SW_ATT | \
					 IL_MC_HOMING_SW_ERR)
/** Homing procedure is in progress. */
#define IL_MC_HOMING_STA_INPROG		0x0000
/** Homing procedure is interrupted or not started. */
#define IL_MC_HOMING_STA_INT		(IL_MC_SW_TR)
/** Homing is attained, but target is not reached. */
#define IL_MC_HOMING_STA_ATT		(IL_MC_HOMING_SW_ATT)
/** Homing procedure is completed successfully. */
#define IL_MC_HOMING_STA_SUCCESS	(IL_MC_SW_TR | IL_MC_HOMING_SW_ATT)
/** Homing error occurred, velocity not zero. */
#define IL_MC_HOMING_STA_ERR_VNZ	(IL_MC_HOMING_SW_ERR)
/** Homing error ocurred, velocity is zero. */
#define IL_MC_HOMING_STA_ERR_VZ		(IL_MC_SW_TR | IL_MC_HOMING_SW_ERR)

/*******************************************************************************
 * Profile Position
 ******************************************************************************/

/*
 * Profile position controlword bits
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2,
 *      Figure 21, Table 85
 */

/** New set-point. */
#define IL_MC_PP_CW_NEWSP	(1 << 4)
/** Change set immediately */
#define IL_MC_PP_CW_IMMEDIATE	(1 << 5)
/** Target position is relative. */
#define IL_MC_PP_CW_REL		(1 << 6)

/*
 * Profile position specific statusword bits
 *
 * Reference:
 *      DS402 Drives and motion control device profile - Part 2,
 *      Figure 22, Table 87
 */

/** Set-point acknowledge. */
#define IL_MC_PP_SW_SPACK	(1 << 12)
/** Following error. */
#define IL_MC_PP_SW_FOLLOWERR	(1 << 13)

#endif
