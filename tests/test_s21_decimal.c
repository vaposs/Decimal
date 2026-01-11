#include <check.h>
#include <stdio.h>

#include "../src/s21_decimal.h"

#define DEC(bits0, bits1, bits2, scale, sign)                 \
  (s21_decimal) {                                             \
    { bits0, bits1, bits2, ((scale) << 16) | ((sign) << 31) } \
  }

//////// Тесты для s21_add ////////
// 0 + 0 = 0
START_TEST(add_zero_zero) {
  s21_decimal v1 = DEC(0, 0, 0, 0, 0);
  s21_decimal v2 = DEC(0, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_add(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 0);
  ck_assert_uint_eq(res.bits[1], 0);
  ck_assert_uint_eq(res.bits[2], 0);
  ck_assert_uint_eq(res.bits[3], 0);
}
END_TEST

// Простое сложение без scale
START_TEST(add_simple_int) {
  s21_decimal v1 = DEC(5, 0, 0, 0, 0);
  s21_decimal v2 = DEC(7, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 12);
  ck_assert_uint_eq(get_scale(res), 0);
  ck_assert_uint_eq(get_sign(res), 0);
}
END_TEST

// Сложение с одинаковым scale
START_TEST(add_same_scale) {
  s21_decimal v1 = DEC(125, 0, 0, 2, 0);  // 1.25
  s21_decimal v2 = DEC(275, 0, 0, 2, 0);  // 2.75
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 400);
  ck_assert_uint_eq(get_scale(res), 2);
}
END_TEST

// Сложение с разным scale
START_TEST(add_diff_scale) {
  s21_decimal v1 = DEC(12, 0, 0, 1, 0);  // 1.2
  s21_decimal v2 = DEC(3, 0, 0, 2, 0);   // 0.03
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 123);
  ck_assert_uint_eq(get_scale(res), 2);
}
END_TEST

// Сложение с разными знаками
START_TEST(add_pos_neg) {
  s21_decimal v1 = DEC(10, 0, 0, 0, 0);
  s21_decimal v2 = DEC(3, 0, 0, 0, 1);
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 7);
  ck_assert_uint_eq(get_sign(res), 0);
}
END_TEST

// Перенос в bits[1]
START_TEST(add_overflow_32bit) {
  s21_decimal v1 = DEC(0xFFFFFFFF, 0, 0, 0, 0);
  s21_decimal v2 = DEC(1, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 0);
  ck_assert_uint_eq(res.bits[1], 1);
}
END_TEST

// Переполнение, максимальное 96-битное число
START_TEST(add_overflow_max) {
  //
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal v2 = DEC(1, 0, 0, 0, 0);
  s21_decimal res = {{0}};
  int code = s21_add(v1, v2, &res);

  ck_assert_int_eq(code, CodeBigNumber);
}
END_TEST

// Переполнение MAX + MAX
START_TEST(add_overflow_max_plus_max) {
  s21_decimal max1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal max2 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_add(max1, max2, &res);

  ck_assert_int_eq(code, CodeBigNumber);
}
END_TEST

// Переполнение -MAX + -1
START_TEST(add_overflow_negative) {
  s21_decimal min = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
  s21_decimal one = DEC(1, 0, 0, 0, 1);
  s21_decimal res = {{0}};

  int code = s21_add(min, one, &res);

  ck_assert_int_eq(code, CodeSmallNumber);
}
END_TEST

// Переполнение (-MAX) + (-MAX)
START_TEST(add_overflow_min_plus_min) {
  s21_decimal min1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
  s21_decimal min2 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
  s21_decimal res = {{0}};

  int code = s21_add(min1, min2, &res);

  ck_assert_int_eq(code, CodeSmallNumber);
}
END_TEST

/////// Тесты для s21_sub ///////
// 0 − 0 = 0
START_TEST(sub_zero_zero) {
  s21_decimal v1 = DEC(0, 0, 0, 0, 0);
  s21_decimal v2 = DEC(0, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  s21_sub(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 0);
  ck_assert_uint_eq(res.bits[3], 0);
}
END_TEST

// Простое вычитание
START_TEST(sub_simple) {
  s21_decimal v1 = DEC(10, 0, 0, 0, 0);
  s21_decimal v2 = DEC(3, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  s21_sub(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 7);
}
END_TEST

// Отрицательный результат
START_TEST(sub_negative_result) {
  s21_decimal v1 = DEC(3, 0, 0, 0, 0);
  s21_decimal v2 = DEC(10, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  s21_sub(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 7);
  ck_assert_uint_eq(get_sign(res), 1);
}
END_TEST

// Вычитание дробей
START_TEST(sub_scale) {
  s21_decimal v1 = DEC(250, 0, 0, 2, 0);
  s21_decimal v2 = DEC(125, 0, 0, 2, 0);
  s21_decimal res = {{0}};

  s21_sub(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 125);
  ck_assert_uint_eq(get_scale(res), 2);
}
END_TEST

// Вычитание с разными знаками
START_TEST(sub_neg) {
  s21_decimal v1 = DEC(5, 0, 0, 0, 0);
  s21_decimal v2 = DEC(5, 0, 0, 0, 1);
  s21_decimal res = {{0}};

  s21_sub(v1, v2, &res);

  ck_assert_uint_eq(res.bits[0], 10);
  ck_assert_uint_eq(get_sign(res), 0);
}
END_TEST

// Переполнение MAX − (-1)
START_TEST(sub_overflow_positive) {
  s21_decimal max = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal minus_one = DEC(1, 0, 0, 0, 1);
  s21_decimal res = {{0}};

  int code = s21_sub(max, minus_one, &res);

  ck_assert_int_eq(code, CodeBigNumber);
}
END_TEST

// Переполнение -MAX − 1
START_TEST(sub_overflow_negative) {
  s21_decimal min = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
  s21_decimal one = DEC(1, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_sub(min, one, &res);

  ck_assert_int_eq(code, CodeSmallNumber);
}
END_TEST

// 0 − MAX
START_TEST(sub_zero_minus_max_ok) {
  s21_decimal zero = DEC(0, 0, 0, 0, 0);
  s21_decimal max = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_sub(zero, max, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(get_sign(res), 1);
  ck_assert_uint_eq(res.bits[0], 0xFFFFFFFF);
  ck_assert_uint_eq(res.bits[1], 0xFFFFFFFF);
  ck_assert_uint_eq(res.bits[2], 0xFFFFFFFF);
}
END_TEST

// 0 − (-MAX)
START_TEST(sub_zero_minus_min_ok) {
  s21_decimal zero = DEC(0, 0, 0, 0, 0);
  s21_decimal min = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 1);
  s21_decimal res = {{0}};

  int code = s21_sub(zero, min, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(get_sign(res), 0);
  ck_assert_uint_eq(res.bits[0], 0xFFFFFFFF);
  ck_assert_uint_eq(res.bits[1], 0xFFFFFFFF);
  ck_assert_uint_eq(res.bits[2], 0xFFFFFFFF);
}
END_TEST

/////// Тесты для s21_mul //////
// 1. Простой тест: 10 * 10 = 100
START_TEST(mul_basic_int) {
  s21_decimal v1 = DEC(10, 0, 0, 0, 0);
  s21_decimal v2 = DEC(10, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_mul(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 100);
  ck_assert_int_eq(get_scale(res), 0);
  ck_assert_int_eq(get_sign(res), 0);
}
END_TEST

// 2. Тест с дробями: 2.5 * 2.5 = 6.25
// 25 (sc 1) * 25 (sc 1) -> 625 (sc 2)
START_TEST(mul_fractions) {
  s21_decimal v1 = DEC(25, 0, 0, 1, 0);  // 2.5
  s21_decimal v2 = DEC(25, 0, 0, 1, 0);  // 2.5
  s21_decimal res = {{0}};

  int code = s21_mul(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 625);
  ck_assert_int_eq(get_scale(res), 2);
  ck_assert_int_eq(get_sign(res), 0);
}
END_TEST

// 3. Отрицательное число: -12 * 4 = -48
START_TEST(mul_negative) {
  s21_decimal v1 = DEC(12, 0, 0, 0, 1);  // -12
  s21_decimal v2 = DEC(4, 0, 0, 0, 0);   // 4
  s21_decimal res = {{0}};

  int code = s21_mul(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 48);
  ck_assert_int_eq(get_sign(res), 1);  // Ожидаем минус
}
END_TEST

// 4. Умножение на ноль: 555 * 0 = 0
START_TEST(mul_by_zero) {
  s21_decimal v1 = DEC(555, 0, 0, 0, 0);
  s21_decimal v2 = decimal_zero();
  s21_decimal res = {{0}};

  int code = s21_mul(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_int_eq(is_zero(res), 1);
  ck_assert_int_eq(get_scale(res), 0);  // Ноль обычно нормализуется в scale 0
}
END_TEST

// 5. Переполнение (Big Number)
// Максимальное число * 2 = Ошибка
START_TEST(mul_overflow) {
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);  // MAX
  s21_decimal v2 = DEC(2, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_mul(v1, v2, &res);

  ck_assert_int_eq(code, CodeBigNumber);
}
END_TEST

// 6. Сложная дробь: 0.05 * 0.05 = 0.0025
// 5 (sc 2) * 5 (sc 2) -> 25 (sc 4)
START_TEST(mul_small_fractions) {
  s21_decimal v1 = DEC(5, 0, 0, 2, 0);
  s21_decimal v2 = DEC(5, 0, 0, 2, 0);
  s21_decimal res = {{0}};

  int code = s21_mul(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 25);
  ck_assert_int_eq(get_scale(res), 4);
}
END_TEST

/////// Тесты s21_div ///////
// 1. Простое деление: 100 / 2 = 50
START_TEST(div_basic) {
  s21_decimal v1 = DEC(100, 0, 0, 0, 0);
  s21_decimal v2 = DEC(2, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_div(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 50);
  ck_assert_int_eq(get_scale(res), 0);
}
END_TEST

// 2. Деление на ноль: 10 / 0 -> Ошибка
START_TEST(div_by_zero) {
  s21_decimal v1 = DEC(10, 0, 0, 0, 0);
  s21_decimal v2 = decimal_zero();
  s21_decimal res = {{0}};

  int code = s21_div(v1, v2, &res);

  ck_assert_int_eq(code, CodeDivisionZero);
}
END_TEST

// 3. Получение дробной части: 5 / 2 = 2.5
// Вход: 5(sc0), 2(sc0). Ожидаем: 25(sc1) или 5(sc1) * 5? Нет, 25 и scale 1
START_TEST(div_fraction_result) {
  s21_decimal v1 = DEC(5, 0, 0, 0, 0);
  s21_decimal v2 = DEC(2, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_div(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 25);
  ck_assert_int_eq(get_scale(res), 1);
}
END_TEST

// 4. Отрицательное деление: -10 / 2 = -5
START_TEST(div_negative) {
  s21_decimal v1 = DEC(10, 0, 0, 0, 1);  // -10
  s21_decimal v2 = DEC(2, 0, 0, 0, 0);   // 2
  s21_decimal res = {{0}};

  int code = s21_div(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 5);
  ck_assert_int_eq(get_sign(res), 1);  // Знак минус
}
END_TEST

// 5. Деление дроби на дробь: 0.5 / 0.1 = 5
// 5 (sc 1) / 1 (sc 1) = 5 (sc 0)
START_TEST(div_fractions) {
  s21_decimal v1 = DEC(5, 0, 0, 1, 0);  // 0.5
  s21_decimal v2 = DEC(1, 0, 0, 1, 0);  // 0.1
  s21_decimal res = {{0}};

  int code = s21_div(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 5);
  ck_assert_int_eq(get_scale(res), 0);
}
END_TEST

// 6. Сложный случай (много цифр после запятой): 1 / 4 = 0.25
START_TEST(div_one_by_four) {
  s21_decimal v1 = DEC(1, 0, 0, 0, 0);
  s21_decimal v2 = DEC(4, 0, 0, 0, 0);
  s21_decimal res = {{0}};

  int code = s21_div(v1, v2, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 25);
  ck_assert_int_eq(get_scale(res), 2);  // 0.25
}
END_TEST

////// Other functions tests ///////
// Обычные целые числа
START_TEST(from_float_int_basic) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(123.0f, &d);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(d.bits[0], 123);
  ck_assert_int_eq(get_scale(d), 0);
  ck_assert_int_eq(get_sign(d), 0);
}
END_TEST

// Отрицательное целое
START_TEST(from_float_negative_int) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(-456.0f, &d);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(d.bits[0], 456);
  ck_assert_int_eq(get_scale(d), 0);
  ck_assert_int_eq(get_sign(d), 1);
}
END_TEST

// Простая дробь
START_TEST(from_float_fraction) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(12.34f, &d);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(d.bits[0], 1234);
  ck_assert_int_eq(get_scale(d), 2);
}
END_TEST

// Дробь с хвостовыми нулями
START_TEST(from_float_trailing_zeros) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(10.5000f, &d);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(d.bits[0], 105);
  ck_assert_int_eq(get_scale(d), 1);
}
END_TEST

// Округление до 7 значащих цифр
START_TEST(from_float_rounding) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(1.12345678f, &d);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(d.bits[0], 1123457);
  ck_assert_int_eq(get_scale(d), 6);
}
END_TEST

// Малое число (ошибка)
START_TEST(from_float_too_small) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(1e-29f, &d);

  ck_assert_int_eq(code, CodeSmallNumber);
  ck_assert(is_zero(d));
}
END_TEST

// Бесконечность
START_TEST(from_float_inf) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(INFINITY, &d);

  ck_assert_int_eq(code, CodeBigNumber);
}
END_TEST

// NaN
START_TEST(from_float_nan) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(NAN, &d);

  ck_assert_int_eq(code, CodeBigNumber);
}
END_TEST

// Ноль
START_TEST(from_float_zero) {
  s21_decimal d;
  int code = s21_from_float_to_decimal(0.0f, &d);

  ck_assert_int_eq(code, CodeOK);
  ck_assert(is_zero(d));
}
END_TEST

// Целое число
START_TEST(from_decimal_to_float_int) {
  s21_decimal d = DEC(123, 0, 0, 0, 0);
  float f = 0;

  int code = s21_from_decimal_to_float(d, &f);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_float_eq_tol(f, 123.0f, 1e-6);
}
END_TEST

// Отрицательное число
START_TEST(from_decimal_to_float_negative) {
  s21_decimal d = DEC(456, 0, 0, 0, 1);
  float f = 0;

  int code = s21_from_decimal_to_float(d, &f);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_float_eq_tol(f, -456.0f, 1e-6);
}
END_TEST

// Дробное число
START_TEST(from_decimal_to_float_fraction) {
  s21_decimal d = DEC(1234, 0, 0, 2, 0);
  float f = 0;

  int code = s21_from_decimal_to_float(d, &f);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_float_eq_tol(f, 12.34f, 1e-6);
}
END_TEST

// Большое значение
START_TEST(from_decimal_to_float_big) {
  s21_decimal d = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0);
  float f = 0;

  int code = s21_from_decimal_to_float(d, &f);

  ck_assert_int_eq(code, CodeOK);
}
END_TEST

///////////////////////////////////////////////
//////// s21_from_int_to_decimal /////////////
/////////////////////////////////////////////

START_TEST(test_from_int_1) {
  s21_decimal dst;
  int src = -1235;
  s21_decimal result = {
      .bits = {
          1235,       // мантисса = 1235
          0,          // bits[1] = 0
          0,          // bits[2] = 0
          0x80000000  // bits[3]:
                      //   0x00000000 → scale = 0
                      //   0x80000000 → sign bit (бит 31) = 1 → отрицательное
      }};
  int status = s21_from_int_to_decimal(src, &dst);
  for (int i = 0; i < 4; i++) {
    ck_assert_int_eq(dst.bits[i], result.bits[i]);
  }
  ck_assert_int_eq(status, 0);
}
END_TEST

START_TEST(test_from_int_2) {
  s21_decimal dst;
  int src = 1235;
  s21_decimal result = {
      .bits = {
          1235,       // мантисса = 1235
          0,          // bits[1] = 0
          0,          // bits[2] = 0
          0x00000000  // bits[3]:
                      //   0x00000000 → scale = 0
                      //   0x80000000 → sign bit (бит 31) = 1 → отрицательное
      }};
  s21_from_int_to_decimal(src, &dst);
  for (int i = 0; i < 4; i++) {
    ck_assert_int_eq(dst.bits[i], result.bits[i]);
  }
}
END_TEST

START_TEST(test_from_int_3) {
  int src = -1235;
  int status = s21_from_int_to_decimal(src, NULL);
  ck_assert_int_eq(status, CodeInvalidData);
}
END_TEST

START_TEST(test_from_int_4) {
  s21_decimal dst;
  int src = 2147483647;
  s21_decimal result = {
      .bits = {
          2147483647U,  // bits[0] — мантисса (лучше с U для unsigned)
          0,            // bits[1]
          0,            // bits[2]
          0             // bits[3]: scale = 0, sign = 0
      }};
  s21_from_int_to_decimal(src, &dst);
  for (int i = 0; i < 4; i++) {
    ck_assert_int_eq(dst.bits[i], result.bits[i]);
  }
}
END_TEST

///////////////////////////////////////////////
//////// s21_from_decimal_to_int /////////////
/////////////////////////////////////////////

START_TEST(test_to_int_1) {
  int dst;
  s21_decimal src = {
      .bits = {
          2147483647U,  // bits[0] — мантисса (лучше с U для unsigned)
          0,            // bits[1]
          0,            // bits[2]
          0             // bits[3]: scale = 0, sign = 0
      }};
  s21_from_decimal_to_int(src, &dst);
  ck_assert_int_eq(dst, 2147483647);
}
END_TEST

START_TEST(test_to_int_2) {
  s21_decimal src = {
      .bits = {
          2147483647U,  // bits[0] — мантисса (лучше с U для unsigned)
          0,            // bits[1]
          0,            // bits[2]
          0             // bits[3]: scale = 0, sign = 0
      }};
  int status = s21_from_decimal_to_int(src, NULL);
  ck_assert_int_eq(status, CodeInvalidData);
}
END_TEST

START_TEST(test_to_int_3) {
  int dst;
  s21_decimal src = {
      .bits = {
          1235,       // мантисса = 1235
          0,          // bits[1] = 0
          0,          // bits[2] = 0
          0x80010000  // bits[3]:
                      //   0x00010000 → scale = 1
                      //   0x80000000 → sign bit (бит 31) = 1 → отрицательное
      }};
  s21_from_decimal_to_int(src, &dst);
  ck_assert_int_eq(dst, -123);
}
END_TEST

START_TEST(test_to_int_4) {
  int dst;
  s21_decimal src = {
      .bits = {
          1235,       // мантисса = 1235
          0,          // bits[1] = 0
          0,          // bits[2] = 0
          0x80000000  // bits[3]:
                      //   0x00000000 → scale = 0
                      //   0x80000000 → sign bit (бит 31) = 1 → отрицательное
      }};
  int status = s21_from_decimal_to_int(src, &dst);
  ck_assert_int_eq(status, 0);
}
END_TEST

START_TEST(test_to_int_5) {
  int dst;
  s21_decimal src = {.bits = {
                         12344,  // 1234.4 * 10^1
                         0, 0,
                         0x80010000  // scale=1, sign=1
                     }};
  s21_from_decimal_to_int(src, &dst);
  ck_assert_int_eq(dst, -1234);
}
END_TEST

START_TEST(test_to_int_6) {
  int dst;
  s21_decimal src = {.bits = {
                         2147483648U,  // мантисса = 2 147 483 648
                         0, 0, 0       // scale = 0, sign = 0 → +2147483648
                     }};

  int status = s21_from_decimal_to_int(src, &dst);
  ck_assert_int_eq(status, 1);
}
END_TEST

///////////////////////////////////////
///// Comparison Operators TESTS /////
//////////////////////////////////////
START_TEST(equal_positive) {
  s21_decimal number = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};

  ck_assert_int_eq(s21_is_less(number, number), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number, number), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number, number), 1);
}
END_TEST

START_TEST(greater_first_positive) {
  s21_decimal number1 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00020000  // bits[3]: 00000000 00000010 00000000 00000000
                      // Число в 10: 1,234,567.89
                      // Число в 2:
                      // 100101101011010000111.111000110101001111101111
      }};
  s21_decimal number2 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};
  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 0);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(less_first_positive) {
  s21_decimal number1 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};
  s21_decimal number2 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00020000  // bits[3]: 00000000 00000010 00000000 00000000
                      // Число в 10: 1,234,567.89
                      // Число в 2:
                      // 100101101011010000111.111000110101001111101111
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

START_TEST(equal_negative) {
  s21_decimal number = {
      .bits = {
          42,         // bits[0]: 00000000 00000000 00000000 00101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -42
                      // Число в 2: -101010
      }};

  ck_assert_int_eq(s21_is_less(number, number), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number, number), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number, number), 1);
}
END_TEST

START_TEST(greater_first_negative) {
  s21_decimal number1 = {
      .bits = {
          42,         // bits[0]: 00000000 00000000 00000000 00101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -42
                      // Число в 2: -101010
      }};
  s21_decimal number2 = {
      .bits = {
          0xFFFFFFFF,  // bits[0]: 11111111 11111111 11111111 11111111
          0x0000FFFF,  // bits[1]: 00000000 00000000 11111111 11111111
          12345,       // bits[2]: 00000000 00000000 00110000 00111001
          0x80050000   // bits[3]: 10000000 00000101 00000000 00000000
                       // Число в 10: ≈ -2,275,586,817,417,451.11655
                       // Число в 2:
          // -1000000100110001001111010001001001100000101010101101010110111.00011101101
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 0);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(less_first_negative) {
  s21_decimal number1 = {
      .bits = {
          0xFFFFFFFF,  // bits[0]: 11111111 11111111 11111111 11111111
          0x0000FFFF,  // bits[1]: 00000000 00000000 11111111 11111111
          12345,       // bits[2]: 00000000 00000000 00110000 00111001
          0x80050000   // bits[3]: 10000000 00000101 00000000 00000000
                       // Число в 10: ≈ -2,275,586,817,417,451.11655
                       // Число в 2:
          // -1000000100110001001111010001001001100000101010101101010110111.00011101101
      }};
  s21_decimal number2 = {
      .bits = {
          42,         // bits[0]: 00000000 00000000 00000000 00101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -42
                      // Число в 2: -101010
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

START_TEST(positive_vs_negative) {
  s21_decimal number1 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};
  s21_decimal number2 = {
      .bits = {
          42,         // bits[0]: 00000000 00000000 00000000 00101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -42
                      // Число в 2: -101010
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 0);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(negative_vs_positive) {
  s21_decimal number1 = {
      .bits = {
          42,         // bits[0]: 00000000 00000000 00000000 00101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -42
                      // Число в 2: -101010
      }};
  s21_decimal number2 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

START_TEST(positive_vs_zero) {
  s21_decimal number1 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};
  s21_decimal number2 = {.bits = {
                             0,  // bits[0]: 00000000 00000000 00000000 00000000
                             0,  // bits[1]: 00000000 00000000 00000000 00000000
                             0,  // bits[2]: 00000000 00000000 00000000 00000000
                             0x00000000  // bits[3]: 00000000 00000000 00000000
                                         // 00000000 Число в 10: 0 Число в 2: 0
                         }};

  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 0);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(negative_vs_zero) {
  s21_decimal number1 = {
      .bits = {
          42,         // bits[0]: 00000000 00000000 00000000 00101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -42
                      // Число в 2: -101010
      }};
  s21_decimal number2 = {.bits = {
                             0,  // bits[0]: 00000000 00000000 00000000 00000000
                             0,  // bits[1]: 00000000 00000000 00000000 00000000
                             0,  // bits[2]: 00000000 00000000 00000000 00000000
                             0x00000000  // bits[3]: 00000000 00000000 00000000
                                         // 00000000 Число в 10: 0 Число в 2: 0
                         }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

START_TEST(zero_equal) {
  s21_decimal number = {.bits = {
                            0,  // bits[0]: 00000000 00000000 00000000 00000000
                            0,  // bits[1]: 00000000 00000000 00000000 00000000
                            0,  // bits[2]: 00000000 00000000 00000000 00000000
                            0x00000000  // bits[3]: 00000000 00000000 00000000
                                        // 00000000 Число в 10: 0 Число в 2: 0
                        }};

  ck_assert_int_eq(s21_is_less(number, number), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number, number), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number, number), 1);
}
END_TEST

START_TEST(same_value_different_scale) {
  s21_decimal number1 = {
      .bits = {
          234,        // bits[0]: 00000000 00000000 00000000 11101010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 23.4
                      // Число в 2: 11101010
      }};
  s21_decimal number2 = {
      .bits = {
          2340,       // bits[0]: 00000000 00000000 00001001 00100100
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00020000  // bits[3]: 00000000 00000010 00000000 00000000
                      // Число в 10: 23.40
                      // Число в 2: 100100100100
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(different_value_different_scale) {
  s21_decimal number1 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00020000  // bits[3]: 00000000 00000010 00000000 00000000
                      // Число в 10: 1,234,567.89
                      // Число в 2:
                      // 100101101011010000111.111000110101001111101111
      }};
  s21_decimal number2 = {
      .bits = {
          0x0000FFFF,  // bits[0]: 00000000 00000000 11111111 11111111
          0x0000000A,  // bits[1]: 00000000 00000000 00000000 00001010
          0,           // bits[2]: 00000000 00000000 00000000 00000000
          0x00030000   // bits[3]: 00000000 00000011 00000000 00000000
                       // Число в 10: 42,949,738.495
          // Число в 2: 10100011110101110000101010111111.0111110001101010
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

START_TEST(large_scale_comparison) {
  s21_decimal number1 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00060000  // bits[3]: 00000000 00000110 00000000 00000000
                      // Число в 10: 123.456789
      }};
  s21_decimal number2 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00030000  // bits[3]: 00000000 00000011 00000000 00000000
                      // Число в 10: 123,456.789
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

START_TEST(max_values) {
  s21_decimal number = {
      .bits = {
          0xFFFFFFFF,  // bits[0]: 11111111 11111111 11111111 11111111
          0xFFFFFFFF,  // bits[1]: 11111111 11111111 11111111 11111111
          0xFFFFFFFF,  // bits[2]: 11111111 11111111 11111111 11111111
          0x00000000   // bits[3]: 00000000 00000000 00000000 00000000
                       // Число в 10: ~7.92 × 10²⁸
      }};

  ck_assert_int_eq(s21_is_less(number, number), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number, number), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number, number), 1);
}
END_TEST

START_TEST(max_positive_vs_max_negative) {
  s21_decimal number1 = {
      .bits = {
          0xFFFFFFFF,  // bits[0]: 11111111 11111111 11111111 11111111
          0xFFFFFFFF,  // bits[1]: 11111111 11111111 11111111 11111111
          0xFFFFFFFF,  // bits[2]: 11111111 11111111 11111111 11111111
          0x00000000   // bits[3]: 00000000 00000000 00000000 00000000
                       // Число в 10: ~7.92 × 10²⁸
      }};
  s21_decimal number2 = {
      .bits = {
          0xFFFFFFFF,  // bits[0]: 11111111 11111111 11111111 11111111
          0xFFFFFFFF,  // bits[1]: 11111111 11111111 11111111 11111111
          0xFFFFFFFF,  // bits[2]: 11111111 11111111 11111111 11111111
          0x80000000   // bits[3]: 10000000 00000000 00000000 00000000
                       // Число в 10: ~ -7.92 × 10²⁸
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 0);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(different_high_bits) {
  s21_decimal number1 = {.bits = {
                             0,  // bits[0]: 00000000 00000000 00000000 00000000
                             1,  // bits[1]: 00000000 00000000 00000000 00000001
                             0,  // bits[2]: 00000000 00000000 00000000 00000000
                             0   // bits[3]: 00000000 00000000 00000000 00000000
                                 // Число в 10: 4,294,967,296
                         }};
  s21_decimal number2 = {
      .bits = {
          0xFFFFFFFF,  // bits[0]: 1111111 11111111 11111111 11111111
          0,           // bits[1]: 00000000 00000000 00000000 00000001
          0,           // bits[2]: 00000000 00000000 00000000 00000000
          0            // bits[3]: 00000000 00000000 00000000 00000000
                       // Число в 10: 4,294,967,295
      }};

  ck_assert_int_eq(s21_is_less(number1, number2), 0);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 0);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 1);
}
END_TEST

START_TEST(near_zero) {
  s21_decimal number1 = {.bits = {
                             1,  // bits[0]: 00000000 00000000 00000000 00000001
                             0,  // bits[1]: 00000000 00000000 00000000 00000000
                             0,  // bits[2]: 00000000 00000000 00000000 00000000
                             0x001C0000  // bits[3]: 00000000 00011100 00000000
                                         // 00000000 Число в 10: 1e-28
                         }};
  s21_decimal number2 = {.bits = {
                             2,  // bits[0]: 00000000 00000000 00000000 00000010
                             0,  // bits[1]: 00000000 00000000 00000000 00000000
                             0,  // bits[2]: 00000000 00000000 00000000 00000000
                             0x001C0000  // bits[3]: 00000000 00011100 00000000
                                         // 00000000 Число в 10: 2e-28
                         }};

  ck_assert_int_eq(s21_is_less(number1, number2), 1);
  ck_assert_int_eq(s21_is_less_or_equal(number1, number2), 1);
  ck_assert_int_eq(s21_is_greater_or_equal(number1, number2), 0);
}
END_TEST

///////////////////////////////////////
///////// s21_truncate TESTS //////////
//////////////////////////////////////
START_TEST(positive_with_fraction) {
  s21_decimal number = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00040000  // bits[3]: 00000000 00000010 00000000 00000000
                      // Число в 10: 1,234,5.6789
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          12345,      // bits[0]: 00000000 00000000 00110000 00111001
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00000000  // bits[3]: 00000000 00000000 00000000 00000000
                      // Число в 10: 12,345
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal(result, expected));
}
END_TEST

START_TEST(negative_with_fraction) {
  s21_decimal number = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80040000  // bits[3]: 10000000 00000010 00000000 00000000
                      // Число в 10: -1,234,5.6789
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          12345,      // bits[0]: 00000000 00000000 00110000 00111001
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -12,345
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal(result, expected));
}
END_TEST

START_TEST(zero_with_scale) {
  s21_decimal number = {.bits = {
                            0,  // bits[0]: 00000000 00000000 00000000 00000000
                            0,  // bits[1]: 00000000 00000000 00000000 00000000
                            0,  // bits[2]: 00000000 00000000 00000000 00000000
                            0x00040000  // bits[3]: 10000000 00000010 00000000
                                        // 00000000 Число в 10: 0.0000
                        }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          0,  // bits[0]: 00000000 00000000 00000000 00000000
          0,  // bits[1]: 00000000 00000000 00000000 00000000
          0,  // bits[2]: 00000000 00000000 00000000 00000000
          0   // bits[3]: 00000000 00000000 00000000 00000000
              // 00000000 Число в 10: 0
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal_for_zero(result, expected));
}
END_TEST

START_TEST(integer) {
  s21_decimal number = {
      .bits = {
          123456,     // bits[0]: 00000000 00000001 11100010 01000000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00000000  // bits[3]: 00000000 00000000 00000000 00000000
                      // Число в 10: 123,456
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          123456,     // bits[0]: 00000000 00000001 11100010 01000000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00000000  // bits[3]: 00000000 00000000 00000000 00000000
                      // Число в 10: 123,456
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal(result, expected));
}
END_TEST

START_TEST(trailing_zeros) {
  s21_decimal number = {
      .bits = {
          12300,      // bits[0]: 00000000 00000000 00110000 00001100
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00030000  // bits[3]: 00000000 00000011 00000000 00000000
                      // Число в 10: 12.300
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          12,         // bits[0]: 00000000 00000000 00000000 00001100
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00000000  // bits[3]: 00000000 00000000 00000000 00000000
                      // Число в 10: 12
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal(result, expected));
}
END_TEST

START_TEST(all_zeros_fraction) {
  s21_decimal number = {
      .bits = {
          1234000,    // bits[0]: 00000000 00010010 11010100 01010000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00030000  // bits[3]: 00000000 00000011 00000000 00000000
                      // Число в 10: 1,234.000
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          1234,       // bits[0]: 00000000 00000000 00000100 11010010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00000000  // bits[3]: 00000000 00000000 00000000 00000000
                      // Число в 10: 1,234
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal(result, expected));
}
END_TEST

START_TEST(less_than_one) {
  s21_decimal number = {
      .bits = {
          1234,       // bits[0]: 00000000 00000000 00000100 11010010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00040000  // bits[3]: 00000000 00000100 00000000 00000000
                      // Число в 10: 0.1234
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          0,  // bits[0]: 00000000 00000000 00000000 00000000
          0,  // bits[1]: 00000000 00000000 00000000 00000000
          0,  // bits[2]: 00000000 00000000 00000000 00000000
          0   // bits[3]: 00000000 00000000 00000000 00000000
              // Число в 10: 0
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal_for_zero(result, expected));
}
END_TEST

START_TEST(small_negative) {
  s21_decimal number = {
      .bits = {
          1234,       // bits[0]: 00000000 00000000 00000100 11010010
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80040000  // bits[3]: 10000000 00000100 00000000 00000000
                      // Число в 10: -0.1234
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          0,          // bits[0]: 00000000 00000000 00000000 00000000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80000000  // bits[3]: 10000000 00000000 00000000 00000000
                      // Число в 10: -0
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal_for_zero(result, expected));
}
END_TEST

START_TEST(large_number) {
  s21_decimal number = {
      .bits = {
          4294967295,  // bits[0]: 11111111 11111111 11111111 11111111
          4294967295,  // bits[1]: 11111111 11111111 11111111 11111111
          444294967,   // bits[2]: 00011010 01111011 01100111 00110111
          0x00010000   // bits[3]: 00000000 00000001 00000000 00000000
                       // Число в 10: ≈ 8.192 × 10²⁶
      }};
  s21_decimal result;
  s21_decimal expected = {
      .bits = {
          3435973836,  // bits[0]: 11001100 11001100 11001100 11001100
          3435973836,  // bits[1]: 11001100 11001100 11001100 11001100
          44429496,    // bits[2]: 00000010 10100101 11110000 10111000
          0x00000000   // bits[3]: 00000000 00000000 00000000 00000000
                       // Число в 10: ≈ 8.192 × 10²⁶
      }};
  int status = s21_truncate(number, &result);

  ck_assert_int_eq(status, CodeOK);
  ck_assert(s21_is_equal(result, expected));
}
END_TEST

START_TEST(null_result) {
  s21_decimal number = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00060000  // bits[3]: 00000000 00000110 00000000 00000000
                      // Число в 10: 123.456789
      }};
  int status = s21_truncate(number, NULL);

  ck_assert_int_eq(status, CodeInvalidData);
}
END_TEST

START_TEST(equal_usual) {
  s21_decimal number1 = DEC(1000, 0, 0, 1, 1);
  s21_decimal number2 = DEC(1000, 0, 0, 1, 0);

  ck_assert_int_eq(s21_is_equal(number1, number2), 0);
}
END_TEST

START_TEST(not_equal_usual) {
  s21_decimal number1 = DEC(1000, 0, 0, 1, 0);
  s21_decimal number2 = DEC(1000, 0, 0, 1, 0);

  ck_assert_int_eq(s21_is_not_equal(number1, number2), 0);
}
END_TEST

////////////////////////////////////////
////////// s21_is_greater() ///////////
//////////////////////////////////////

START_TEST(is_greater1) {
  s21_decimal number1 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 1,234,567.89
                      // Число в 2:
                      // 100101101011010000111.111000110101001111101111
      }};
  s21_decimal number2 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};
  ck_assert_int_eq(s21_is_greater(number1, number2), 1);
}
END_TEST

START_TEST(is_greater2) {
  s21_decimal number3 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          1000,       // bits[1]: 00000000 00000000 00000011 11101000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 429496729700.0
                      // Число в 2: 110010000000000000000000000000001100100
      }};
  s21_decimal number4 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          1002,       // bits[1]: 00000000 00000000 00000011 11101010
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 430355723159.2
                      // Число в 2:
                      // 110010000110011001100110011001110010111.001100110011
      }};
  ck_assert_int_eq(s21_is_greater(number3, number4), 0);
}
END_TEST

START_TEST(is_greater3) {
  s21_decimal number5 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          105,        // bits[2]: 00000000 00000000 00000000 01101001
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 193 690 812 773 950 391 988
                      // Число в 2:
                      // 11010010000000000000000000000000000000000000000000000000000001111101000
      }};
  s21_decimal number6 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          100,        // bits[2]: 00000000 00000000 00000000 01100100
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 184 467 440 737 095 516 260
                      // Число в 2:
          // 101000000000000000000000000000000000000000000000000000000000000001100100
      }};
  ck_assert_int_eq(s21_is_greater(number5, number6), 1);
}
END_TEST

START_TEST(is_greater4) {
  s21_decimal number7 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: - 1,234,567.89
                      // Число в 2:
                      // 100101101011010000111.111000110101001111101111
      }};
  s21_decimal number8 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: - 100.0
                      // Число в 2: 1100100.0
      }};
  ck_assert_int_eq(s21_is_greater(number7, number8), 0);
}
END_TEST

START_TEST(is_greater5) {
  s21_decimal number9 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          1000,       // bits[1]: 00000000 00000000 00000011 11101000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: - 429496729700.0
                      // Число в 2: 110010000000000000000000000000001100100
      }};
  s21_decimal number10 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          1002,       // bits[1]: 00000000 00000000 00000011 11101010
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: -430355723159.2
                      // Число в 2:
                      // 110010000110011001100110011001110010111.001100110011
      }};
  ck_assert_int_eq(s21_is_greater(number9, number10), 1);
}
END_TEST

START_TEST(is_greater6) {
  s21_decimal number11 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          105,        // bits[2]: 00000000 00000000 00000000 01101001
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: - 193 690 812 773 950 391 988
                      // Число в 2:
                      // 11010010000000000000000000000000000000000000000000000000000001111101000
      }};
  s21_decimal number12 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          100,        // bits[2]: 00000000 00000000 00000000 01100100
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: - 184 467 440 737 095 516 260
                      // Число в 2:
          // 101000000000000000000000000000000000000000000000000000000000000001100100
      }};
  ck_assert_int_eq(s21_is_greater(number11, number12), 0);
}
END_TEST

START_TEST(is_greater7) {
  s21_decimal number13 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          105,        // bits[2]: 00000000 00000000 00000000 01101001
          0x00010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: 193 690 812 773 950 391 988
                      // Число в 2:
                      // 11010010000000000000000000000000000000000000000000000000000001111101000
      }};
  s21_decimal number14 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          100,        // bits[2]: 00000000 00000000 00000000 01100100
          0x80010000  // bits[3]: 00000000 00000001 00000000 00000000
                      // Число в 10: - 184 467 440 737 095 516 260
                      // Число в 2:
          // 101000000000000000000000000000000000000000000000000000000000000001100100
      }};
  ck_assert_int_eq(s21_is_greater(number13, number14), 1);
}
END_TEST

START_TEST(is_greater8) {
  s21_decimal number15 = {
      .bits = {
          123456789,  // bits[0]: 00000111 01011011 11001101 00010101
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x80010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: - 1,234,567.89
                      // Число в 2:
                      // 100101101011010000111.111000110101001111101111
      }};
  s21_decimal number16 = {
      .bits = {
          1000,       // bits[0]: 00000000 00000000 00000011 11101000
          0,          // bits[1]: 00000000 00000000 00000000 00000000
          0,          // bits[2]: 00000000 00000000 00000000 00000000
          0x00010000  // bits[3]: 10000000 00000001 00000000 00000000
                      // Число в 10: 100.0
                      // Число в 2: 1100100.0
      }};
  ck_assert_int_eq(s21_is_greater(number15, number16), 0);
}
END_TEST

//////////////////////////////////
/////////// s21_round ///////////
////////////////////////////////

// 1. Округление 0.5 -> 1
START_TEST(round_0_5) {
  s21_decimal v = DEC(5, 0, 0, 1, 0);  // 0.5
  s21_decimal res = {{0}};
  int code = s21_round(v, &res);
  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 1);
  ck_assert_int_eq(get_scale(res), 0);
  ck_assert_int_eq(get_sign(res), 0);
}
END_TEST

// 2. Округление -0.5 -> -1
START_TEST(round_minus_0_5) {
  s21_decimal v = DEC(5, 0, 0, 1, 1);  // -0.5
  s21_decimal res = {{0}};
  s21_round(v, &res);
  ck_assert_uint_eq(res.bits[0], 1);
  ck_assert_int_eq(get_sign(res), 1);
}
END_TEST

// 3. Округление 0.4 -> 0
START_TEST(round_0_4) {
  s21_decimal v = DEC(4, 0, 0, 1, 0);  // 0.4
  s21_decimal res = {{0}};
  s21_round(v, &res);
  ck_assert_uint_eq(res.bits[0], 0);
}
END_TEST

// 4. Округление большого числа: 123.567 -> 124
START_TEST(round_complex) {
  s21_decimal v = DEC(123567, 0, 0, 3, 0);  // 123.567
  s21_decimal res = {{0}};
  s21_round(v, &res);
  ck_assert_uint_eq(res.bits[0], 124);
}
END_TEST

// 5. Число без дробной части: 10 -> 10
START_TEST(round_no_scale) {
  s21_decimal v = DEC(10, 0, 0, 0, 0);
  s21_decimal res = {{0}};
  s21_round(v, &res);
  ck_assert_uint_eq(res.bits[0], 10);
  ck_assert_int_eq(get_scale(res), 0);
}
END_TEST

//////////////////////////////////
/////////// s21_floor ///////////
////////////////////////////////

// 1. Положительное число: 1.9 -> 1
START_TEST(floor_positive) {
  s21_decimal v = DEC(19, 0, 0, 1, 0);  // 1.9
  s21_decimal res = {{0}};
  int code = s21_floor(v, &res);
  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 1);
  ck_assert_int_eq(get_sign(res), 0);
}
END_TEST

// 2. Отрицательное число: -1.1 -> -2 (важный случай для floor)
START_TEST(floor_negative) {
  s21_decimal v = DEC(11, 0, 0, 1, 1);  // -1.1
  s21_decimal res = {{0}};
  s21_floor(v, &res);
  ck_assert_uint_eq(res.bits[0], 2);
  ck_assert_int_eq(get_sign(res), 1);
}
END_TEST

// 3. Отрицательное целое: -5.0 -> -5
START_TEST(floor_negative_int) {
  s21_decimal v = DEC(50, 0, 0, 1, 1);  // -5.0
  s21_decimal res = {{0}};
  s21_floor(v, &res);
  ck_assert_uint_eq(res.bits[0], 5);
  ck_assert_int_eq(get_sign(res), 1);
}
END_TEST

// 4. Очень маленькое отрицательное: -0.1 -> -1
START_TEST(floor_small_neg) {
  s21_decimal v = DEC(1, 0, 0, 1, 1);  // -0.1
  s21_decimal res = {{0}};
  s21_floor(v, &res);
  ck_assert_uint_eq(res.bits[0], 1);
  ck_assert_int_eq(get_sign(res), 1);
}
END_TEST

//////////////////////////////////
/////////// s21_negate //////////
////////////////////////////////

// 1. Положительное число становится отрицательным
START_TEST(negate_positive) {
  s21_decimal v = DEC(12345, 0, 0, 0, 0);  // +12345
  s21_decimal res = {{0}};
  int code = s21_negate(v, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 12345);
  ck_assert_int_eq(get_sign(res), 1);  // Стал минус
}
END_TEST

// 2. Отрицательное число становится положительным
START_TEST(negate_negative) {
  s21_decimal v = DEC(12345, 0, 0, 0, 1);  // -12345
  s21_decimal res = {{0}};
  int code = s21_negate(v, &res);

  ck_assert_int_eq(code, CodeOK);
  ck_assert_uint_eq(res.bits[0], 12345);
  ck_assert_int_eq(get_sign(res), 0);  // Стал плюс
}
END_TEST

// 3. Инверсия нуля (0 -> -0)
START_TEST(negate_zero) {
  s21_decimal v = DEC(0, 0, 0, 0, 0);  // +0
  s21_decimal res = {{0}};
  s21_negate(v, &res);

  ck_assert_int_eq(get_sign(res), 1);  // Должен стать -0
  ck_assert_uint_eq(res.bits[0], 0);
}
END_TEST

// 4. Инверсия отрицательного нуля (-0 -> 0)
START_TEST(negate_minus_zero) {
  s21_decimal v = DEC(0, 0, 0, 0, 1);  // -0
  s21_decimal res = {{0}};
  s21_negate(v, &res);

  ck_assert_int_eq(get_sign(res), 0);  // Должен стать +0
}
END_TEST

// 5. Проверка сохранения scale (0.123 -> -0.123)
START_TEST(negate_with_scale) {
  s21_decimal v = DEC(123, 0, 0, 3, 0);  // 0.123
  s21_decimal res = {{0}};
  s21_negate(v, &res);

  ck_assert_int_eq(get_scale(res), 3);
  ck_assert_int_eq(get_sign(res), 1);
  ck_assert_uint_eq(res.bits[0], 123);
}
END_TEST

// --- ТЕСТЫ ДЛЯ s21_mul (покрывают div_by_10_big, add_one_big,
// bank_rounding_big) ---

START_TEST(mul_big_overflow_bank_rounding_to_even) {
  // Тест на переполнение 96 бит и банковское округление.
  // Максимальная мантисса: 79,228,162,514,264,337,593,543,950,335
  // Берем v1 = max_mantissa, v2 = 2. Scale = 1.
  // (Max * 2) / 10 = Max / 5.
  // Подбираем числа так, чтобы остаток был 5 и округление шло к четному.
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);  // Max
  s21_decimal v2 = DEC(2, 0, 0, 1, 0);                             // 0.2
  s21_decimal res = {{0}};

  // Реальный результат до округления: 15845632502852867518708790067.0
  // После деления на 10 и округления мы должны влезть в 96 бит.
  int status = s21_mul(v1, v2, &res);

  ck_assert_int_eq(status, 0);
  // Проверяем, что мантисса изменилась и scale уменьшился
  ck_assert_uint_ne(res.bits[2], 0);
}
END_TEST

START_TEST(mul_bank_rounding_up) {
  // Случай, когда остаток > 5, округляем вверх (задействует add_one_big)
  // 1.1 * 0.5 = 0.55 -> при scale 1 должно стать 0.6?
  // Но нам нужно именно ПЕРЕПОЛНЕНИЕ мантиссы.
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal v2 = DEC(11, 0, 0, 1, 0);  // 1.1
  s21_decimal res = {{0}};

  s21_mul(v1, v2, &res);

  // Здесь сработает цикл: пока результат > 96 бит, дели на 10.
  // Это гарантированно задействует div_by_10_big.
  ck_assert_int_eq(get_scale(res),
                   0);  // Scale 1 упадет до 0, чтобы вместить число
}
END_TEST

// --- ТЕСТЫ ДЛЯ s21_add/align_scale (покрывают s21_bank_rounding и
// add_one_to_mantissa) ---

START_TEST(add_align_bank_rounding_even) {
  // Складываем числа с разными масштабами, где один масштаб нельзя увеличить.
  // v1 = Max мантисса, scale 0.
  // v2 = 0.5 (5 * 10^-1), scale 1.
  // Чтобы выровнять, нужно v2 привести к scale 0 (так как v1 нельзя умножить на
  // 10). При приведении 0.5 к scale 0 сработает банковское округление: 0.5 -> 0
  // (так как 0 - четное).
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal v2 = DEC(5, 0, 0, 1, 0);  // 0.5
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  // Ожидаем, что 0.5 округлилось к 0, и результат равен v1
  ck_assert_uint_eq(res.bits[0], 0xFFFFFFFF);
  ck_assert_uint_eq(res.bits[1], 0xFFFFFFFF);
  ck_assert_uint_eq(res.bits[2], 0xFFFFFFFF);
}
END_TEST

START_TEST(add_align_bank_rounding_odd) {
  // Аналогично, но 1.5 -> 2 (округление к четному, задействует
  // add_one_to_mantissa) v1 = Max мантисса - 1 (чтобы было нечетное в конце,
  // или просто число 1).
  s21_decimal v1 = DEC(1, 0, 0, 0, 0);
  s21_decimal v2 = DEC(15, 0, 0, 1, 0);  // 1.5

  // Чтобы заставить align_scale понижать точность v2, нужно чтобы v1 нельзя
  // было расширить. Забьем v1 под завязку:
  v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  // Теперь при выравнивании 1.5 превратится в 2.
  s21_decimal res = {{0}};

  s21_add(v1, v2, &res);

  // v1 (Max) + 2 (из 1.5) = Overflow (status 1)
  int status = s21_add(v1, v2, &res);
  ck_assert_int_eq(status, 1);
}
END_TEST

START_TEST(add_bank_rounding_exact_logic) {
  // Проверка: 2.5 -> 2, 3.5 -> 4 (банковское округление)
  // Используем пограничные значения мантиссы, чтобы нельзя было повышать scale
  s21_decimal max = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal v2 = DEC(25, 0, 0, 1, 0);  // 2.5
  // s21_decimal v3 = DEC(35, 0, 0, 1, 0); // 3.5
  s21_decimal res = {{0}};

  // 2.5 округлится до 2. Max + 2 = overflow (1)
  ck_assert_int_eq(s21_add(max, v2, &res), 1);

  // Специальный кейс для align_scale: складываем 0.0...25 и 100...00 (max)
  // Это гарантированно вызовет понижение масштаба у меньшего числа.
}
END_TEST

// --- ТЕСТЫ ДЛЯ s21_from_float_to_decimal (покрывают логику округления флоатов)
// ---

START_TEST(float_to_decimal_bank_rounding) {
  s21_decimal res = {{0}};

  // 1234567.5 должен округлиться к 1234568 (к четному)
  s21_from_float_to_decimal(1234567.5f, &res);
  ck_assert_uint_eq(res.bits[0], 1234568);

  // 1234568.5 должен округлиться к 1234568 (к четному)
  s21_from_float_to_decimal(1234568.5f, &res);
  ck_assert_uint_eq(res.bits[0], 1234568);
}
END_TEST

// --- ТЕСТЫ ДЛЯ s21_mul (покрывают div_by_10_big, add_one_big,
// bank_rounding_big) ---

START_TEST(mul_trigger_add_one_big_by_rounding) {
  /* Задача: получить BigInt > 96 бит, который при делении на 10
     даст остаток > 5, чтобы сработал add_one_big.

     Возьмем число чуть меньше Max Decimal:
     v1 = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF} (Max Decimal)
     v2 = 0.6 (6 * 10^-1)

     При умножении Max * 6 получится BigInt, который не лезет в 96 бит.
     Алгоритм mul_normalize вызовет div_by_10_big.
     Остаток будет 0, но если мы подберем множитель так, чтобы остаток был > 5,
     сработает add_one_big.
  */

  // v1 = Max Decimal
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  // v2 = 0.9 (9 * 10^-1).
  // Умножение даст число, заканчивающееся на ...5.5 (при делении на 10 остаток
  // 5 или 9)
  s21_decimal v2 = DEC(9, 0, 0, 1, 0);
  s21_decimal res = {{0}};

  s21_mul(v1, v2, &res);

  /* Логика:
     1. v1 * 9 дает 22-значное число в 16-ричной системе, точно больше 96 бит.
     2. mul_normalize видит переполнение и вызывает div_by_10_big.
     3. Последняя цифра (остаток) будет 5 или более (так как 0xFF...F * 9
     кончается на 1).
     4. bank_rounding_big видит остаток и вызывает add_one_big для инкремента
     мантиссы.
  */
  ck_assert_int_eq(get_scale(res), 0);
  // Если округление сработало вверх, результат будет корректно усечен до 96 бит
  ck_assert_uint_ne(res.bits[0], 0);
}
END_TEST

START_TEST(mul_bank_rounding_to_even_trigger) {
  /* Специфический случай: остаток РОВНО 5.
     Округление к ближайшему четному.
     Если текущее число нечетное — add_one_big сработает.
     Если четное — не сработает.
  */
  // Число, которое после умножения на 5 и деления на 10 (scale) даст остаток 5
  // и потребует округления к четному.
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  s21_decimal v2 = DEC(5, 0, 0, 1, 0);  // 0.5
  s21_decimal res = {{0}};

  s21_mul(v1, v2, &res);

  // Здесь результат (Max / 2) округлится согласно банковскому правилу.
  // Это задействует всю цепочку условий внутри bank_rounding_big.
  ck_assert_int_eq(get_scale(res), 0);
}
END_TEST

// --- ТЕСТЫ ДЛЯ s21_add (покрывают s21_bank_rounding и add_one_to_mantissa) ---

START_TEST(add_trigger_add_one_to_mantissa) {
  /*
     Чтобы задействовать add_one_to_mantissa в s21_decimal.c:
     Нужно сложить числа так, чтобы при align_scale произошло уменьшение
     масштаба с остатком >= 5.
  */
  // v1 — максимально возможное число (нельзя увеличить scale)
  s21_decimal v1 = DEC(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0);
  // v2 = 0.6 (6 * 10^-1). Чтобы прибавить его к v1, нужно превратить v2 в scale
  // 0. 0.6 при scale 0 станет 1.0 (так как 6 > 5, округление вверх).
  s21_decimal v2 = DEC(6, 0, 0, 1, 0);
  s21_decimal res = {{0}};

  // v1(Max) + 1 = Overflow (Code 1)
  int status = s21_add(v1, v2, &res);

  ck_assert_int_eq(status,
                   1);  // Ожидаем переполнение, т.к. 0.6 округлилось до 1
}
END_TEST

Suite *pack_suite() {
  Suite *s;
  s = suite_create("Decimal functions tests");

  // Arithmetics operators tests
  TCase *tc_add = tcase_create("s21_add");
  tcase_add_test(tc_add, add_zero_zero);
  tcase_add_test(tc_add, add_simple_int);
  tcase_add_test(tc_add, add_same_scale);
  tcase_add_test(tc_add, add_diff_scale);
  tcase_add_test(tc_add, add_pos_neg);
  tcase_add_test(tc_add, add_overflow_32bit);
  tcase_add_test(tc_add, add_overflow_max);
  tcase_add_test(tc_add, add_overflow_max_plus_max);
  tcase_add_test(tc_add, add_overflow_negative);
  tcase_add_test(tc_add, add_overflow_min_plus_min);
  suite_add_tcase(s, tc_add);

  TCase *tc_sub = tcase_create("s21_sub");
  tcase_add_test(tc_sub, sub_zero_zero);
  tcase_add_test(tc_sub, sub_simple);
  tcase_add_test(tc_sub, sub_negative_result);
  tcase_add_test(tc_sub, sub_scale);
  tcase_add_test(tc_sub, sub_neg);
  tcase_add_test(tc_sub, sub_overflow_positive);
  tcase_add_test(tc_sub, sub_overflow_negative);
  tcase_add_test(tc_sub, sub_zero_minus_max_ok);
  tcase_add_test(tc_sub, sub_zero_minus_min_ok);
  suite_add_tcase(s, tc_sub);

  TCase *tc_mul = tcase_create("s21_mul");
  tcase_add_test(tc_mul, mul_basic_int);
  tcase_add_test(tc_mul, mul_fractions);
  tcase_add_test(tc_mul, mul_negative);
  tcase_add_test(tc_mul, mul_by_zero);
  tcase_add_test(tc_mul, mul_overflow);
  tcase_add_test(tc_mul, mul_small_fractions);
  suite_add_tcase(s, tc_mul);

  TCase *tc_div = tcase_create("s21_div");
  tcase_add_test(tc_div, div_basic);
  tcase_add_test(tc_div, div_by_zero);
  tcase_add_test(tc_div, div_fraction_result);
  tcase_add_test(tc_div, div_negative);
  tcase_add_test(tc_div, div_fractions);
  tcase_add_test(tc_div, div_one_by_four);
  suite_add_tcase(s, tc_div);

  // Comparison operators tests
  TCase *tc_is_greater = tcase_create("is_greater");
  tcase_add_test(tc_is_greater, is_greater1);
  tcase_add_test(tc_is_greater, is_greater2);
  tcase_add_test(tc_is_greater, is_greater3);
  tcase_add_test(tc_is_greater, is_greater4);
  tcase_add_test(tc_is_greater, is_greater5);
  tcase_add_test(tc_is_greater, is_greater6);
  tcase_add_test(tc_is_greater, is_greater7);
  tcase_add_test(tc_is_greater, is_greater8);
  suite_add_tcase(s, tc_is_greater);

  TCase *tc_s21_comparison = tcase_create("tc_s21_comparison");
  tcase_add_test(tc_s21_comparison, equal_positive);
  tcase_add_test(tc_s21_comparison, greater_first_positive);
  tcase_add_test(tc_s21_comparison, less_first_positive);
  tcase_add_test(tc_s21_comparison, equal_negative);
  tcase_add_test(tc_s21_comparison, greater_first_negative);
  tcase_add_test(tc_s21_comparison, less_first_negative);
  tcase_add_test(tc_s21_comparison, positive_vs_negative);
  tcase_add_test(tc_s21_comparison, negative_vs_positive);
  tcase_add_test(tc_s21_comparison, positive_vs_zero);
  tcase_add_test(tc_s21_comparison, negative_vs_zero);
  tcase_add_test(tc_s21_comparison, zero_equal);
  tcase_add_test(tc_s21_comparison, same_value_different_scale);
  tcase_add_test(tc_s21_comparison, different_value_different_scale);
  tcase_add_test(tc_s21_comparison, large_scale_comparison);
  tcase_add_test(tc_s21_comparison, max_values);
  tcase_add_test(tc_s21_comparison, max_positive_vs_max_negative);
  tcase_add_test(tc_s21_comparison, different_high_bits);
  tcase_add_test(tc_s21_comparison, near_zero);
  tcase_add_test(tc_s21_comparison, equal_usual);
  tcase_add_test(tc_s21_comparison, not_equal_usual);
  suite_add_tcase(s, tc_s21_comparison);

  // Converters function tests

  TCase *tc_from_int = tcase_create("from_int");
  tcase_add_test(tc_from_int, test_from_int_1);
  tcase_add_test(tc_from_int, test_from_int_2);
  tcase_add_test(tc_from_int, test_from_int_3);
  tcase_add_test(tc_from_int, test_from_int_4);
  suite_add_tcase(s, tc_from_int);

  TCase *tc_to_int = tcase_create("to_int");
  tcase_add_test(tc_to_int, test_to_int_1);
  tcase_add_test(tc_to_int, test_to_int_2);
  tcase_add_test(tc_to_int, test_to_int_3);
  tcase_add_test(tc_to_int, test_to_int_4);
  tcase_add_test(tc_to_int, test_to_int_5);
  tcase_add_test(tc_to_int, test_to_int_6);
  suite_add_tcase(s, tc_to_int);

  TCase *tc_float2decimal = tcase_create("s21_convert");
  tcase_add_test(tc_float2decimal, from_float_int_basic);
  tcase_add_test(tc_float2decimal, from_float_negative_int);
  tcase_add_test(tc_float2decimal, from_float_fraction);
  tcase_add_test(tc_float2decimal, from_float_trailing_zeros);
  tcase_add_test(tc_float2decimal, from_float_rounding);
  tcase_add_test(tc_float2decimal, from_float_too_small);
  tcase_add_test(tc_float2decimal, from_float_inf);
  tcase_add_test(tc_float2decimal, from_float_nan);
  tcase_add_test(tc_float2decimal, from_float_zero);
  suite_add_tcase(s, tc_float2decimal);

  TCase *tc_decimal2float = tcase_create("s21_convert");
  tcase_add_test(tc_decimal2float, from_decimal_to_float_int);
  tcase_add_test(tc_decimal2float, from_decimal_to_float_negative);
  tcase_add_test(tc_decimal2float, from_decimal_to_float_fraction);
  tcase_add_test(tc_decimal2float, from_decimal_to_float_big);
  suite_add_tcase(s, tc_decimal2float);

  // Other function tests

  TCase *tc_s21_truncate = tcase_create("tc_s21_truncate");
  tcase_add_test(tc_s21_truncate, positive_with_fraction);
  tcase_add_test(tc_s21_truncate, negative_with_fraction);
  tcase_add_test(tc_s21_truncate, zero_with_scale);
  tcase_add_test(tc_s21_truncate, integer);
  tcase_add_test(tc_s21_truncate, trailing_zeros);
  tcase_add_test(tc_s21_truncate, all_zeros_fraction);
  tcase_add_test(tc_s21_truncate, less_than_one);
  tcase_add_test(tc_s21_truncate, small_negative);
  tcase_add_test(tc_s21_truncate, large_number);
  tcase_add_test(tc_s21_truncate, null_result);
  suite_add_tcase(s, tc_s21_truncate);

  TCase *tc_round = tcase_create("s21_round");
  tcase_add_test(tc_round, round_0_5);
  tcase_add_test(tc_round, round_minus_0_5);
  tcase_add_test(tc_round, round_0_4);
  tcase_add_test(tc_round, round_complex);
  tcase_add_test(tc_round, round_no_scale);
  suite_add_tcase(s, tc_round);

  TCase *tc_floor = tcase_create("s21_floor");
  tcase_add_test(tc_floor, floor_positive);
  tcase_add_test(tc_floor, floor_negative);
  tcase_add_test(tc_floor, floor_negative_int);
  tcase_add_test(tc_floor, floor_small_neg);
  suite_add_tcase(s, tc_floor);

  TCase *tc_negate = tcase_create("s21_negate");
  tcase_add_test(tc_negate, negate_positive);
  tcase_add_test(tc_negate, negate_negative);
  tcase_add_test(tc_negate, negate_zero);
  tcase_add_test(tc_negate, negate_minus_zero);
  tcase_add_test(tc_negate, negate_with_scale);
  suite_add_tcase(s, tc_negate);

  TCase *tc_bonus = tcase_create("Bonus");
  tcase_add_test(tc_bonus, mul_big_overflow_bank_rounding_to_even);
  tcase_add_test(tc_bonus, mul_bank_rounding_up);
  tcase_add_test(tc_bonus, add_align_bank_rounding_even);
  tcase_add_test(tc_bonus, add_align_bank_rounding_odd);
  tcase_add_test(tc_bonus, add_bank_rounding_exact_logic);
  tcase_add_test(tc_bonus, float_to_decimal_bank_rounding);
  tcase_add_test(tc_bonus, mul_trigger_add_one_big_by_rounding);
  tcase_add_test(tc_bonus, mul_bank_rounding_to_even_trigger);
  tcase_add_test(tc_bonus, add_trigger_add_one_to_mantissa);
  suite_add_tcase(s, tc_bonus);

  return s;
}

int main(void) {
  int number_failed;
  SRunner *sr;

  sr = srunner_create(pack_suite());
  srunner_run_all(sr, CK_NORMAL);
  number_failed = srunner_ntests_failed(sr);
  srunner_free(sr);

  printf("=== DECIMAL FUNCTIONS TESTING SUMMARY ===\n");
  printf("Failed tests: %d\n\n", number_failed);

  return (number_failed == 0) ? 0 : 1;
}