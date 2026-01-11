#ifndef S21_DECIMAL_H
#define S21_DECIMAL_H

#include <stdio.h>

// от 79,228,162,514,264,337,593,543,950,335 до
// -79,228,162,514,264,337,593,543,950,335 Возможные ошибки
// 0 — OK;
// 1 — число слишком велико или равно бесконечности; -
// 2 — число слишком мало или равно отрицательной бесконечности;
// 3 — деление на 0.

/*сама структура

[0000 0000 0000 0000 0000 0000 0000 0000] [0000 0000 0000 0000 0000 0000 0000
0000] [0000 0000 0000 0000 0000 0000 0000 0000] [0000 0000 0000 0000 0000 0000
0000 0000] 0    1    2    3    4    5    6    7      8    9   10   11   12   13
14   15     16   17   18   19   20   21   22   23     24   25   26   27   28 29
30   31  - номеры цифры [           биты 0-31                   ] [ биты 32-63
] [           биты 64-                   ] [           биты 0-31 ]

bits[0],bits[1] и bits[2] младшие, средние и старшие 32 бита 96 разрядного числа
соответственно. bits[3] состоит из частей: -биты от  0-15 не используются и
равны 0 -биты от 16-23 содержит информацию о степени -биты от 24-30 не
испоьзуются и равные 0 -бит 31 содержит знак числа - 0 = +, 1 = - ;

пример числа где использование ячейки помечены X а не используемы 0
[XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX] [XXXX XXXX XXXX XXXX XXXX XXXX XXXX
XXXX] [XXXX XXXX XXXX XXXX XXXX XXXX XXXX XXXX] [0000 0000 0000 0000 XXXX XXXX
0000 000X] инф. о степени    знак числа 0(+), 1(-)
*/
typedef struct {
  unsigned int bits[4];
} s21_decimal;

#define CodeOK 0
#define CodeBigNumber 1
#define CodeSmallNumber 2
#define CodeDivisionZero 3
#define CodeInvalidData -1

// Арифметические операторы
int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);
int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result);

// Операторы сравнения
int s21_is_less(s21_decimal value_1, s21_decimal value_2);
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_greater(s21_decimal num1, s21_decimal num2);
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2);
int s21_is_equal(s21_decimal num1, s21_decimal num2);
int s21_is_not_equal(s21_decimal, s21_decimal);

// Преобразователи
int s21_from_int_to_decimal(int src, s21_decimal *dst);
int s21_from_float_to_decimal(float src, s21_decimal *dst);
int s21_from_decimal_to_int(s21_decimal src, int *dst);
int s21_from_decimal_to_float(s21_decimal src, float *dst);

// Другие функции
int s21_floor(s21_decimal value, s21_decimal *result);
int s21_round(s21_decimal value, s21_decimal *result);
int s21_truncate(s21_decimal value, s21_decimal *result);
int s21_negate(s21_decimal value, s21_decimal *result);

// Вспомогательные функции
s21_decimal decimal_zero();
int is_zero(s21_decimal value);
int get_bit(s21_decimal number, int bit);
void set_bit(s21_decimal *number, int bit, int sign);
int get_sign(s21_decimal number);
void set_sign(s21_decimal *number, int sing);
int get_scale(s21_decimal number);
void set_scale(s21_decimal *number, int scale);
int align_scale(s21_decimal *number1, s21_decimal *number2);
int mul_by_10(s21_decimal *value);
int div_by_10(s21_decimal *value);
int shift_left(s21_decimal *value, int shift);
// int shift_right(s21_decimal *value, int shift);
int compare_magnitude(s21_decimal value_1, s21_decimal value_2);
void normalize(s21_decimal *value);
int bit_equality(s21_decimal num1, s21_decimal num2, int result);
int s21_is_equal_for_zero(s21_decimal num1, s21_decimal num2);
void s21_bank_rounding(s21_decimal *val, int remainder);

#endif
