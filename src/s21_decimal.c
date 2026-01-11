#include "s21_decimal.h"

// Файл содержит вспомогательные функции, которые используются интерфейсными
// функциями

// Создание нулевого decimal
s21_decimal decimal_zero() {
  s21_decimal result = {0};
  return result;
}

// Проверка на ноль
int is_zero(s21_decimal value) {
  return (value.bits[0] == 0 && value.bits[1] == 0 && value.bits[2] == 0);
}

// Получение значения бита по номеру bit
int get_bit(s21_decimal number, int bit) {
  int discharge = bit / 32;
  int bitPosition = bit % 32;
  return (number.bits[discharge] >> bitPosition) & 1;
}

// Установка значения бита по номеру bit на знак sign
void set_bit(s21_decimal *number, int bit, int sign) {
  int discharge = bit / 32;
  int bitPosition = bit % 32;
  if (sign == 0) {
    number->bits[discharge] &= ~(1u << bitPosition);
  } else {
    number->bits[discharge] |= (1u << bitPosition);
  }
}

// Получение знака числа
int get_sign(s21_decimal number) { return (number.bits[3] >> 31) & 1; }

// Установка знака числа (0 - положительное, 1 - отрицательное)
void set_sign(s21_decimal *number, int sign) {
  if (number != NULL) {
    if (sign == 1) {
      number->bits[3] |= (1u << 31);
    } else {
      number->bits[3] &= ~(1u << 31);
    }
  }
}

// Получение масштаба числа
int get_scale(s21_decimal number) { return (number.bits[3] >> 16) & 0xFF; }

// Установка масштаба числа
void set_scale(s21_decimal *number, int scale) {
  if (number != NULL && scale >= 0 && scale <= 28) {
    number->bits[3] &= ~(0xFF << 16);
    number->bits[3] |= (scale << 16);
  }
}

// Сравнение по модулю (без учета знака и порядка!)
int compare_magnitude(s21_decimal value_1, s21_decimal value_2) {
  int result = 0;

  int flag = 0;
  for (int i = 2; i >= 0 && !flag; i--) {
    if (value_1.bits[i] > value_2.bits[i]) {
      result = 1;
      flag = 1;
    } else if (value_1.bits[i] < value_2.bits[i]) {
      result = -1;
      flag = 1;
    }
  }

  return result;
}

// Сложение для мантиссы + 1 (используется при округлении)
static void add_one_to_mantissa(s21_decimal *val) {
  unsigned long long carry = 1;
  for (int i = 0; i < 3 && carry; i++) {
    unsigned long long sum = (unsigned long long)val->bits[i] + carry;
    val->bits[i] = (unsigned int)sum;
    carry = sum >> 32;
  }
  // Если carry осталось, значит переполнение мантиссы, но в контексте
  // округления это обрабатывается вызывающим кодом
}

// Банковское округление
// remainder - остаток от деления на 10
void s21_bank_rounding(s21_decimal *val, int remainder) {
  if (remainder > 5) {
    add_one_to_mantissa(val);
  } else if (remainder == 5) {
    // Если ровно 0.5, смотрим на последнюю цифру (младший бит)
    if (val->bits[0] & 1) {  // Если нечетное
      add_one_to_mantissa(val);
    }
  }
}

// Нормализация с банковским округлением (для удаления лишних нулей)
void normalize(s21_decimal *value) {
  int scale = get_scale(*value);
  int flag = 0;
  while (scale > 0 && !flag) {
    s21_decimal temp = *value;
    int remainder = div_by_10(&temp);
    if (remainder == 0) {
      *value = temp;
      scale--;
      set_scale(value, scale);
    } else {
      flag = 1;
    }
  }
}

// Выравнивание масштабов с банковским округлением при потере точности
int align_scale(s21_decimal *number1, s21_decimal *number2) {
  int scale1 = get_scale(*number1);
  int scale2 = get_scale(*number2);
  if (scale1 == scale2) return CodeOK;

  int flag = 0;
  // Сначала пытаемся привести к большему scale (умножением)
  while (scale1 != scale2 && !flag) {
    if (scale1 < scale2) {
      s21_decimal temp = *number1;
      // Пробуем умножить на 10
      if (mul_by_10(&temp) == CodeOK) {
        *number1 = temp;
        scale1++;
        set_scale(number1, scale1);
      } else {
        // Переполнение при повышении scale1.
        // Придется уменьшать scale2.
        flag = 1;
      }
    } else {  // scale1 > scale2
      s21_decimal temp = *number2;
      if (mul_by_10(&temp) == CodeOK) {
        *number2 = temp;
        scale2++;
        set_scale(number2, scale2);
      } else {
        flag = 1;
      }
    }
  }

  // Если scales все еще не равны (достигли переполнения), уменьшаем больший
  // scale используя банковское округление
  while (scale1 != scale2) {
    if (scale1 > scale2) {
      int remainder = div_by_10(number1);
      s21_bank_rounding(number1, remainder);
      scale1--;
      set_scale(number1, scale1);
      // После округления число могло стать 0, но scale все равно уменьшился
    } else {
      int remainder = div_by_10(number2);
      s21_bank_rounding(number2, remainder);
      scale2--;
      set_scale(number2, scale2);
    }
  }

  return CodeOK;
}

// Умножение decimal на 10
int mul_by_10(s21_decimal *value) {
  // x * 10 = (x * 8) + (x * 2) = (x << 3) + (x << 1)
  s21_decimal temp = *value;
  int result = CodeOK;
  result = shift_left(&temp, 3);

  s21_decimal temp2 = *value;
  if (result == CodeOK) {
    result = shift_left(&temp2, 1);
  }

  return (result == CodeOK) ? s21_add(temp, temp2, value) : result;
}

// Деление decimal на 10 с получением остатка
int div_by_10(s21_decimal *value) {
  unsigned long long remainder = 0;
  unsigned long long quotient;

  for (int i = 2; i >= 0; i--) {
    quotient = value->bits[i] + (remainder << 32);
    value->bits[i] = quotient / 10;
    remainder = quotient % 10;
  }
  return (int)remainder;
}

// Сдвиг влево
int shift_left(s21_decimal *value, int shift) {
  if (shift <= 0) return CodeOK;

  int res = CodeOK;
  // Простая реализация через побитовый сдвиг
  for (int k = 0; k < shift; k++) {
    int carry = 0;
    for (int i = 0; i < 3; i++) {
      int next_carry = (value->bits[i] >> 31) & 1;
      value->bits[i] = (value->bits[i] << 1) | carry;
      carry = next_carry;
    }
    if (carry) res = CodeBigNumber;  // Переполнение
  }
  return res;
}

// // Сдвиг вправо
// int shift_right(s21_decimal *value, int shift) {
//   if (shift <= 0) return CodeOK;

//   for (int k = 0; k < shift; k++) {
//     int carry = 0;
//     for (int i = 2; i >= 0; i--) {
//       int next_carry = value->bits[i] & 1;
//       value->bits[i] = (value->bits[i] >> 1) | (carry << 31);
//       carry = next_carry;
//     }
//   }
//   return CodeOK;
// }

// Сравнение битов
int bit_equality(s21_decimal num1, s21_decimal num2, int result) {
  (void)result;
  return (num1.bits[0] == num2.bits[0] && num1.bits[1] == num2.bits[1] &&
          num1.bits[2] == num2.bits[2]);
}

int s21_is_equal_for_zero(s21_decimal num1, s21_decimal num2) {
  return (is_zero(num1) && is_zero(num2));
}
