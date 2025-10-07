#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  printf("Hello world!\n");

  /* --- ТВОЙ ОРИГИНАЛЬНЫЙ БЛОК (оставлен без удаления) --- */
  char a1[2048] = "0";
  char a2[2048] = "01";
  char a3[2048] = "";

  int count = 15;

  /* Этот цикл оставлен точно как у тебя — он печатает a3 каждый шаг */
  for (int i = 0; i < count; i++) {
    a3[0] = '\0';

    strcat(a3, a1);
    strcat(a3, a2);

    strcpy(a1, a2);
    strcpy(a2, a3);

    printf("%s\n", a3); /* вывод для сравнения (не удалялось) */
  }

  /* --- Построим и сохраним F0..F(count-1) для дальнейшей записи в файл --- */
  char **fibs = malloc(sizeof(char *) * (size_t)count);
  if (!fibs) {
    perror("malloc");
    return 1;
  }

  /* базовые */
  fibs[0] = strdup("0");
  if (!fibs[0]) {
    perror("strdup");
    return 1;
  }

  if (count > 1) {
    fibs[1] = strdup("01");
    if (!fibs[1]) {
      perror("strdup");
      return 1;
    }
  }

  for (int i = 2; i < count; ++i) {
    size_t l1 = strlen(fibs[i - 2]);
    size_t l2 = strlen(fibs[i - 1]);
    size_t len = l1 + l2;
    char *s = malloc(len + 1);
    if (!s) {
      perror("malloc");
      /* cleanup */
      for (int j = 0; j < i; ++j)
        free(fibs[j]);
      free(fibs);
      return 1;
    }
    memcpy(s, fibs[i - 2], l1);
    memcpy(s + l1, fibs[i - 1], l2);
    s[len] = '\0';
    fibs[i] = s;
  }

  /* Открываем файл в бинарном режиме */
  FILE *file = fopen("bits.bin", "wb");
  if (!file) {
    perror("Ошибка при открытии файла");
    for (int i = 0; i < count; ++i)
      free(fibs[i]);
    free(fibs);
    return 1;
  }

  uint8_t byte = 0;
  int bits_count = 0;

  /* Проходим по всем сгенерированным строкам и упаковываем биты */
  for (int i = 0; i < count; ++i) {
    size_t len = strlen(fibs[i]);
    for (size_t k = 0; k < len; ++k) {
      char bit = fibs[i][k];

      /* как у тебя раньше: печатаем бит в терминал */
      putchar(bit);

      if (bit == '1') {
        byte |= (uint8_t)(1u << bits_count);
      }
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
    }
    putchar('\n'); /* как в твоём оригинальном коде после каждой строки */
  }

  /* Записать оставшиеся биты (если есть) */
  if (bits_count > 0) {
    if (fwrite(&byte, sizeof(uint8_t), 1, file) != 1) {
      perror("fwrite");
    }
  }

  fclose(file);

  /* очистка */
  for (int i = 0; i < count; ++i)
    free(fibs[i]);
  free(fibs);

  printf("Готово, записано в bits.bin\n");
  return 0;
}
