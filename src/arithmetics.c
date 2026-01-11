#include "s21_decimal.h"

// Структуры и вспомогательные функции для BigInt (внутреннее использование)

typedef struct {
  unsigned int bits[6];
} s21_big_decimal;

static int is_zero_big(s21_big_decimal val) {
  int res = 1;
  for (int i = 0; i < 6 && res; i++) {
    if (val.bits[i] != 0) res = 0;
  }
  return res;
}

static int div_by_10_big(s21_big_decimal *val) {
  unsigned long long remainder = 0;
  for (int i = 5; i >= 0; i--) {
    unsigned long long cur = val->bits[i] + (remainder << 32);
    val->bits[i] = (unsigned int)(cur / 10);
    remainder = cur % 10;
  }
  return (int)remainder;
}

static void add_one_big(s21_big_decimal *val) {
  unsigned long long carry = 1;
  for (int i = 0; i < 6 && carry; i++) {
    unsigned long long sum = (unsigned long long)val->bits[i] + carry;
    val->bits[i] = (unsigned int)sum;
    carry = sum >> 32;
  }
}

static void bank_rounding_big(s21_big_decimal *val, int remainder) {
  if (remainder > 5 || (remainder == 5 && (val->bits[0] & 1))) {
    add_one_big(val);
  }
}

static int fits_in_96(s21_big_decimal val) {
  return (val.bits[3] == 0 && val.bits[4] == 0 && val.bits[5] == 0);
}

static s21_big_decimal mul_big(s21_decimal v1, s21_decimal v2) {
  s21_big_decimal res = {0};
  for (int i = 0; i < 3; i++) {
    unsigned long long carry = 0;
    for (int j = 0; j < 3; j++) {
      unsigned long long prod = (unsigned long long)v1.bits[i] * v2.bits[j];
      unsigned long long sum =
          (unsigned long long)res.bits[i + j] + prod + carry;
      res.bits[i + j] = (unsigned int)sum;
      carry = sum >> 32;
    }
    int k = i + 3;
    while (carry && k < 6) {
      unsigned long long sum = (unsigned long long)res.bits[k] + carry;
      res.bits[k] = (unsigned int)sum;
      carry = sum >> 32;
      k++;
    }
  }
  return res;
}

// Вспомогательные функции для декомпозиции

// Сложение мантисс одинакового знака с обработкой переполнения BigInt
static int add_same_sign(s21_decimal v1, s21_decimal v2, s21_decimal *res,
                         int sign) {
  int status = CodeOK;
  s21_big_decimal big = {0};
  unsigned long long carry = 0;
  int scale = get_scale(v1);

  for (int i = 0; i < 3; i++) {
    unsigned long long sum =
        (unsigned long long)v1.bits[i] + v2.bits[i] + carry;
    big.bits[i] = (unsigned int)sum;
    carry = sum >> 32;
  }
  big.bits[3] = (unsigned int)carry;

  if (carry && scale > 0) {
    int rem = div_by_10_big(&big);
    bank_rounding_big(&big, rem);
    scale--;
  }

  if (!fits_in_96(big)) {
    status = (sign == 0) ? CodeBigNumber : CodeSmallNumber;
  } else {
    for (int i = 0; i < 3; i++) res->bits[i] = big.bits[i];
    set_sign(res, sign);
    set_scale(res, scale);
  }
  return status;
}

// Вычитание мантисс (v1 >= v2)
static void sub_mantissas(s21_decimal v1, s21_decimal v2, s21_decimal *res) {
  unsigned long long borrow = 0;
  for (int i = 0; i < 3; i++) {
    unsigned long long a = v1.bits[i];
    unsigned long long b = v2.bits[i] + borrow;
    if (a < b) {
      res->bits[i] = (unsigned int)(a + 0x100000000ULL - b);
      borrow = 1;
    } else {
      res->bits[i] = (unsigned int)(a - b);
      borrow = 0;
    }
  }
}

// Обработка сложения разных знаков
static int add_diff_sign(s21_decimal v1, s21_decimal v2, s21_decimal *res) {
  int sign1 = get_sign(v1);
  int cmp = compare_magnitude(v1, v2);
  s21_decimal *larger = (cmp >= 0) ? &v1 : &v2;
  s21_decimal *smaller = (cmp >= 0) ? &v2 : &v1;
  int res_sign = (cmp >= 0) ? sign1 : get_sign(v2);

  sub_mantissas(*larger, *smaller, res);
  set_sign(res, res_sign);
  set_scale(res, get_scale(v1));
  return CodeOK;
}

// Нормализация результата умножения (уменьшение scale при переполнении)
static void mul_normalize(s21_big_decimal *big, int *scale) {
  int stop = 0;
  while (*scale > 0 && !stop) {
    if (!fits_in_96(*big) || *scale > 28) {
      int rem = div_by_10_big(big);
      bank_rounding_big(big, rem);
      (*scale)--;
    } else {
      stop = 1;
    }
  }
}

// Вспомогательная для деления мантисс (целая часть)
static void div_calc_integer(s21_decimal v1, s21_decimal v2, s21_decimal *quot,
                             s21_decimal *rem) {
  *quot = decimal_zero();
  *rem = decimal_zero();
  if (compare_magnitude(v1, v2) >= 0) {
    s21_decimal temp_v2 = v2;
    // Простой алгоритм деления
    for (int i = 95; i >= 0; i--) {
      shift_left(rem, 1);
      if (get_bit(v1, i)) set_bit(rem, 0, 1);
      if (compare_magnitude(*rem, temp_v2) >= 0) {
        sub_mantissas(*rem, temp_v2, rem);
        set_bit(quot, i, 1);
      }
    }
  } else {
    *rem = v1;
  }
}

// Расчет дробной части деления
static int div_calc_fractional(s21_decimal v2, s21_decimal *rem,
                               s21_decimal *res, int *scale) {
  int status = CodeOK;
  int stop = 0;
  while (!is_zero(*rem) && *scale < 28 && status == CodeOK && !stop) {
    s21_decimal tmp_res = *res;
    s21_decimal tmp_rem = *rem;
    if (mul_by_10(&tmp_res) == CodeOK && mul_by_10(&tmp_rem) == CodeOK) {
      *rem = tmp_rem;
      s21_decimal part = {0};
      div_calc_integer(*rem, v2, &part, rem);
      if (s21_add(tmp_res, part, res) == CodeOK) {
        (*scale)++;
      } else {
        status = CodeInvalidData;  // Переполнение при сложении дробной части
      }
    } else {
      stop = 1;  // Переполнение при умножении на 10
    }
  }
  return status;
}

// Основные функции

int s21_add(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return CodeInvalidData;
  *result = decimal_zero();
  int status = align_scale(&value_1, &value_2);

  if (status == CodeOK) {
    if (get_sign(value_1) == get_sign(value_2)) {
      status = add_same_sign(value_1, value_2, result, get_sign(value_1));
    } else {
      status = add_diff_sign(value_1, value_2, result);
    }
  }
  return status;
}

int s21_sub(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return CodeInvalidData;
  set_sign(&value_2, !get_sign(value_2));
  return s21_add(value_1, value_2, result);
}

int s21_mul(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return CodeInvalidData;
  *result = decimal_zero();
  int status = CodeOK;
  int sign = get_sign(value_1) ^ get_sign(value_2);
  int scale = get_scale(value_1) + get_scale(value_2);
  s21_big_decimal res_big = mul_big(value_1, value_2);

  if (!is_zero_big(res_big)) {
    mul_normalize(&res_big, &scale);
    if (fits_in_96(res_big) && scale <= 28) {
      for (int i = 0; i < 3; i++) result->bits[i] = res_big.bits[i];
      set_scale(result, scale);
      set_sign(result, sign);
    } else {
      status = (sign == 0) ? CodeBigNumber : CodeSmallNumber;
    }
  } else {
    set_scale(result, 0);  // Чистый ноль
  }
  return status;
}

int s21_div(s21_decimal value_1, s21_decimal value_2, s21_decimal *result) {
  if (!result) return CodeInvalidData;
  if (is_zero(value_2)) return CodeDivisionZero;
  *result = decimal_zero();

  int return_code = CodeOK;

  int sign = get_sign(value_1) ^ get_sign(value_2);
  int scale = get_scale(value_1) - get_scale(value_2);
  set_scale(&value_1, 0);
  set_sign(&value_1, 0);
  set_scale(&value_2, 0);
  set_sign(&value_2, 0);

  // Выравнивание отрицательного scale
  while (scale < 0) {
    if (mul_by_10(&value_1) == CodeOK) {
      scale++;
    } else {
      // Если делимое не влезает, уменьшаем делитель
      int rem = div_by_10(&value_2);
      s21_bank_rounding(&value_2, rem);  // Внешняя функция
      scale++;
    }
  }

  if (is_zero(value_2))
    return_code = CodeDivisionZero;  // Защита от схлопывания в 0

  if (return_code == CodeOK) {
    s21_decimal rem = {0};
    div_calc_integer(value_1, value_2, result, &rem);
    div_calc_fractional(value_2, &rem, result, &scale);

    set_sign(result, sign);
    set_scale(result, scale);
  }

  return return_code;
}