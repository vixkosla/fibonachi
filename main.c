#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char fib_char(int count, int k);
int fib_length(int count);

int main() {

  printf("Hello world!");

  char a1[1024] = "0";
  char a2[1024] = "01";

  char a3[1024] = "";

  int count = 14;

  for (int i = 0; i < count; i++) {
    a3[0] = '\0';

    strcat(a3, a1);
    strcat(a3, a2);

    strcpy(a1, a2);
    strcpy(a2, a3);

    printf("%s\n", a3);
  }

  FILE *file = fopen("bits.bin", "w");

  if (file == NULL) {
    perror("Ошибка при подключении файла");
    return 1;
  }

  // char *result = fib(4);
  unsigned char byte = 0;
  int bits_count = 0;

  // printf("Fibonachi :\n%s", result);

  for (int i = 0; i < count; i++) {
    int kMax = fib_length(i);
    for (int k = 0; k < kMax; k++) {
      char bit = fib_char(i, k);

      if (bits_count == 8) {
        fwrite(&byte, sizeof(unsigned char), 1, file);
        bits_count = 0;
        byte = byte & 0;
      } else {
        if (bit == '1') {
          byte = byte | (1 << bits_count);
        }
      }

      bits_count++;
      printf("%c", bit);
    }

    printf("\n");
  }

  // free(result);
  fclose(file);

  return 0;
}

int fib_length(int count) {
  if (count == 0) {
    return 1;
  } else if (count == 1) {
    return 2;
  }

  return fib_length(count - 2) + fib_length(count - 1);
}

char fib_char(int count, int k) {
  int result = 0;

  if (count == 0) {

    if (k == 0)
      return '0';
    return '\0';
  } else if (count == 1) {

    if (k == 0)
      return '0';
    if (k == 1)
      return '1';

    return '\0';
  } else {

    // char str1[1024] =
    int fib_length_left = fib_length(count - 2);

    // char *ptr2 = fib(count - 2);
    // char *ptr1 = fib(count - 1);

    if (k < fib_length_left) {
      return fib_char(count - 2, k);
    } else {
      return fib_char(count - 1, k - fib_length_left);
    }

    // int size_of = strlen(ptr2) + strlen(ptr1);

    // char *str1 = malloc(sizeof(char) * size_of);

    // strcpy(str1, ptr2);
    // strcat(str1, ptr1);

    // free(ptr1);
    // free(ptr2);

    // return str1;
  }
}