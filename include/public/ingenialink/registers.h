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

#ifndef PUBLIC_INGENIALINK_REGISTERS_H_
#define PUBLIC_INGENIALINK_REGISTERS_H_

#include "dict_labels.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/registers.h
 * @brief Registers.
 * @defgroup IL_REGS Registers
 * @ingroup IL
 * @{
 */

/** Register data type. */
typedef enum {
	/** Unsigned 8-bit integer. */
	IL_REG_DTYPE_U8,
	/** Signed 8-bit integer. */
	IL_REG_DTYPE_S8,
	/** Unsigned 16-bit integer. */
	IL_REG_DTYPE_U16,
	/** Signed 16-bit integer. */
	IL_REG_DTYPE_S16,
	/** Unsigned 32-bit integer. */
	IL_REG_DTYPE_U32,
	/** Signed 32-bit integer. */
	IL_REG_DTYPE_S32,
	/** Unsigned 64-bit integer. */
	IL_REG_DTYPE_U64,
	/** Signed 64-bit integer. */
	IL_REG_DTYPE_S64,
	/** Float. */
	IL_REG_DTYPE_FLOAT,
	/** String. */
	IL_REG_DTYPE_STR,
} il_reg_dtype_t;

/** Register access. */
typedef enum {
	/** Read/Write. */
	IL_REG_ACCESS_RW,
	/** Read only. */
	IL_REG_ACCESS_RO,
	/** Write only */
	IL_REG_ACCESS_WO,
} il_reg_access_t;

/** Register physical units type. */
typedef enum {
	/** None. */
	IL_REG_PHY_NONE,
	/** Torque. */
	IL_REG_PHY_TORQUE,
	/** Position. */
	IL_REG_PHY_POS,
	/** Velocity. */
	IL_REG_PHY_VEL,
	/** Acceleration. */
	IL_REG_PHY_ACC,
	/** Voltage (relative to DC bus). */
	IL_REG_PHY_VOLT_REL,
	/** Radians. */
	IL_REG_PHY_RAD,
} il_reg_phy_t;

/** Values. */
typedef union {
	/** Unsigned 8-bit value. */
	uint8_t u8;
	/** Signed 8-bit value. */
	int8_t s8;
	/** Unsigned 16-bit value. */
	uint16_t u16;
	/** Signed 16-bit value. */
	int16_t s16;
	/** Unsigned 32-bit value. */
	uint32_t u32;
	/** Signed 32-bit value. */
	int32_t s32;
	/** Unsigned 64-bit value. */
	uint64_t u64;
	/** Signed 64-bit value. */
	int64_t s64;
	/** Float value. */
	float flt;
} il_reg_value_t;

/** Range. */
typedef struct {
	/** Minimum. */
	il_reg_value_t min;
	/** Maximum. */
	il_reg_value_t max;
} il_reg_range_t;

/* Enumeration */
typedef struct {
	/** Value. */
	int value;
	/** Labels. */
	const char *label;
} il_reg_enum_t;

/** Register. */
typedef struct {
	/** Identifier. */
	const char *identifier;
	/** Units. */
	const char *units;
	/** Subnode. */
	uint8_t subnode;
	/** Cyclic. */
	const char *cyclic;
	/** Address. */
	uint32_t address;
	/** Data type. */
	il_reg_dtype_t dtype;
	/** Access type. */
	il_reg_access_t access;
	/** Physical units type. */
	il_reg_phy_t phy;
	/** Range. */
	il_reg_range_t range;
	/** Storage. */
	il_reg_value_t storage;
	/** Storage is valid. */
	int storage_valid;
	/** Labels dictionary. */
	il_dict_labels_t *labels;
	/** Enumerations array. */
	il_reg_enum_t enums[20];
	/** Enumerations count. */
	int enums_count;
	/** Category ID. */
	const char *cat_id;
	/** Subcategory ID. */
	const char *scat_id;
	/** Internal use. */
	uint8_t internal_use;
} il_reg_t;

/** @} */

IL_END_DECL

#endif
