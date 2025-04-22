#include <libtap/libtap.h>

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void test_buffer(tap_buffer *buffer) {
  assert(buffer != NULL);

  uint8_t write_buf[100];
  for (int i = 0; i < 100; i++) {
    write_buf[i] = i;
  }

  // Write buffer 5 times, increasing the set by 1 each time
  for (int i = 0; i < 5; i++) {
    size_t write_result = tap_buffer_write(buffer, write_buf, 100);
    assert(write_result == 100);
    for (int j = 0; j < 100; j++) {
      write_buf[j]++;
    }
  }

  bool seek_result = tap_buffer_seek(buffer, 0, SEEK_SET);
  assert(seek_result);

  // Read data back, ensuring the correct values
  uint8_t read_buf[100];
  for (int i = 0; i < 5; i++) {
    size_t read = tap_buffer_read(buffer, read_buf, 100);
    assert(read == 100);
    for (int j = 0; j < 100; j++) {
      assert(read_buf[j] == j + i);
    }
  }

  // Ensure reading past the end results in EOF.
  bool failed_read_eof = tap_buffer_read(buffer, read_buf, 1);
  assert(failed_read_eof == 0);

  // Close the buffer
  tap_buffer_close(buffer);
  tap_buffer_free(buffer);
}

int main(void) {
  tap_buffer *tmpfile_buffer = tap_buffer_tmpfile();
  tap_buffer *dyn_mem_buffer = tap_buffer_dynamic_memory(2);
  tap_buffer *dyn_mem_buffer_no_storage = tap_buffer_dynamic_memory(0);

  char *static_buf = malloc(500);
  tap_buffer *static_mem_buffer = tap_buffer_static_memory(static_buf, 500);

  test_buffer(tmpfile_buffer);
  test_buffer(dyn_mem_buffer);
  test_buffer(dyn_mem_buffer_no_storage);
  test_buffer(static_mem_buffer);

  free(static_buf);

  return 0;
}
