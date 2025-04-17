#include <libtap/libtap.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void) {
  tap_buffer *buffer = tap_buffer_tmpfile();
  assert(buffer != NULL);

  // Build a 100 byte buffer of 0xFF
  uint8_t byte_buf[100];
  for (int i = 0; i < 100; i++) {
    byte_buf[i] = i;
  }

  // Write the buffer 5 times
  for (int i = 0; i < 5; i++) {
    size_t res = tap_buffer_write(buffer, byte_buf, 100);
    assert(res == 100);
    for (int j = 0; j < 100; j++) {
      byte_buf[j]++;
    }
  }

  // Ensure seeking to the start works
  bool res = tap_buffer_seek(buffer, 0, SEEK_SET);
  assert(res);

  // Test to ensure the correct data can be read back
  uint8_t read_buf[100];
  for (int i = 0; i < 5; i++) {
    size_t read = tap_buffer_read(buffer, read_buf, 100);
    assert(read == 100);
    for (int j = 0; j < 100; j++) {
      assert(read_buf[j] == j + i);
    }
  }

  // Ensure reading past the end results in eof
  bool failed_read_eof = tap_buffer_read(buffer, read_buf, 1);
  assert(failed_read_eof == 0);

  // Close the buffer
  tap_buffer_close(buffer);
  // Free the buffer too
  tap_buffer_free(buffer);

  return 0;
}
