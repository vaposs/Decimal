#include "s21_decimal.h"

// Оператор сравнения
static int s21_comparison_operator(s21_decimal value_1, s21_decimal value_2) {
  int comparison = CodeDivisionZero;
  int status = CodeOK;

  // нормализуем число и проверяем масштабы - если не равны, то выравниваем
  s21_decimal aligned1 = value_1;
  s21_decimal aligned2 = value_2;

  normalize(&aligned1);
  normalize(&aligned2);

  int scale1 = get_scale(aligned1);
  int scale2 = get_scale(aligned2);

  if (scale1 != scale2) {
    status = align_scale(&aligned1, &aligned2);
  }

  if (status == CodeOK) {
    // сравниваем числа по модулю
    s21_decimal temp_value1 = aligned1;
    s21_decimal temp_value2 = aligned2;
    // делаем числа положительными
    set_sign(&temp_value1, 0);
    set_sign(&temp_value2, 0);

    comparison = compare_magnitude(temp_value1, temp_value2);
  }
  return comparison;
}

// Меньше
int s21_is_less(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;

  if (!(is_zero(value_1) && is_zero(value_2))) {
    int sign1 = get_sign(value_1);
    int sign2 = get_sign(value_2);

    if (sign1 != sign2) {
      result = (sign1 > sign2) ? 1 : 0;
    } else {
      int comparison = s21_comparison_operator(value_1, value_2);
      // В зависимости от знаков чисел, смотрим результаты сравнения
      result = (sign1 == 0) ? ((comparison < 0) ? 1 : 0)
                            : ((comparison > 0) ? 1 : 0);
    }
  }
  return result;
}

// Меньше или равно
int s21_is_less_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;

  if (!(is_zero(value_1) && is_zero(value_2))) {
    int sign1 = get_sign(value_1);
    int sign2 = get_sign(value_2);

    if (sign1 != sign2) {
      result = (sign1 > sign2) ? 1 : 0;
    } else {
      int comparison = s21_comparison_operator(value_1, value_2);
      // В зависимости от знаков чисел, смотрим результаты сравнения
      result = (sign1 == 0) ? ((comparison <= 0) ? 1 : 0)
                            : ((comparison >= 0) ? 1 : 0);
    }
  } else {
    result = 1;
  }
  return result;
}

int s21_is_greater(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;

  if (!(is_zero(value_1) && is_zero(value_2))) {
    int sign1 = get_sign(value_1);
    int sign2 = get_sign(value_2);

    if (sign1 != sign2) {
      result = (sign1 < sign2) ? 1 : 0;
    } else {
      int comparison = s21_comparison_operator(value_1, value_2);
      // В зависимости от знаков чисел, смотрим результаты сравнения
      result = (sign1 == 0) ? ((comparison > 0) ? 1 : 0)
                            : ((comparison < 0) ? 1 : 0);
    }
  } else {
    result = 1;
  }
  return result;
}

// Больше или равно
int s21_is_greater_or_equal(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;

  if (!(is_zero(value_1) && is_zero(value_2))) {
    int sign1 = get_sign(value_1);
    int sign2 = get_sign(value_2);

    if (sign1 != sign2) {
      result = (sign1 < sign2) ? 1 : 0;
    } else {
      int comparison = s21_comparison_operator(value_1, value_2);
      // В зависимости от знаков чисел, смотрим результаты сравнения
      result = (sign1 == 0) ? ((comparison >= 0) ? 1 : 0)
                            : ((comparison <= 0) ? 1 : 0);
    }
  } else {
    result = 1;
  }
  return result;
}

// Равно
int s21_is_equal(s21_decimal num1, s21_decimal num2) {
  int result = 0;
  int status = align_scale(&num1, &num2);
  if ((status == CodeOK && get_sign(num1) == 1 && get_sign(num2) == 1) ||
      (status == CodeOK && get_sign(num1) == 0 &&
       get_sign(num2) == 0)) {  // 0 - положительное; 1 - отрицательное.
    result = bit_equality(num1, num2, result);
  } else {
    result = 0;
  }
  return result;
}

// Неравенство
int s21_is_not_equal(s21_decimal value_1, s21_decimal value_2) {
  return !s21_is_equal(value_1, value_2);
}
