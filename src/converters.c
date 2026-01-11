#include <float.h>
#include <math.h>

#include "s21_decimal.h"

int s21_from_int_to_decimal(int src, s21_decimal *dst) {
  if (!dst) return CodeInvalidData;
  *dst = decimal_zero();

  if (src < 0) {
    set_sign(dst, 1);
    src = -src;
  }
  dst->bits[0] = (unsigned int)src;
  return CodeOK;
}

int s21_from_decimal_to_int(s21_decimal src, int *dst) {
  if (!dst) return CodeInvalidData;

  int return_code = CodeOK;
  // Truncate дробную часть
  s21_decimal truncated;
  s21_truncate(src, &truncated);

  // Проверка на вместимость в int
  if (truncated.bits[1] != 0 || truncated.bits[2] != 0 ||
      truncated.bits[0] > 2147483648u) {
    return_code = CodeInvalidData;
  }
  if (return_code == CodeOK && get_sign(src) == 0 &&
      truncated.bits[0] > 2147483647u)
    return_code = CodeBigNumber;

  if (return_code == CodeOK) {
    *dst = (int)truncated.bits[0];
    if (get_sign(src)) *dst = -(*dst);
  }

  return return_code;
}

int s21_from_float_to_decimal(float src, s21_decimal *dst) {
  if (!dst) return CodeInvalidData;
  *dst = decimal_zero();

  if (isinf(src) || isnan(src))
    return CodeBigNumber;  // По заданию error code 1
  if (src != 0.0f && fabsf(src) < 1e-28f)
    return CodeSmallNumber;  // Ошибка и 0 по заданию
  if (src == 0.0f) return CodeOK;

  int return_code = CodeOK;
  // Работаем с double для промежуточной точности
  double dbl = (double)fabsf(src);
  int sign = (src < 0) ? 1 : 0;
  int scale = 0;

  // 1. Увеличиваем, если число слишком маленькое (< 1000000)
  // Нужно мминимум 1000000 (1e6)
  while (dbl < 1000000.0 && scale < 28) {
    dbl *= 10.0;
    scale++;
  }

  // 2. Уменьшаем, если число слишком большое (>= 10000000) (1e7)
  // Если scale > 0, мы уменьшаем scale. Если scale == 0, мы теряем точность
  // (экспонента > 0) Но float поддерживает диапазон до 1e38, а decimal только
  // до примерно 7.9e28.

  // Сначала грубо проверим max decimal
  if (dbl > 7.9228162514e28) return_code = CodeInvalidData;

  if (return_code == CodeOK) {
    // Приводим к 7 цифрам (макс 9999999)
    while (dbl >= 10000000.0) {
      dbl /= 10.0;
      scale--;
    }

    // Если scale стал отрицательным, значит число имеет большие целые цифры.
    // Decimal хранит целые числа, поэтому scale должен быть >= 0.
    // 1.23E+10 -> dbl=12300000, scale = -...
    // В decimal это просто большое число в bits.
    // Но алгоритм "7 значащих" для float требует отрезать лишнее.

    // Восстановим scale в диапазон 0..28
    while (scale < 0) {
      dbl *= 10.0;
      scale++;
    }

    // Теперь у нас число dbl (например 1234567.89) и scale.
    // Нам нужно округлить dbl до целого.
    // Используем long long
    unsigned long long integ = (unsigned long long)dbl;
    double fractional = dbl - integ;

    // Банковское округление на этапе float -> int
    // Если дробная часть > 0.5 -> +1
    // Если дробная часть < 0.5 -> +0
    // Если дробная часть == 0.5 -> к четному
    // Учитываем погрешность float

    if (fractional > 0.5) {
      integ++;
    } else if (fractional == 0.5 ||
               (fractional > 0.4999999f && fractional < 0.5000001f)) {
      if (integ % 2 != 0) integ++;
    }

    // Записываем в decimal
    // integ гарантированно влезает в 64 бита (даже в 32 бита, т.к. < 1e7)
    dst->bits[0] = (unsigned int)integ;
    dst->bits[1] = 0;
    dst->bits[2] = 0;
    set_scale(dst, scale);
    set_sign(dst, sign);

    normalize(dst);
  }

  return return_code;
}

int s21_from_decimal_to_float(s21_decimal src, float *dst) {
  if (!dst) return CodeInvalidData;
  double temp = 0.0;
  for (int i = 0; i < 3; i++) {
    temp += src.bits[i] * pow(2.0, i * 32);
  }
  int scale = get_scale(src);
  temp /= pow(10.0, scale);
  if (get_sign(src)) temp = -temp;
  *dst = (float)temp;
  return CodeOK;
}