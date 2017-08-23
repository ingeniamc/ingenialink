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

#include "node.h"

#include <string.h>
#include <errno.h>

#include "ingenialink/err.h"
#include "ingenialink/utils.h"

/*******************************************************************************
 * Public
 ******************************************************************************/

il_node_t *il_node_create(il_net_t *net, uint8_t id)
{
	il_node_t *node;
	int r;

	/* validate network */
	if (!net) {
		ilerr__set("Invalid network (NULL)");
		return NULL;
	}

	/* validate node id */
	if ((id < NODEID_MIN) || (id > NODEID_MAX)) {
		ilerr__set("Node id out of range");
		return NULL;
	}

	/* allocate node */
	node = malloc(sizeof(*node));
	if (!node) {
		ilerr__set("Could not allocate node (%s)", strerror(errno));
		return NULL;
	}

	node->net = net;
	node->id = id;

	/* disable asynchronous statusword (unused, we gain some bw) */
	r = il_node_write_u8(node, UARTCFG_SW_IDX, UARTCFG_SW_SIDX, 0);
	if (r < 0)
		goto cleanup_node;

	return node;

cleanup_node:
	free(node);

	return NULL;
}

void il_node_destroy(il_node_t *node)
{
	/* validate node */
	if (!node)
		return;

	/* de-allocate resources */
	free(node);
}

int il_node_read(il_node_t *node, uint16_t idx, uint8_t sidx, void *buf,
		 size_t sz, size_t *recvd)
{
	int r;

	/* validate nodework */
	if (!node) {
		ilerr__set("Invalid node (NULL)");
		return IL_EFAULT;
	}

	/* send request, receice response */
	r = il_net__send(node->net, node->id, idx, sidx, NULL, 0);
	if (r < 0)
		return r;

	return il_net__recv(node->net, node->id, idx, sidx, buf, sz, recvd);
}

int il_node_read_u8(il_node_t *node, uint16_t idx, uint8_t sidx, uint8_t *buf)
{
	return il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
}

int il_node_read_s8(il_node_t *node, uint16_t idx, uint8_t sidx, int8_t *buf)
{
	return il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
}

int il_node_read_u16(il_node_t *node, uint16_t idx, uint8_t sidx, uint16_t *buf)
{
	int r;

	r = il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_16(*buf);

	return r;
}

int il_node_read_s16(il_node_t *node, uint16_t idx, uint8_t sidx, int16_t *buf)
{
	int r;

	r = il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int16_t)__swap_16(*buf);

	return r;
}

int il_node_read_u32(il_node_t *node, uint16_t idx, uint8_t sidx, uint32_t *buf)
{
	int r;

	r = il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_32(*buf);

	return r;
}

int il_node_read_s32(il_node_t *node, uint16_t idx, uint8_t sidx, int32_t *buf)
{
	int r;

	r = il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int32_t)__swap_32(*buf);

	return r;
}

int il_node_read_u64(il_node_t *node, uint16_t idx, uint8_t sidx,
		     uint64_t *buf)
{
	int r;

	r = il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = __swap_64(*buf);

	return r;
}

int il_node_read_s64(il_node_t *node, uint16_t idx, uint8_t sidx, int64_t *buf)
{
	int r;

	r = il_node_read(node, idx, sidx, buf, sizeof(*buf), NULL);
	if (r == 0)
		*buf = (int64_t)__swap_64(*buf);

	return r;
}

int il_node_write(il_node_t *node, uint16_t idx, uint8_t sidx, const void *data,
		  size_t sz)
{
	/* validate nodework */
	if (!node) {
		ilerr__set("Invalid node (NULL)");
		return IL_EFAULT;
	}

	/* send write request */
	return il_net__send(node->net, node->id, idx, sidx, data, sz);
}

int il_node_write_u8(il_node_t *node, uint16_t idx, uint8_t sidx, uint8_t val)
{
	return il_node_write(node, idx, sidx, &val, sizeof(val));
}

int il_node_write_s8(il_node_t *node, uint16_t idx, uint8_t sidx, int8_t val)
{
	return il_node_write(node, idx, sidx, &val, sizeof(val));
}

int il_node_write_u16(il_node_t *node, uint16_t idx, uint8_t sidx, uint16_t val)
{
	uint16_t val_;

	val_ = __swap_16(val);

	return il_node_write(node, idx, sidx, &val_, sizeof(val_));
}

int il_node_write_s16(il_node_t *node, uint16_t idx, uint8_t sidx, int16_t val)
{
	int16_t val_;

	val_ = (int16_t)__swap_16(val);

	return il_node_write(node, idx, sidx, &val_, sizeof(val_));
}

int il_node_write_u32(il_node_t *node, uint16_t idx, uint8_t sidx, uint32_t val)
{
	uint32_t val_;

	val_ = __swap_32(val);

	return il_node_write(node, idx, sidx, &val_, sizeof(val_));
}

int il_node_write_s32(il_node_t *node, uint16_t idx, uint8_t sidx, int32_t val)
{
	int32_t val_;

	val_ = (int32_t)__swap_32(val);

	return il_node_write(node, idx, sidx, &val_, sizeof(val_));
}

int il_node_write_u64(il_node_t *node, uint16_t idx, uint8_t sidx, uint64_t val)
{
	uint64_t val_;

	val_ = __swap_64(val);

	return il_node_write(node, idx, sidx, &val_, sizeof(val_));
}

int il_node_write_s64(il_node_t *node, uint16_t idx, uint8_t sidx,
		      int64_t val)
{
	int64_t val_;

	val_ = (int64_t)__swap_64(val);

	return il_node_write(node, idx, sidx, &val_, sizeof(val_));
}
