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

#ifndef PUBLIC_INGENIALINK_NODE_H_
#define PUBLIC_INGENIALINK_NODE_H_

#include "net.h"

IL_BEGIN_DECL

/**
 * @file ingenialink/node.h
 * @brief Nodes.
 * @defgroup IL_NODE Nodes
 * @ingroup IL
 * @{
 */

/** IngeniaLink node instance. */
typedef struct il_node il_node_t;

/**
 * Create IngeniaLink node instance.
 *
 * @param [in] net
 *	IngeniaLink network.
 * @param [in] id
 *	Node id.
 *
 * @return
 *	  Node instance (NULL if it could not be created).
 */
IL_EXPORT il_node_t *il_node_create(il_net_t *net, uint8_t id);

/**
 * Destroy an IngeniaLink node instance.
 *
 * @param [in] node
 *	  IngeniaLink node instance.
 */
IL_EXPORT void il_node_destroy(il_node_t *node);

/**
 * Read data from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 * @param [in] sz
 *	  Buffer size.
 * @param [out] recvd
 *	  Variable where the number of bytes received will be stored (optional).
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read(il_node_t *node, uint16_t idx, uint8_t sidx,
			   void *buf, size_t sz, size_t *recvd);

/**
 * Read unsigned 8-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_u8(il_node_t *node, uint16_t idx, uint8_t sidx,
			      uint8_t *buf);

/**
 * Read signed 8-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_s8(il_node_t *node, uint16_t idx, uint8_t sidx,
			      int8_t *buf);

/**
 * Read unsigned 16-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_u16(il_node_t *node, uint16_t idx, uint8_t sidx,
			       uint16_t *buf);

/**
 * Read signed 16-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_s16(il_node_t *node, uint16_t idx, uint8_t sidx,
			       int16_t *buf);

/**
 * Read unsigned 32-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_u32(il_node_t *node, uint16_t idx, uint8_t sidx,
			       uint32_t *buf);

/**
 * Read signed 32-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_s32(il_node_t *node, uint16_t idx, uint8_t sidx,
			       int32_t *buf);

/**
 * Read unsigned 64-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_u64(il_node_t *node, uint16_t idx, uint8_t sidx,
			       uint64_t *buf);

/**
 * Read signed 64-bit value from a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [out] buf
 *	  Buffer where to store received data.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_read_s64(il_node_t *node, uint16_t idx, uint8_t sidx,
			       int64_t *buf);

/**
 * Write data to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] buf
 *	  Buffer where to pick data from.
 * @param [in] sz
 *	  Buffer size.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write(il_node_t *node, uint16_t idx, uint8_t sidx,
			    const void *buf, size_t sz);

/**
 * Write unsigned 8-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_u8(il_node_t *node, uint16_t idx, uint8_t sidx,
			       uint8_t val);

/**
 * Write signed 8-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_s8(il_node_t *node, uint16_t idx, uint8_t sidx,
			       int8_t val);

/**
 * Write unsigned 16-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_u16(il_node_t *node, uint16_t idx, uint8_t sidx,
				uint16_t val);

/**
 * Write signed 16-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_s16(il_node_t *node, uint16_t idx, uint8_t sidx,
				int16_t val);

/**
 * Write unsigned 32-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_u32(il_node_t *node, uint16_t idx, uint8_t sidx,
				uint32_t val);

/**
 * Write signed 32-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_s32(il_node_t *node, uint16_t idx, uint8_t sidx,
				int32_t val);

/**
 * Write unsigned 64-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_u64(il_node_t *node, uint16_t idx, uint8_t sidx,
				uint64_t val);

/**
 * Write signed 64-bit integer to a register.
 *
 * @param [in] node
 *	  IngeniaLink node.
 * @param [in] idx
 *	  Register index.
 * @param [in] sidx
 *	  Register subindex.
 * @param [in] val
 *	  Value.
 *
 * @return
 *	  0 on success, error code otherwise.
 */
IL_EXPORT int il_node_write_s64(il_node_t *node, uint16_t idx, uint8_t sidx,
				int64_t val);

/** @} */

IL_END_DECL

#endif
