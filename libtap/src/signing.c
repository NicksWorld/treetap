#include "libtap/libtap.h"

#include <string.h>

#include <sodium.h>
#include <sodium/crypto_sign.h>

#include "export.h"

struct tap_keypair {
  unsigned char public_key[crypto_sign_PUBLICKEYBYTES];
  unsigned char secret_key[crypto_sign_SECRETKEYBYTES];
};

/*
 * Initialize a keypair
 */
LIBTAP_EXPORT tap_keypair *tap_keypair_new(unsigned char *public_key,
                                           unsigned char *secret_key) {
  tap_keypair *keypair = malloc(sizeof(tap_keypair));
  if (!keypair) {
    return NULL;
  }

  memcpy(keypair->public_key, public_key, crypto_sign_PUBLICKEYBYTES);
  memcpy(keypair->secret_key, secret_key, crypto_sign_SECRETKEYBYTES);

  return keypair;
}
LIBTAP_EXPORT tap_keypair *tap_keypair_generate(void) {
  if (sodium_init() == -1) {
    return NULL; // Ensure sodium is initialized
  }

  tap_keypair* keypair = malloc(sizeof(tap_keypair));
  if (!keypair) {
    return NULL;
  }

  crypto_sign_keypair(keypair->secret_key, keypair->public_key);

  return keypair;
}

LIBTAP_EXPORT void tap_keypair_free(tap_keypair * keypair) {
  if (keypair && sodium_init() != -1) {
    // Zero out the memory that was storing the keypair if possible
    sodium_memzero(keypair, sizeof(tap_keypair));
  }
  free(keypair);
}

/**
 * Retrieve the components of the keypair
 */
LIBTAP_EXPORT void tap_keypair_public(tap_keypair* keypair, unsigned char* out_pk) {
  memcpy(out_pk, keypair->public_key, crypto_sign_PUBLICKEYBYTES);
}
LIBTAP_EXPORT void tap_keypair_secret(tap_keypair* keypair, unsigned char* out_sk) {
  memcpy(out_sk, keypair->secret_key, crypto_sign_SECRETKEYBYTES);
}

/**
 * Sign a buffer of data
 */
// TODO: Replace UNSUPPORTED with TAP_ERR_CRYPTO
tap_error tap_keypair_sign(tap_keypair* keypair, tap_buffer* input, unsigned char* signature) {
  if (sodium_init() == -1) {
    return TAP_ERR_UNSUPPORTED;
  }

  crypto_sign_state state;
  if (crypto_sign_init(&state) == -1) {
    return TAP_ERR_UNSUPPORTED;
  }

  unsigned char read_buffer[1024];
  for (;;) {
    size_t read = tap_buffer_read(input, read_buffer, 1024);

    if (read != 0) {
      // Update with next portion of data
      if (crypto_sign_update(&state, read_buffer, read) == -1) {
        return TAP_ERR_UNSUPPORTED;
      }
      continue;
    }
    if (tap_buffer_fatal(input)) {
      return TAP_ERR_IO;
    }
    break;
  }

  if (crypto_sign_final_create(&state, signature, NULL, keypair->secret_key) == -1) {
    return TAP_ERR_UNSUPPORTED;
  }

  return TAP_ERR_OK;
}
