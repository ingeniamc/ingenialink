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
