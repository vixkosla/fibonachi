#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* (fib_length_iter можно удалить, если не используется) */
int fib_length_iter(int n) {
  if (n == 0)
    return 1;
  if (n == 1)
    return 2;
  int a = 1, b = 2, c = 0;
  for (int i = 2; i <= n; ++i) {
    c = a + b;
    a = b;
    b = c;
  }
  return c;
}

int main(void) {
  int count = 20; /* можно увеличить при необходимости */
  if (count < 1)
    return 0;
  if (count == 1) {
    puts("F[0] (len=1): 0");
    return 0;
  }

  char **fibs = malloc(sizeof(char *) * (size_t)count);
  if (!fibs) {
    perror("malloc");
    return 1;
  }

  fibs[0] = strdup("0");
  fibs[1] = strdup("01");
  if (!fibs[0] || !fibs[1]) {
    perror("strdup");
    free(fibs[0]);
    free(fibs[1]);
    free(fibs);
    return 1;
  }

  for (int i = 2; i < count; ++i) {
    size_t len1 = strlen(fibs[i - 2]);
    size_t len2 = strlen(fibs[i - 1]);
    size_t len = len1 + len2;
    char *s = malloc(len + 1);
    if (!s) {
      perror("malloc");
      /* cleanup previously allocated */
      for (int j = 0; j < i; ++j)
        free(fibs[j]);
      free(fibs);
      return 1;
    }
    memcpy(s, fibs[i - 2], len1);
    memcpy(s + len1, fibs[i - 1], len2);
    s[len] = '\0';
    fibs[i] = s;
  }

  FILE *file = fopen("bits.bin", "wb");
  if (!file) {
    perror("Ошибка при открытии файла");
    for (int i = 0; i < count; ++i)
      free(fibs[i]);
    free(fibs);
    return 1;
  }
  setvbuf(file, NULL, _IOFBF, 1 << 20); /* опционально: 1 MiB буфер */

  uint8_t byte = 0;
  int bits_count = 0;

  for (int i = 0; i < count; ++i) {
    size_t len = strlen(fibs[i]);
    // printf("F[%d] (len=%zu): %s\n", i, len, fibs[i]); /* отладочный вывод */
    for (size_t k = 0; k < len; ++k) {
      char bit = fibs[i][k];
      if (bit == '1')
        byte |= (uint8_t)(1u << (7 - bits_count));
      bits_count++;
      if (bits_count == 8) {
        if (fwrite(&byte, sizeof(uint8_t), 1, file) != 1) {
          perror("fwrite");
          fclose(file);
          for (int j = 0; j < count; ++j)
            free(fibs[j]);
          free(fibs);
          return 1;
        }
        bits_count = 0;
        byte = 0;
      }
      // putchar(bit); /* опционально: печатаем биты в терминал */
    }
    // putchar('\n');
  }

  if (bits_count > 0) {
    if (fwrite(&byte, sizeof(uint8_t), 1, file) != 1)
      perror("fwrite");
  }

  fclose(file);
  for (int i = 0; i < count; ++i)
    free(fibs[i]);
  free(fibs);

  printf("Готово, записано в bits.bin\n");
  return 0;
}
