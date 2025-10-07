#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* итеративная длина "фибоначчиевой" строки:
   L0 = 1, L1 = 2, L_n = L_{n-2} + L_{n-1} */
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
  int count = 29; /* можно увеличить при необходимости */

  /* allocate array of char* to store strings F0..F(count-1) */
  char **fibs = malloc(sizeof(char *) * count);
  if (!fibs) {
    perror("malloc");
    return 1;
  }

  /* base strings */
  fibs[0] = strdup("0");
  fibs[1] = strdup("01");
  if (!fibs[0] || !fibs[1]) {
    perror("strdup");
    return 1;
  }

  /* build iteratively and store each string */
  for (int i = 2; i < count; ++i) {
    int len1 = strlen(fibs[i - 2]);
    int len2 = strlen(fibs[i - 1]);
    int len = len1 + len2;
    char *s = malloc((size_t)len + 1);
    if (!s) {
      perror("malloc");
      return 1;
    }
    memcpy(s, fibs[i - 2], (size_t)len1);
    memcpy(s + len1, fibs[i - 1], (size_t)len2);
    s[len] = '\0';
    fibs[i] = s;
  }

  /* покажем (опционально) */
  for (int i = 0; i < count; ++i) {
    printf("F[%d] (len=%zu): %s\n", i, strlen(fibs[i]), fibs[i]);
  }

  /* открываем файл в бинарном режиме */
  FILE *file = fopen("bits.bin", "wb");
  if (!file) {
    perror("Ошибка при открытии файла");
    /* cleanup */
    for (int i = 0; i < count; ++i)
      free(fibs[i]);
    free(fibs);
    return 1;
  }

  uint8_t byte = 0;
  int bits_count = 0; /* сколько бит уже в byte (0..7) */

  /* проходим по всем строкам и упаковываем биты LSB-first */
  for (int i = 0; i < count; ++i) {
    size_t len = strlen(fibs[i]);
    for (size_t k = 0; k < len; ++k) {
      char bit = fibs[i][k];
      /* если символ '1' — ставим соответствующий бит */
      if (bit == '1') {
        byte |= (uint8_t)(1u << bits_count);
      }
      bits_count++;

      /* когда набрали 8 бит — записываем и сбрасываем */
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
      /* для отладки/визуализации: */
      putchar(bit);
    }
    putchar('\n');
  }

  /* если остались незаписанные биты — допишем последний байт (младшие биты
   * используются) */
  if (bits_count > 0) {
    if (fwrite(&byte, sizeof(uint8_t), 1, file) != 1) {
      perror("fwrite");
    }
  }

  fclose(file);

  /* cleanup */
  for (int i = 0; i < count; ++i)
    free(fibs[i]);
  free(fibs);

  printf("Готово, записано в bits.bin\n");
  return 0;
}
