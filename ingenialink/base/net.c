#include "../net.h"

#include <string.h>

#include "ingenialink/err.h"

/*******************************************************************************
 * Base/Shared implementation
 ******************************************************************************/

void il_net_base__state_set(il_net_t *net, il_net_state_t state)
{
	osal_mutex_lock(net->state_lock);
	net->state = state;
	osal_mutex_unlock(net->state_lock);
}

int il_net_base__sw_subscribe(il_net_t *net, uint16_t id,
			      il_net_sw_subscriber_cb_t cb, void *ctx)
{
	int r = 0;
	int slot;

	osal_mutex_lock(net->sw_subs.lock);

	/* check if already subscribed */
	for (slot = 0; slot < net->sw_subs.sz; slot++) {
		if (net->sw_subs.subs[slot].id == id) {
			ilerr__set("Node already subscribed");
			r = IL_EALREADY;
			goto unlock;
		}
	}

	/* look for the first empty slot */
	for (slot = 0; slot < net->sw_subs.sz; slot++) {
		if (!net->sw_subs.subs[slot].cb)
			break;
	}

	/* increase array if no space left */
	if (slot == net->sw_subs.sz) {
		size_t sz;
		il_net_sw_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * net->sw_subs.sz * sizeof(*subs);
		subs = realloc(net->sw_subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		net->sw_subs.subs = subs;
		net->sw_subs.sz = sz;
	}

	net->sw_subs.subs[slot].id = id;
	net->sw_subs.subs[slot].cb = cb;
	net->sw_subs.subs[slot].ctx = ctx;

	r = slot;

unlock:
	osal_mutex_unlock(net->sw_subs.lock);

	return r;
}

void il_net_base__sw_unsubscribe(il_net_t *net, int slot)
{
	osal_mutex_lock(net->sw_subs.lock);

	/* skip out of range slot */
	if (slot >= net->sw_subs.sz)
		goto unlock;

	net->sw_subs.subs[slot].id = 0;
	net->sw_subs.subs[slot].cb = NULL;
	net->sw_subs.subs[slot].ctx = NULL;

unlock:
	osal_mutex_unlock(net->sw_subs.lock);
}

int il_net_base__emcy_subscribe(il_net_t *net, uint16_t id,
				il_net_emcy_subscriber_cb_t cb, void *ctx)
{
	int r;
	int slot;

	/* check if already subscribed */
	for (slot = 0; slot < net->emcy_subs.sz; slot++) {
		if (net->emcy_subs.subs[slot].id == id) {
			ilerr__set("Node already subscribed");
			r = IL_EALREADY;
			goto unlock;
		}
	}

	/* look for the first empty slot */
	for (slot = 0; slot < net->emcy_subs.sz; slot++) {
		if (!net->emcy_subs.subs[slot].cb)
			break;
	}

	/* increase array if no space left */
	if (slot == net->emcy_subs.sz) {
		size_t sz;
		il_net_emcy_subscriber_t *subs;

		/* double in size on each realloc */
		sz = 2 * net->emcy_subs.sz * sizeof(*subs);
		subs = realloc(net->emcy_subs.subs, sz);
		if (!subs) {
			ilerr__set("Subscribers re-allocation failed");
			r = IL_ENOMEM;
			goto unlock;
		}

		net->emcy_subs.subs = subs;
		net->emcy_subs.sz = sz;
	}

	net->emcy_subs.subs[slot].id = id;
	net->emcy_subs.subs[slot].cb = cb;
	net->emcy_subs.subs[slot].ctx = ctx;

	r = slot;

unlock:
	osal_mutex_unlock(net->emcy_subs.lock);

	return r;
}

void il_net_base__emcy_unsubscribe(il_net_t *net, int slot)
{
	osal_mutex_lock(net->emcy_subs.lock);

	/* skip out of range slot */
	if (slot >= net->emcy_subs.sz)
		goto unlock;

	net->emcy_subs.subs[slot].id = 0;
	net->emcy_subs.subs[slot].cb = NULL;
	net->emcy_subs.subs[slot].ctx = NULL;

unlock:
	osal_mutex_unlock(net->emcy_subs.lock);
}

int il_net_base__init(il_net_t *net, const il_net_opts_t *opts)
{
	int r;

	/* initialize */
	net->port = strdup(opts->port);
	net->timeout_rd = opts->timeout_rd;
	net->timeout_wr = opts->timeout_wr;

	/* initialize network lock */
	net->lock = osal_mutex_create();
	if (!net->lock) {
		ilerr__set("Network lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_init;
	}

	/* initialize network state */
	net->state_lock = osal_mutex_create();
	if (!net->state_lock) {
		ilerr__set("Network state lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_lock;
	}

	net->state = IL_NET_STATE_DISCONNECTED;

	/* initialize statusword update subscribers */
	net->sw_subs.subs = calloc(SW_SUBS_SZ_DEF, sizeof(*net->sw_subs.subs));
	if (!net->sw_subs.subs) {
		ilerr__set("Network statusword subscribers allocation failed");
		r = IL_ENOMEM;
		goto cleanup_state_lock;
	}

	net->sw_subs.lock = osal_mutex_create();
	if (!net->sw_subs.lock) {
		ilerr__set("Network statusword lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_sw_subs_subs;
	}

	net->sw_subs.sz = SW_SUBS_SZ_DEF;

	/* initialize emcy update subscribers */
	net->emcy_subs.subs = calloc(EMCY_SUBS_SZ_DEF,
				     sizeof(*net->emcy_subs.subs));
	if (!net->emcy_subs.subs) {
		ilerr__set("Network emergency subscribers allocation failed");
		r = IL_ENOMEM;
		goto cleanup_sw_subs_lock;
	}

	net->emcy_subs.lock = osal_mutex_create();
	if (!net->emcy_subs.lock) {
		ilerr__set("Network emergency lock allocation failed");
		r = IL_ENOMEM;
		goto cleanup_emcy_subs_subs;
	}

	net->emcy_subs.sz = EMCY_SUBS_SZ_DEF;

	return 0;

cleanup_emcy_subs_subs:
	free(net->emcy_subs.subs);

cleanup_sw_subs_lock:
	osal_mutex_destroy(net->sw_subs.lock);

cleanup_sw_subs_subs:
	free(net->sw_subs.subs);

cleanup_state_lock:
	osal_mutex_destroy(net->state_lock);

cleanup_lock:
	osal_mutex_destroy(net->lock);

cleanup_init:
	free(net->port);

	return r;
}

void il_net_base__deinit(il_net_t *net)
{
	osal_mutex_destroy(net->emcy_subs.lock);
	free(net->emcy_subs.subs);

	osal_mutex_destroy(net->sw_subs.lock);
	free(net->sw_subs.subs);

	osal_mutex_destroy(net->state_lock);

	osal_mutex_destroy(net->lock);

	free(net->port);
}

il_net_state_t il_net_base__state_get(il_net_t *net)
{
	il_net_state_t state;

	osal_mutex_lock(net->state_lock);
	state = net->state;
	osal_mutex_unlock(net->state_lock);

	return state;
}
