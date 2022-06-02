#include "utils.h"

#include <string.h>

il_net_prot_t str2prot(const char *name) {
	if (strcmp(name, "eusb") == 0)
		return IL_NET_PROT_EUSB;
	if (strcmp(name, "mcb") == 0)
		return IL_NET_PROT_MCB;
	else
		return IL_NET_PROT_EUSB;
}
