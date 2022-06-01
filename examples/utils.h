#ifndef UTILS_H_
#define UTILS_H_

#include <ingenialink/ingenialink.h>

/**
 * Obtain protocol type from string
 *
 * @param [in] name
 *	Protocol name.
 *
 * @return
 *	Protocol type (defaults to E-USB).
 */
il_net_prot_t str2prot(const char *name);

#endif
