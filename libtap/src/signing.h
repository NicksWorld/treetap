#ifndef LIBTAP_SIGNING_H
#define LIBTAP_SIGNING_H

#include "libtap/libtap.h"

tap_error tap_keypair_sign(tap_keypair *keypair, tap_buffer *input,
                           unsigned char *signature);

#endif
