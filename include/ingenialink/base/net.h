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

#ifndef INGENIALINK_BASE_NET_H_
#define INGENIALINK_BASE_NET_H_

#include "public/ingenialink/net.h"

void il_net_base__state_set(il_net_t *net, il_net_state_t state);

int il_net_base__sw_subscribe(il_net_t *net, uint16_t id,
			      il_net_sw_subscriber_cb_t cb, void *ctx);

void il_net_base__sw_unsubscribe(il_net_t *net, int slot);

int il_net_base__emcy_subscribe(il_net_t *net, uint16_t id,
				il_net_emcy_subscriber_cb_t cb, void *ctx);

void il_net_base__emcy_unsubscribe(il_net_t *net, int slot);

int il_net_base__init(il_net_t *net, const il_net_opts_t *opts);

void il_net_base__deinit(il_net_t *net);

il_net_state_t il_net_base__state_get(il_net_t *net);

#endif
