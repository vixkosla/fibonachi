// fib_stream.c  — streaming генератор F0..F_{COUNT-1} -> bits.bin (LSB-first)
// Собрать: gcc -O2 -std=c11 -march=native -o fib_stream fib_stream.c

#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Настройки — поменяй при необходимости */
#define COUNT 53            /* сколько F[i] генерировать: i=0..COUNT-1 */
#define OUT_PATH "bits.bin" /* имя выходного файла */
#define VERBOSE 0 /* 0 = не печатать биты в stdout, 1 = печатать (медленно) */

typedef void (*emit_bit_fn)(char bit, void *ctx);

/* Печать unsigned __int128 в десятичной форме */
static void print_u128_unsigned(unsigned __int128 v) {
  if (v == 0) {
    putchar('0');
    return;
  }
  char buf[64];
  int pos = 0;
  while (v > 0) {
    int digit = (int)(v % 10);
    buf[pos++] = '0' + digit;
    v /= 10;
  }
  for (int i = pos - 1; i >= 0; --i)
    putchar(buf[i]);
}

/* Вычислить Fib(n) (Fib(0)=0, Fib(1)=1) в unsigned __int128 */
static unsigned __int128 fib_u128(int n) {
  if (n < 0)
    return 0;
  unsigned __int128 a = 0, b = 1;
  for (int i = 0; i < n; ++i) {
    unsigned __int128 c = a + b;
    a = b;
    b = c;
  }
  return a;
}

/* Итеративный DFS: стримим F(n) = F(n-2) || F(n-1) */
void stream_fib_n(int n, emit_bit_fn emit_bit, void *ctx) {
  int stack_cap = n + 8;
  int *stack = malloc(sizeof(int) * stack_cap);
  if (!stack) {
    perror("malloc");
    exit(1);
  }
  int top = 0;
  stack[top++] = n;
  while (top > 0) {
    int cur = stack[--top];
    if (cur == 0) {
      emit_bit('0', ctx);
    } else if (cur == 1) {
      emit_bit('0', ctx);
      emit_bit('1', ctx);
    } else {
      if (top + 2 >= stack_cap) {
        stack_cap *= 2;
        int *tmp = realloc(stack, sizeof(int) * stack_cap);
        if (!tmp) {
          perror("realloc");
          free(stack);
          exit(1);
        }
        stack = tmp;
      }
      stack[top++] = cur - 1; /* right */
      stack[top++] = cur - 2; /* left (processed first) */
    }
  }
  free(stack);
}

/* Контекст для записи байтов в файл */
typedef struct {
  FILE *out;
  uint8_t byte;
  int bits_count; /* 0..7 */
  int verbose;
} writer_ctx_t;

void emit_bit_to_file(char bit, void *vctx) {
  writer_ctx_t *ctx = (writer_ctx_t *)vctx;
  if (bit == '1')
    ctx->byte |= (uint8_t)(1u << ctx->bits_count);
  ctx->bits_count++;
  if (ctx->verbose)
    putchar(bit);
  if (ctx->bits_count == 8) {
    if (fwrite(&ctx->byte, sizeof(uint8_t), 1, ctx->out) != 1) {
      perror("fwrite");
      exit(1);
    }
    ctx->bits_count = 0;
    ctx->byte = 0;
  }
}

int main(void) {
  /* предварительный подсчёт ожидаемого числа бит: S = Fib(COUNT+3) - 2 */
  unsigned __int128 S = 0;
  if (COUNT >= 0) {
    unsigned __int128 fib_n3 = fib_u128(COUNT + 3);
    if (fib_n3 < 2)
      S = 0;
    else
      S = fib_n3 - 2;
  }
  unsigned __int128 expected_bytes = (S + 7) / 8;

  printf("COUNT = %d\n", COUNT);
  printf("Expected total bits: ");
  print_u128_unsigned(S);
  printf("\nExpected bytes (ceil(bits/8)): ");
  print_u128_unsigned(expected_bytes);
  printf("\nOutput file: %s\n", OUT_PATH);

  /* предупреждение о размере */
  if (expected_bytes > (unsigned __int128)0) {
    printf("Proceeding to generate. Ensure you have enough disk space.\n");
  }

  FILE *out = fopen(OUT_PATH, "wb");
  if (!out) {
    perror("fopen");
    return 1;
  }
  setvbuf(out, NULL, _IOFBF, 1 << 20);

  writer_ctx_t ctx = {
      .out = out, .byte = 0, .bits_count = 0, .verbose = VERBOSE};

  for (int i = 0; i < COUNT; ++i) {
    /* при VERBOSE = 1 печатает биты в stdout; по завершении F(i) печатаем
     * перенос строки */
    stream_fib_n(i, emit_bit_to_file, &ctx);
    if (VERBOSE)
      putchar('\n');
  }

  if (ctx.bits_count > 0) {
    if (fwrite(&ctx.byte, sizeof(uint8_t), 1, out) != 1) {
      perror("fwrite");
      fclose(out);
      return 1;
    }
  }

  fclose(out);
  if (VERBOSE)
    fflush(stdout);
  printf("Done. Wrote bits to %s\n", OUT_PATH);
  return 0;
}
