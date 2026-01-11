#include "s21_decimal.h"

// Получение целой части
int s21_truncate(s21_decimal value, s21_decimal *result) {
  int status = CodeOK;

  if (result == NULL) {
    status = CodeInvalidData;
  } else {
    *result = decimal_zero();
    if (is_zero(value)) {
      status = CodeOK;
    } else {
      int scale = get_scale(value);
      s21_decimal temp_value = value;
      while (scale > 0) {
        div_by_10(&temp_value);
        scale--;
      }
      *result = temp_value;
      set_scale(result, 0);
    }
  }
  return status;
}

// Умножение на -1
int s21_negate(s21_decimal value, s21_decimal *result) {
  int status = CodeOK;

  if (result == NULL) {
    status = CodeInvalidData;
  }

  if (status == CodeOK) {
    *result = value;
    set_sign(result, !get_sign(value));
  }
  return status;
}

// Математическое округление
int s21_round(s21_decimal value, s21_decimal *result) {
  if (!result) return CodeInvalidData;
  int status = CodeOK;
  int sign = get_sign(value);
  int scale = get_scale(value);
  s21_decimal fractional = value;

  // Извлекаем целую часть
  status = s21_truncate(value, result);

  // Чтобы узнать первую цифру после запятой:
  // Уменьшаем масштаб до 1, затем берем остаток от деления на 10
  set_sign(&fractional, 0);
  for (int i = 0; i < scale - 1; i++) {
    div_by_10(&fractional);
  }

  if (status == CodeOK && scale > 0) {
    int last_digit = div_by_10(&fractional);
    if (last_digit >= 5) {
      s21_decimal one = {{1, 0, 0, 0}};
      s21_decimal tmp_res = *result;
      set_sign(&one, sign);
      status = s21_add(tmp_res, one, result);
    }
  }

  if (status == CodeOK) {
    set_scale(result, 0);
  }

  return status;
}

// Округление до минус бесконечности
int s21_floor(s21_decimal value, s21_decimal *result) {
  if (!result) return CodeInvalidData;
  int status = CodeOK;
  int sign = get_sign(value);

  // Получаем целую часть
  status = s21_truncate(value, result);

  if (status == CodeOK && sign) {
    s21_decimal diff = {{0}};
    s21_sub(value, *result, &diff);

    // Если число отрицательное и есть остаток — вычитаем 1
    if (!is_zero(diff)) {
      s21_decimal one = {{1, 0, 0, 0}};
      s21_decimal tmp_res = *result;
      set_sign(&one, 1);
      status = s21_add(tmp_res, one, result);
    }
  }

  if (status == CodeOK) {
    set_scale(result, 0);
  }

  return status;
}
