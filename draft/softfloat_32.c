// softfloat32.c — минимальная реализация IEEE‑754 single‑precision (float)
// без использования FPU/аппаратных операций с плавающей запятой.
// Поддержаны: преобразования int32<->float, +, -, *, /, сравнения (eq, lt, le),
// обработка NaN/Inf/denormals, округление — к ближайшему с ties-to-even.
//
// ⚠️ Ограничения:
//  * Требуются 64‑битные целые (uint64_t) для умножения/деления мантисс.
//  * Производительность ориентирована на простоту и корректность, не на скорость.
//  * Реализовано только single precision (32 бита).
//
// Использование: храните «float» как uint32_t битовый образ. Все функции
// работают с такими представлениями и не используют тип float.
//
// Пример:
//   uint32_t a = f32_from_int32(3);
//   uint32_t b = f32_from_int32(5);
//   uint32_t c = f32_add(a, b); // 8.0f
//   int32_t  i = f32_to_int32_trunc(c); // 8

#include <stdint.h>
#include <stdbool.h>

// --- Константы формата IEEE‑754 single precision ---
#define F32_SIGN_MASK 0x80000000u
#define F32_EXP_MASK  0x7F800000u
#define F32_FRAC_MASK 0x007FFFFFu
#define F32_EXP_BIAS  127
#define F32_EXP_INF_NAN 255

// Справочные конструкторы
static inline uint32_t f32_pack(bool sign, int32_t exp, uint32_t frac) {
    return (sign ? F32_SIGN_MASK : 0) | ((uint32_t)exp << 23) | (frac & F32_FRAC_MASK);
}

static inline bool f32_sign(uint32_t a){ return (a & F32_SIGN_MASK) != 0; }
static inline int32_t f32_exp(uint32_t a){ return (int32_t)((a & F32_EXP_MASK) >> 23); }
static inline uint32_t f32_frac(uint32_t a){ return a & F32_FRAC_MASK; }

static inline bool f32_is_nan(uint32_t a){ return (f32_exp(a) == F32_EXP_INF_NAN) && (f32_frac(a) != 0); }
static inline bool f32_is_inf(uint32_t a){ return (f32_exp(a) == F32_EXP_INF_NAN) && (f32_frac(a) == 0); }
static inline bool f32_is_zero(uint32_t a){ return (a & ~F32_SIGN_MASK) == 0; }

// Canonical quiet NaN
static inline uint32_t f32_qnan(void){ return 0x7FC00000u; }

// Подсчёт ведущих нулей (без compiler builtins)
static inline int clz32(uint32_t x){
    if(!x) return 32;
    int n = 0;
    if ((x & 0xFFFF0000u) == 0){ n += 16; x <<= 16; }
    if ((x & 0xFF000000u) == 0){ n += 8; x <<= 8; }
    if ((x & 0xF0000000u) == 0){ n += 4; x <<= 4; }
    if ((x & 0xC0000000u) == 0){ n += 2; x <<= 2; }
    if ((x & 0x80000000u) == 0){ n += 1; }
    return n;
}

static inline int clz64(uint64_t x){
    if(!x) return 64;
    uint32_t hi = (uint32_t)(x >> 32);
    if (hi) return clz32(hi);
    return 32 + clz32((uint32_t)x);
}

// Нормализация субнормалов: выдаёт нормализованную мантиссу (с скрытым 1)
// и корректирует экспоненту. На выходе mant имеет вид [1.xxx]*2^23.
static inline void f32_norm_subnormal(uint32_t frac, int32_t *exp, uint32_t *mant){
    int shift = clz32(frac) - 8; // 31‑23 = 8
    if (shift < 0) shift = 0;
    *mant = frac << (shift + 1); // добавим предполагаемую «1.» в старший разряд 24‑битной мантиссы
    *exp  = 1 - shift; // exp==0 => денормал; после сдвига появится скрытая 1.
}

// Округление: вход mant имеет ширину >= 27 бит: [1][23 бит][G][R][S]
// Возвращает упакованные exp/mant с учётом переполнения округлением.
static inline uint32_t f32_round_pack(bool sign, int32_t exp, uint64_t mant){
    // mant: 27+ бит: 1.23 | G | R | S
    // Соберём GRS
    uint32_t guard = (uint32_t)((mant >> 2) & 1u);
    uint32_t round = (uint32_t)((mant >> 1) & 1u);
    uint32_t sticky= (uint32_t)(mant & 1u);

    uint32_t frac = (uint32_t)(mant >> 3); // сейчас это 24‑бит с ведущей 1 в бите 23 либо 0

    // round-to-nearest-even
    bool increment = false;
    if (guard) {
        if (round || sticky || (frac & 1u)) increment = true;
    }
    if (increment){
        frac += 1u;
        // handle carry из 24‑битной мантиссы -> переполнение в 25‑й бит
        if (frac == (1u<<24)){
            frac >>= 1; // 1.000000 -> 1.000000 после сдвига
            exp += 1;
        }
    }

    if (exp >= 0xFF){ // overflow -> inf
        return f32_pack(sign, F32_EXP_INF_NAN, 0);
    }
    if (exp <= 0){
        // Поднормализуем результат: exp<=0 => превращаем в денормал с возможной потерей точности
        if (exp < -24) {
            // Слишком маленькое -> ±0 (под действием округления)
            return f32_pack(sign, 0, 0);
        }
        // Сдвинем так, чтобы экспонента стала 0: потеряем старшие биты, добавим GRS и округлим заново
        int shift = 1 - exp; // на сколько нужно опустить
        uint64_t m = ((uint64_t)frac) << 3; // вернём GRS позиции
        // Соберём новую GRS при сдвиге вправо на shift
        uint64_t shifted = m >> shift;
        uint64_t lost = m & (((uint64_t)1 << shift) - 1);
        uint64_t sticky2 = lost ? 1 : 0;
        mant = (shifted & ~7ull) | (shifted & 4ull) | (shifted & 2ull) | sticky2; // уже содержит GRS
        // Повторим финальное округление с exp=0
        uint32_t guard2 = (uint32_t)((mant >> 2) & 1u);
        uint32_t round2 = (uint32_t)((mant >> 1) & 1u);
        uint32_t sticky3= (uint32_t)(mant & 1u);
        frac = (uint32_t)(mant >> 3);
        bool inc2 = false;
        if (guard2) {
            if (round2 || sticky3 || (frac & 1u)) inc2 = true;
        }
        if (inc2){
            frac += 1u;
            if (frac == (1u<<24)){
                // переполнение денормала -> минимальная нормаль
                return f32_pack(sign, 1, 0);
            }
        }
        return f32_pack(sign, 0, frac & F32_FRAC_MASK);
    }

    // Срезаем скрытую 1 перед упаковкой (бит 23)
    return f32_pack(sign, exp, frac & F32_FRAC_MASK);
}

// --- Преобразования int32 <-> float ---
uint32_t f32_from_int32(int32_t a){
    if (a == 0) return 0;
    bool sign = a < 0;
    uint32_t mag = sign ? (uint32_t)(- (int64_t)a) : (uint32_t)a; // избежим UB на INT32_MIN
    int lz = clz32(mag);
    int shift = 31 - lz; // индекс старшего 1
    int32_t exp = shift + F32_EXP_BIAS;
    // Сформируем мантиссу 24+3 бита с GRS
    uint64_t mant = (uint64_t)mag << (23 + 3 - shift); // выровняем так, что старшая 1 в бите 23+3
    return f32_round_pack(sign, exp, mant);
}

int32_t f32_to_int32_trunc(uint32_t a){
    if (f32_is_nan(a) || f32_is_inf(a)) return 0x80000000u; // сигнал об ошибке
    int32_t exp = f32_exp(a);
    uint32_t frac = f32_frac(a);
    bool sign = f32_sign(a);

    uint32_t mant = (exp ? (1u<<23) | frac : frac); // для денормала нет скрытой 1
    int32_t e = exp - F32_EXP_BIAS - 23; // степень для целого

    int64_t val;
    if (e >= 0){
        if (e > 31) return 0x80000000u; // overflow
        val = (int64_t)mant << e;
    } else {
        if (-e > 63) return 0; // слишком маленькое
        val = (int64_t)(mant >> (-e));
    }
    if (sign) val = -val;
    if (val > 0x7FFFFFFFLL || val < (int64_t)0x80000000LL) return 0x80000000u; // overflow
    return (int32_t)val;
}

// --- Сложение/вычитание ---
static uint32_t f32_add_core(uint32_t a, uint32_t b, bool sub){
    // Обработка спецслучаев
    if (f32_is_nan(a) || f32_is_nan(b)) return f32_qnan();
    if (f32_is_inf(a) || f32_is_inf(b)){
        if (f32_is_inf(a) && f32_is_inf(b)){
            // inf + (-inf) => NaN
            if ((f32_sign(a) ^ f32_sign(b)) ^ sub) return f32_qnan();
        }
        return f32_is_inf(a) ? a : (sub ? (b ^ F32_SIGN_MASK) : b);
    }

    bool signA = f32_sign(a);
    bool signB = f32_sign(b) ^ sub; // для вычитания инвертируем знак b

    int32_t expA = f32_exp(a);
    int32_t expB = f32_exp(b);
    uint32_t fracA = f32_frac(a);
    uint32_t fracB = f32_frac(b);

    // Преобразуем в 27‑битные мантиссы (24 + GRS)
    uint64_t mantA, mantB;
    if (expA){ mantA = ((uint64_t)((1u<<23) | fracA) << 3); }
    else if (fracA){ int32_t e; uint32_t m; f32_norm_subnormal(fracA, &e, &m); expA = e; mantA = ((uint64_t)m << 3); }
    else { mantA = 0; }

    if (expB){ mantB = ((uint64_t)((1u<<23) | fracB) << 3); }
    else if (fracB){ int32_t e; uint32_t m; f32_norm_subnormal(fracB, &e, &m); expB = e; mantB = ((uint64_t)m << 3); }
    else { mantB = 0; }

    // Выравнивание по большей экспоненте
    int32_t exp = expA;
    if (expA < expB){
        int shift = expB - expA;
        if (shift > 31) shift = 31;
        // сдвиг вправо с формированием sticky
        uint64_t lost = mantA & (((uint64_t)1<<shift) - 1);
        mantA = (mantA >> shift) | (lost ? 1 : 0);
        exp = expB;
    } else if (expB < expA){
        int shift = expA - expB;
        if (shift > 31) shift = 31;
        uint64_t lost = mantB & (((uint64_t)1<<shift) - 1);
        mantB = (mantB >> shift) | (lost ? 1 : 0);
        exp = expA;
    }

    // Сложение/вычитание с учётом знаков
    uint64_t mant;
    bool sign;
    if (signA == signB){
        mant = mantA + mantB;
        sign = signA;
        // Нормализация возможного переноса (бит 27)
        if (mant & (1ull << (23+4))){ // 1 доп бит сверху (24+3 -> 25+3)
            mant = (mant >> 1) | (mant & 1ull); // sticky сохраняем
            exp += 1;
        }
    } else {
        // Вычитание: выберем большее по мантиссе
        if (mantA > mantB){ mant = mantA - mantB; sign = signA; }
        else if (mantB > mantA){ mant = mantB - mantA; sign = signB; }
        else { // ноль
            return f32_pack(false, 0, 0);
        }
        // Нормализация вниз: сдвигаем до появления 1 в бите 26 (после GRS это 23+3)
        int lz = clz64(mant) - (64 - (23+4)); // сколько незначащих сверху относительно бита (23+3)
        if (lz < 0) lz = 0;
        if (lz > 0){
            // При сдвиге влево sticky не нужен
            mant <<= lz;
            exp -= lz;
        }
    }

    return f32_round_pack(sign, exp, mant);
}

uint32_t f32_add(uint32_t a, uint32_t b){ return f32_add_core(a, b, false); }
uint32_t f32_sub(uint32_t a, uint32_t b){ return f32_add_core(a, b, true); }

// --- Умножение ---
uint32_t f32_mul(uint32_t a, uint32_t b){
    if (f32_is_nan(a) || f32_is_nan(b)) return f32_qnan();
    bool sign = f32_sign(a) ^ f32_sign(b);
    int32_t expA = f32_exp(a), expB = f32_exp(b);
    uint32_t fracA = f32_frac(a), fracB = f32_frac(b);

    if (f32_is_inf(a) || f32_is_inf(b)){
        if (f32_is_zero(a) || f32_is_zero(b)) return f32_qnan();
        return f32_pack(sign, F32_EXP_INF_NAN, 0);
    }
    if (f32_is_zero(a) || f32_is_zero(b)) return f32_pack(sign, 0, 0);

    // Нормализуем субнормалы
    uint32_t mantA, mantB; int32_t eA = expA, eB = expB;
    if (expA){ mantA = (1u<<23) | fracA; }
    else { f32_norm_subnormal(fracA, &eA, &mantA); }
    if (expB){ mantB = (1u<<23) | fracB; }
    else { f32_norm_subnormal(fracB, &eB, &mantB); }

    int32_t exp = eA + eB - F32_EXP_BIAS;
    // Перемножим 24x24 -> 48 бит
    uint64_t prod = (uint64_t)mantA * (uint64_t)mantB; // верхние 48 бит значимы

    // Нормализуем так, чтобы старшая 1 была в бите 47
    // у нормализованных мантисс 1.xxx * 1.xxx ∈ [1,4), следовательно
    // если бит 47 == 1 и бит 46 может быть 0/1; если бит 47 == 2 (т.е. >=2), сдвинем.
    // Приведём к формату 1.23 + GRS => сдвинем к 24+3 = 27 доп.битам
    // Смотрим на бит 47 (индексация от 0)
    if (prod & (1ull<<47)){
        // отлично, старшая 1 в 47 бите -> сдвигаем так, чтобы 23‑й бит мантиссы оказался на месте
        // Нам нужна раскладка: (prod >> (47-23-3)) => с GRS
    } else {
        // Если вдруг < 1.0 (теоретически может для субнормалов), поднимем
        prod <<= 1; exp -= 1;
    }

    uint64_t mant = prod >> (47 - 23 - 3); // оставим 24+3 бита (включая скрытую 1 и GRS)

    return f32_round_pack(sign, exp, mant);
}

// --- Деление ---
uint32_t f32_div(uint32_t a, uint32_t b){
    if (f32_is_nan(a) || f32_is_nan(b)) return f32_qnan();
    bool sign = f32_sign(a) ^ f32_sign(b);
    if (f32_is_zero(b)){
        if (f32_is_zero(a)) return f32_qnan();
        return f32_pack(sign, F32_EXP_INF_NAN, 0);
    }
    if (f32_is_inf(b)){
        return f32_pack(sign, 0, 0);
    }
    if (f32_is_inf(a)){
        return f32_pack(sign, F32_EXP_INF_NAN, 0);
    }
    if (f32_is_zero(a)) return f32_pack(sign, 0, 0);

    int32_t expA = f32_exp(a), expB = f32_exp(b);
    uint32_t mantA, mantB; int32_t eA = expA, eB = expB;
    uint32_t fracA = f32_frac(a), fracB = f32_frac(b);

    if (expA){ mantA = (1u<<23) | fracA; }
    else { f32_norm_subnormal(fracA, &eA, &mantA); }
    if (expB){ mantB = (1u<<23) | fracB; }
    else { f32_norm_subnormal(fracB, &eB, &mantB); }

    int32_t exp = eA - eB + F32_EXP_BIAS;

    // Выполним деление с повышенной точностью: получим 24+3 бита частного
    // Нормализуем делимое в диапазон [1.0, 2.0) << 24
    uint64_t dividend = ((uint64_t)mantA) << (23 + 3); // чтобы получить GRS внизу
    uint32_t divisor  = mantB;

    // Целочисленное деление
    uint64_t quot = dividend / divisor;       // до 24+3+? бит
    uint64_t rem  = dividend % divisor;

    // Ограничим квоту до 24+3+1 бит, чтобы передать в round_pack
    // Если квота слишком большая, подвинем
    int msb = 63 - clz64(quot);
    if (msb < (23+3)){
        // квота меньше ожидаемой -> сдвинем вверх
        int d = (23+3) - msb;
        quot <<= d; exp -= d;
    } else if (msb > (23+3)){
        int d = msb - (23+3);
        // sticky бит объединим с тем, что потеряем при сдвиге
        uint64_t lost = quot & (((uint64_t)1<<d)-1);
        quot = (quot >> d) | (lost ? 1 : 0);
        exp += d;
    }

    // Добавим sticky, если остаток не нулевой
    if (rem) quot |= 1ull; // sticky в младшем бите (S)

    return f32_round_pack(sign, exp, quot);
}

// --- Сравнения (опционально) ---
bool f32_eq(uint32_t a, uint32_t b){ if (f32_is_nan(a) || f32_is_nan(b)) return false; return a==b || (f32_is_zero(a) && f32_is_zero(b)); }
bool f32_lt(uint32_t a, uint32_t b){ if (f32_is_nan(a) || f32_is_nan(b)) return false; if (a==b) return false; bool sa=f32_sign(a), sb=f32_sign(b); if (sa!=sb) return sa; // - < +
    // Сравнение абсолютных значений
    uint32_t aa = a & ~F32_SIGN_MASK, bb = b & ~F32_SIGN_MASK; bool less = (aa<bb); return sa ? !less : less; }
bool f32_le(uint32_t a, uint32_t b){ return f32_lt(a,b) || f32_eq(a,b); }

// --- Вспомогательные конструкторы для констант ---
uint32_t f32_make_zero(bool sign){ return f32_pack(sign, 0, 0); }
uint32_t f32_make_inf(bool sign){ return f32_pack(sign, F32_EXP_INF_NAN, 0); }
uint32_t f32_make_nan(void){ return f32_qnan(); }

// --- (Необязательно) печать в десятичной форме без FPU ---
// Реализация dtoa не включена для краткости.

#ifdef SOFTFLOAT32_TEST_MAIN
#include <stdio.h>
static uint32_t U(int x){ return (uint32_t)x; }
int main(){
    uint32_t a = f32_from_int32(3);
    uint32_t b = f32_from_int32(5);
    uint32_t c = f32_add(a,b);
    uint32_t d = f32_mul(a,b);
    uint32_t e = f32_div(d,b);
    printf("c(add)=0x%08X\n", c);
    printf("d(mul)=0x%08X\n", d);
    printf("e(div)=0x%08X\n", e);
    printf("to_int=%d\n", f32_to_int32_trunc(c));
    (void)U; return 0;
}
#endif


// ==========================
//  Расширенные сравнения f32
// ==========================
bool f32_gt(uint32_t a, uint32_t b){ return f32_lt(b,a); }
bool f32_ge(uint32_t a, uint32_t b){ return f32_le(b,a); }
bool f32_unordered(uint32_t a, uint32_t b){ return f32_is_nan(a) || f32_is_nan(b); }

// ==========================
//  Реализация IEEE-754 double
// ==========================
// Требования: 64-битные целые обязательны; для умножения/деления используется unsigned __int128.
// Если ваш компилятор не поддерживает __int128, дайте знать — добавлю программную 128-битную арифметику.

#define F64_SIGN_MASK 0x8000000000000000ull
#define F64_EXP_MASK  0x7FF0000000000000ull
#define F64_FRAC_MASK 0x000FFFFFFFFFFFFFull
#define F64_EXP_BIAS  1023
#define F64_EXP_INF_NAN 2047

static inline uint64_t f64_pack(bool sign, int32_t exp, uint64_t frac){
    return (sign ? F64_SIGN_MASK : 0) | ((uint64_t)exp << 52) | (frac & F64_FRAC_MASK);
}
static inline bool f64_sign(uint64_t a){ return (a & F64_SIGN_MASK) != 0; }
static inline int32_t f64_exp(uint64_t a){ return (int32_t)((a & F64_EXP_MASK) >> 52); }
static inline uint64_t f64_frac(uint64_t a){ return a & F64_FRAC_MASK; }
static inline bool f64_is_nan(uint64_t a){ return (f64_exp(a)==F64_EXP_INF_NAN) && (f64_frac(a)!=0); }
static inline bool f64_is_inf(uint64_t a){ return (f64_exp(a)==F64_EXP_INF_NAN) && (f64_frac(a)==0); }
static inline bool f64_is_zero(uint64_t a){ return (a & ~F64_SIGN_MASK) == 0; }
static inline uint64_t f64_qnan(void){ return 0x7FF8000000000000ull; }

static inline void f64_norm_subnormal(uint64_t frac, int32_t *exp, uint64_t *mant){
    int shift = clz64((uint64_t)frac) - 11; // 63-52=11
    if (shift < 0) shift = 0;
    *mant = frac << (shift + 1); // включаем скрытую 1 в 53-й бит
    *exp  = 1 - shift;
}

static inline uint64_t f64_round_pack(bool sign, int32_t exp, unsigned __int128 mant){
    // mant: [1][52] + GRS (итого >= 56 бит), G=2-й, R=1-й, S=0-й
    uint32_t guard = (uint32_t)((mant >> 2) & 1u);
    uint32_t round = (uint32_t)((mant >> 1) & 1u);
    uint32_t sticky= (uint32_t)(mant & 1u);
    uint64_t frac = (uint64_t)(mant >> 3); // 53 бита с ведущей 1 или 0

    bool increment = false;
    if (guard){ if (round || sticky || (frac & 1u)) increment = true; }
    if (increment){
        frac += 1ull;
        if (frac == (1ull<<53)){
            frac >>= 1; exp += 1; // перенос
        }
    }
    if (exp >= 0x7FF){ return f64_pack(sign, F64_EXP_INF_NAN, 0); }
    if (exp <= 0){
        if (exp < -53) return f64_pack(sign, 0, 0);
        int shift = 1 - exp;
        unsigned __int128 m = ((unsigned __int128)frac) << 3;
        unsigned __int128 shifted = m >> shift;
        unsigned __int128 lost = m & ((((unsigned __int128)1)<<shift) - 1);
        unsigned __int128 sticky2 = lost ? 1 : 0;
        mant = (shifted & ~((unsigned __int128)7)) | (shifted & 4) | (shifted & 2) | sticky2;
        uint32_t g2 = (uint32_t)((mant>>2)&1u); uint32_t r2=(uint32_t)((mant>>1)&1u); uint32_t s2=(uint32_t)(mant&1u);
        frac = (uint64_t)(mant>>3);
        bool inc2=false; if (g2){ if (r2||s2||(frac&1u)) inc2=true; }
        if (inc2){
            frac += 1ull; if (frac == (1ull<<53)) return f64_pack(sign, 1, 0);
        }
        return f64_pack(sign, 0, frac & F64_FRAC_MASK);
    }
    return f64_pack(sign, exp, frac & F64_FRAC_MASK);
}

// --- Преобразования int64 <-> double ---
uint64_t f64_from_int64(int64_t a){
    if (a == 0) return 0;
    bool sign = a < 0;
    uint64_t mag = sign ? (uint64_t)(-( __int128)a) : (uint64_t)a; // избежать UB на INT64_MIN
    int lz = clz64(mag);
    int msb = 63 - lz;
    int32_t exp = msb + F64_EXP_BIAS;
    unsigned __int128 mant = (unsigned __int128)mag << (52 + 3 - msb);
    return f64_round_pack(sign, exp, mant);
}

int64_t f64_to_int64_trunc(uint64_t a){
    if (f64_is_nan(a) || f64_is_inf(a)) return (int64_t)0x8000000000000000ull;
    int32_t exp = f64_exp(a); uint64_t frac = f64_frac(a); bool sign=f64_sign(a);
    uint64_t mant = exp ? (1ull<<52)|frac : frac; int32_t e = exp - F64_EXP_BIAS - 52;
    __int128 val;
    if (e >= 0){ if (e > 63) return (int64_t)0x8000000000000000ull; val = ( (__int128)mant) << e; }
    else { if (-e > 127) return 0; val = ( (__int128)mant) >> (-e); }
    if (sign) val = -val;
    if (val > (__int128)0x7FFFFFFFFFFFFFFFLL || val < -(((__int128)1)<<63)) return (int64_t)0x8000000000000000ull;
    return (int64_t)val;
}

// --- Сложение/вычитание ---
static uint64_t f64_add_core(uint64_t a, uint64_t b, bool sub){
    if (f64_is_nan(a) || f64_is_nan(b)) return f64_qnan();
    if (f64_is_inf(a) || f64_is_inf(b)){
        if (f64_is_inf(a) && f64_is_inf(b)){
            if ((f64_sign(a) ^ f64_sign(b)) ^ sub) return f64_qnan();
        }
        return f64_is_inf(a) ? a : (sub ? (b ^ F64_SIGN_MASK) : b);
    }
    bool signA=f64_sign(a), signB=f64_sign(b)^sub;
    int32_t expA=f64_exp(a), expB=f64_exp(b);
    uint64_t fracA=f64_frac(a), fracB=f64_frac(b);

    unsigned __int128 mantA=0, mantB=0; // 53+3 бита
    if (expA){ mantA = ((unsigned __int128)((1ull<<52)|fracA))<<3; }
    else if (fracA){ int32_t e; uint64_t m; f64_norm_subnormal(fracA,&e,&m); expA=e; mantA=((unsigned __int128)m)<<3; }
    if (expB){ mantB = ((unsigned __int128)((1ull<<52)|fracB))<<3; }
    else if (fracB){ int32_t e; uint64_t m; f64_norm_subnormal(fracB,&e,&m); expB=e; mantB=((unsigned __int128)m)<<3; }

    int32_t exp = expA;
    if (expA < expB){ int shift = expB-expA; if (shift>63) shift=63; unsigned __int128 lost = mantA & ((((unsigned __int128)1)<<shift)-1); mantA = (mantA>>shift) | (lost?1:0); exp=expB; }
    else if (expB < expA){ int shift = expA-expB; if (shift>63) shift=63; unsigned __int128 lost = mantB & ((((unsigned __int128)1)<<shift)-1); mantB = (mantB>>shift) | (lost?1:0); exp=expA; }

    unsigned __int128 mant; bool sign;
    if (signA==signB){
        mant = mantA + mantB; sign=signA;
        if (mant & (((unsigned __int128)1) << (52+4))){ mant = (mant>>1) | (mant&1); exp += 1; }
    } else {
        if (mantA > mantB){ mant = mantA-mantB; sign=signA; }
        else if (mantB > mantA){ mant = mantB-mantA; sign=signB; }
        else { return f64_pack(false,0,0); }
        // нормализация вверх
        int lz = 127 - (int)(8*sizeof(unsigned __int128) - 1 - 0); // грубо; уточним через цикл
        // Найдём позицию MSB
        int msb = -1; for (int i=127;i>=0;--i){ if ((mant>>i)&1){ msb=i; break; } }
        int target = 52+3; if (msb < target){ int d = target - msb; mant <<= d; exp -= d; }
    }
    return f64_round_pack(sign, exp, mant);
}

uint64_t f64_add(uint64_t a, uint64_t b){ return f64_add_core(a,b,false); }
uint64_t f64_sub(uint64_t a, uint64_t b){ return f64_add_core(a,b,true); }

// --- Умножение ---
uint64_t f64_mul(uint64_t a, uint64_t b){
    if (f64_is_nan(a) || f64_is_nan(b)) return f64_qnan();
    bool sign = f64_sign(a) ^ f64_sign(b);
    if (f64_is_inf(a) || f64_is_inf(b)){ if (f64_is_zero(a) || f64_is_zero(b)) return f64_qnan(); return f64_pack(sign, F64_EXP_INF_NAN, 0); }
    if (f64_is_zero(a) || f64_is_zero(b)) return f64_pack(sign, 0, 0);

    int32_t expA=f64_exp(a), expB=f64_exp(b); uint64_t fracA=f64_frac(a), fracB=f64_frac(b);
    uint64_t mantA, mantB; int32_t eA=expA,eB=expB;
    if (expA){ mantA=(1ull<<52)|fracA; } else { f64_norm_subnormal(fracA,&eA,&mantA); }
    if (expB){ mantB=(1ull<<52)|fracB; } else { f64_norm_subnormal(fracB,&eB,&mantB); }
    int32_t exp = eA + eB - F64_EXP_BIAS;

    unsigned __int128 prod = (unsigned __int128)mantA * (unsigned __int128)mantB; // 106 бит
    if (!(prod & ((unsigned __int128)1<<105))){ prod <<= 1; exp -= 1; }
    unsigned __int128 mant = prod >> (105 - 52 - 3); // 53+3 бита
    return f64_round_pack(sign, exp, mant);
}

// --- Деление ---
uint64_t f64_div(uint64_t a, uint64_t b){
    if (f64_is_nan(a) || f64_is_nan(b)) return f64_qnan();
    bool sign = f64_sign(a) ^ f64_sign(b);
    if (f64_is_zero(b)){ if (f64_is_zero(a)) return f64_qnan(); return f64_pack(sign, F64_EXP_INF_NAN, 0); }
    if (f64_is_inf(b)) return f64_pack(sign, 0, 0);
    if (f64_is_inf(a)) return f64_pack(sign, F64_EXP_INF_NAN, 0);
    if (f64_is_zero(a)) return f64_pack(sign, 0, 0);

    int32_t expA=f64_exp(a), expB=f64_exp(b); uint64_t fracA=f64_frac(a), fracB=f64_frac(b);
    uint64_t mantA, mantB; int32_t eA=expA, eB=expB;
    if (expA){ mantA=(1ull<<52)|fracA; } else { f64_norm_subnormal(fracA,&eA,&mantA); }
    if (expB){ mantB=(1ull<<52)|fracB; } else { f64_norm_subnormal(fracB,&eB,&mantB); }

    int32_t exp = eA - eB + F64_EXP_BIAS;

    unsigned __int128 dividend = ((unsigned __int128)mantA) << (52 + 3);
    uint64_t divisor = mantB;
    unsigned __int128 quot = dividend / divisor;
    unsigned __int128 rem  = dividend % divisor;

    int msb = 0; for (int i=127;i>=0;--i){ if ((quot>>i)&1){ msb=i; break; } }
    if (msb < (52+3)){ int d=(52+3)-msb; quot <<= d; exp -= d; }
    else if (msb > (52+3)){ int d=msb-(52+3); unsigned __int128 lost = quot & ((((unsigned __int128)1)<<d)-1); quot = (quot>>d) | (lost?1:0); exp += d; }

    if (rem) quot |= 1; // sticky
    return f64_round_pack(sign, exp, quot);
}

// --- Сравнения ---
bool f64_eq(uint64_t a, uint64_t b){ if (f64_is_nan(a) || f64_is_nan(b)) return false; return a==b || (f64_is_zero(a) && f64_is_zero(b)); }
bool f64_lt(uint64_t a, uint64_t b){ if (f64_is_nan(a) || f64_is_nan(b)) return false; if (a==b) return false; bool sa=f64_sign(a), sb=f64_sign(b); if (sa!=sb) return sa; uint64_t aa=a & ~F64_SIGN_MASK, bb=b & ~F64_SIGN_MASK; bool less=(aa<bb); return sa ? !less : less; }
bool f64_le(uint64_t a, uint64_t b){ return f64_lt(a,b) || f64_eq(a,b); }
bool f64_gt(uint64_t a, uint64_t b){ return f64_lt(b,a); }
bool f64_ge(uint64_t a, uint64_t b){ return f64_le(b,a); }
bool f64_unordered(uint64_t a, uint64_t b){ return f64_is_nan(a) || f64_is_nan(b); }
