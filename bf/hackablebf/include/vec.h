#ifndef PARAMETRIZED_VEC_H__
#define PARAMETRIZED_VEC_H__

/**
* Вспомогательные макросы для работы с динамическими массивами.
*/

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

/**
* Объявляет тип вектора для элементов типа ETYPE.
* Пример: DECL_VEC(int) → объявляет struct int_vec_t.
*/
#define VEC_TYPE(ETYPE) ETYPE##_vec_t
#define DECL_VEC(ETYPE)              \
    typedef struct VEC_TYPE(ETYPE) { \
        ETYPE* data;                 \
        size_t length;               \
        size_t capacity;             \
    } VEC_TYPE(ETYPE)

/**
* Инициализирует пустой вектор (без выделения памяти).
*/
#define VEC_INIT() { .data = NULL, .length = 0, .capacity = 0 }

/**
 * Резервирует точное capacity в векторе.
 * V - вектор
 * C - capacity, которое планируется зарезервировать
 * R - результат (успех/ошибка)
 * Если C < V.length, резервируется V.length
 * Если резервирование не удалось, поля capacity и data у вектора не изменяются.
 */
#define VEC_RESERVE(V, C, RES)                                                               \
    do {                                                                                     \
        size_t __reserved_capacity = (C);                                                    \
        if (__reserved_capacity < (V).length) {                                              \
            __reserved_capacity = (V).length;                                                \
        }                                                                                    \
        if ((V).capacity != __reserved_capacity) {                                           \
            void* __new_data = realloc((V).data, __reserved_capacity * sizeof((V).data[0])); \
            if (__new_data != NULL) {                                                        \
                (V).data = __new_data;                                                       \
                (V).capacity = __reserved_capacity;                                          \
                (RES) = true;                                                                \
            } else { (RES) = false; }                                                        \
        } else { (RES) = true; }                                                             \
    } while (0)

/**
* Освобождает неиспользуемую память вектора.
* V — вектор, RES — результат.
*/
#define VEC_SHRINK_TO_FIT(V, RES) VEC_RESERVE(V, (V).length, RES)

/**
* Рассчитывает новую ёмкость для вектора.
* C — текущая ёмкость, NEW_C — выходная новая ёмкость,
* L — текущая длина, RES — результат (успех/ошибка).
* Ошибка возникает в случае если новая ёмкость выходит за рамки SIZE_MAX
*/
#define GROW_CAPACITY(C, NEW_C, L, RES)       \
    do {                                      \
        (RES) = true;                         \
        (NEW_C) = (C);                        \
        if ((L) >= (C)) {                     \
            if ((NEW_C) < 8)                  \
                (NEW_C) = 8;                  \
            while ((L) >= (NEW_C)) {          \
                if ((NEW_C) > SIZE_MAX / 2) { \
                    (RES) = false;            \
                    break;                    \
                }                             \
                (NEW_C) *= 2;                 \
            }                                 \
        }                                     \
    } while (0)

/**
* Увеличивает ёмкость вектора при необходимости.
* V — вектор, RES — результат (успех/ошибка).
*/
#define VEC_GROW(V, RES)                                              \
    do {                                                              \
        (RES) = true;                                                 \
        size_t __new_capacity;                                        \
        GROW_CAPACITY((V).capacity, __new_capacity, (V).length, RES); \
        if ((RES)) {                                                  \
            VEC_RESERVE(V, __new_capacity, RES);                      \
        }                                                             \
    } while(0)

/**
* Добавляет элемент в конец вектора.
* V — вектор, X — элемент, RES — результат.
*/
#define VEC_PUSH(V, X, RES)               \
    do {                                  \
        VEC_GROW(V, RES);                 \
        if ((RES)) {                      \
            (V).data[(V).length++] = (X); \
        }                                 \
    } while(0)

/**
* Получает элемент по индексу без проверок.
* Используйте с осторожностью! Только для случаев когда проверка
* индекса уже осуществлена снаружи вызова посредством доступа к полю length
*/
#define VEC_GET_UNCHECKED(V, I) (V).data[(I)]

/**
 * Безопасно получает элемент по индексу.
 * V — вектор, I — индекс, OUT — выходное значение, RES — результат.
 */
#define VEC_GET(V, I, OUT, RES)              \
    do {                                     \
        if ((I) < 0 || (I) >= (V).length) {  \
            (RES) = false;                   \
        } else {                             \
            (RES) = true;                    \
            (OUT) = VEC_GET_UNCHECKED(V, I); \
        }                                    \
    } while (0)

/**
* Получает указатели начального и конечного элемента вектора для итерирования.
* V - вектор
* S - куда поместить указатель на первый элемент
* E - куда поместить указатель последний элемент
* R - результат (успех/ошибка)
* Если вектор пустой, результат будет ошибочным
*/
#define VEC_GET_ITERATOR(V, S, E, RES)             \
    do {                                           \
        if ((V).data == NULL || (V).length == 0) { \
            (RES) = false;                         \
        } else {                                   \
            (S) = (V).data;                        \
            (E) = (V).data + V.length-1;           \
            (RES) = true;                          \
        }                                          \
    } while(0)

/**
* Удаляет элемент по индексу, сдвигая остальные.
* Не изменяет capacity.
* V — вектор, I — индекс (должен быть < length), RES — результат.
*/
#define VEC_REMOVE(V, I, RES)                                                 \
    do {                                                                      \
        if ((V).length == 0 || (I) >= (V).length) {                           \
            (RES) = false;                                                    \
        } else {                                                              \
            (RES) = true;                                                     \
            size_t __to_move =  sizeof((V).data[0]) * ((V).length - 1 - (I)); \
            if (__to_move > 0)                                                \
                memmove((V).data + (I), (V).data + (I) + 1, __to_move);       \
            --(V).length;                                                     \
        }                                                                     \
    } while(0)

/**
* Удаляет последний элемент и возвращает его.
* Не изменяет capacity.
* V — вектор, OUT — выходное значение, RES — результат.
*/
#define VEC_POP(V, OUT, RES)                            \
    do {                                                \
        if ((V).length == 0) {                          \
            (RES) = false;                              \
        } else {                                        \
            (RES) = true;                               \
            (OUT) = VEC_GET_UNCHECKED(V, (V).length-1); \
            --(V).length;                               \
        }                                               \
    } while (0)

/**
* Очищает вектор без освобождения памяти.
*/
#define VEC_CLEAR(V) do { (V).length = 0; } while(0)

/**
* Освобождает память вектора и обнуляет его поля.
*/
#define VEC_FREE(V)             \
    do {                        \
        if ((V).data != NULL) { \
            free((V).data);     \
            (V).data = NULL;    \
            (V).length = 0;     \
            (V).capacity = 0;   \
        }                       \
    } while(0)

#endif
