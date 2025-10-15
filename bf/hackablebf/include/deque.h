#ifndef PARAMETRIZED_DEQUE_H__
#define PARAMETRIZED_DEQUE_H__

/**
* Вспомогательные макросы для работы с двусторонними очередями.
*/

#include <vec.h>

#define DEQUE_TYPE(ETYPE) ETYPE##_deque_t
#define DECL_DEQUE(ETYPE)                          \
    typedef ETYPE __##ETYPE##_deque_element;       \
    DECL_VEC(__##ETYPE##_deque_element);           \
    typedef struct DEQUE_TYPE(ETYPE) {             \
        VEC_TYPE(__##ETYPE##_deque_element) front; \
        VEC_TYPE(__##ETYPE##_deque_element) back;  \
    } DEQUE_TYPE(ETYPE)

#define DEQUE_INIT() { .front = VEC_INIT(), .back = VEC_INIT() }

#define DEQUE_IS_EMPTY(D) ((D).front.length == 0 && (D).back.length == 0)

#define DEQUE_LENGTH(D) ((D).front.length + (D).back.length)

// Добавление в начало
#define DEQUE_PUSH_FRONT(D, X, RES) \
    VEC_PUSH((D).front, (X), (RES))

// Добавление в конец
#define DEQUE_PUSH_BACK(D, X, RES) \
    VEC_PUSH((D).back, (X), (RES))

// Получение первого элемента
#define DEQUE_FRONT(D, OUT, RES)                                        \
    do {                                                                \
        if ((D).front.length > 0) {                                     \
            (OUT) = VEC_GET_UNCHECKED((D).front, (D).front.length - 1); \
            (RES) = true;                                               \
        } else if ((D).back.length > 0) {                               \
            (OUT) = VEC_GET_UNCHECKED((D).back, 0);                     \
            (RES) = true;                                               \
        } else {                                                        \
            (RES) = false;                                              \
        }                                                               \
    } while(0)

// Получение последнего элемента
#define DEQUE_BACK(D, OUT, RES)                                       \
    do {                                                              \
        if ((D).back.length > 0) {                                    \
            (OUT) = VEC_GET_UNCHECKED((D).back, (D).back.length - 1); \
            (RES) = true;                                             \
        } else if ((D).front.length > 0) {                            \
            (OUT) = VEC_GET_UNCHECKED((D).front, 0);                  \
            (RES) = true;                                             \
        } else {                                                      \
            (RES) = false;                                            \
        }                                                             \
    } while(0)

#define DEQUE_POP_FRONT(D, OUT, RES)                                        \
    do {                                                                    \
        /* Проверка предусловия о непустоте двусторонней очереди:        */ \
        if (DEQUE_IS_EMPTY(D)) {                                            \
            (RES) = false;                                                  \
            break;                                                          \
        }                                                                   \
                                                                            \
        /* Если front был пуст, значит нужно перебалансировать очередь   */ \
        /* через сдвиг половины содержимого back во front, после чего    */ \
        /* сделать VEC_POP из front. Есть так же специальный случай,     */ \
        /* когда в back всего один элемент, в этом случае просто делаем  */ \
        /* VEC_POP из back и выходим заранее.                            */ \
        if ((D).front.length == 0) {                                        \
            if ((D).back.length == 1) {                                     \
                VEC_POP((D).back, OUT, RES);                                \
                break;                                                      \
            }                                                               \
            size_t __half = (D).back.length / 2;                            \
            VEC_RESERVE((D).front, __half, RES);                            \
            if (!(RES)) break;                                              \
            for(size_t __i = 0; __i < __half; __i++) {                      \
                (D).front.data[__i] = (D).back.data[__half-1-__i];          \
            }                                                               \
            size_t __to_move = (D).back.length - __half;                    \
            __to_move *= sizeof((D).back.data[0]);                          \
            memmove((D).back.data, (D).back.data + __half, __to_move);      \
            (D).front.length += __half;                                     \
            (D).back.length -= __half;                                      \
        }                                                                   \
        VEC_POP((D).front, OUT, RES);                                       \
    } while(0)

#define DEQUE_POP_BACK(D, OUT, RES)                                      \
    do {                                                                 \
        if (DEQUE_IS_EMPTY(D)) {                                         \
            (RES) = false;                                               \
            break;                                                       \
        }                                                                \
                                                                         \
        /* Если back был пуст, значит нужно перебалансировать очередь */ \
        /* через сдвиг половины содержимого front в back, после чего  */ \
        /* сделать VEC_POP из back. Есть так же специальный случай,   */ \
        /* когда во front всего один элемент, в этом случае просто    */ \
        /* делаем VEC_POP из front и выходим заранее.                 */ \
        if ((D).back.length == 0) {                                      \
            if ((D).front.length == 1) {                                 \
                VEC_POP((D).front, OUT, RES);                            \
                break;                                                   \
            }                                                            \
            size_t __half = (D).front.length / 2;                        \
            VEC_RESERVE((D).back, __half, RES);                          \
            if (!(RES)) break;                                           \
            for(size_t __i = 0; __i < __half; __i++) {                     \
                (D).back.data[__i] = (D).front.data[__half-1-__i];       \
            }                                                            \
            size_t __to_move = (D).front.length - __half;                \
            __to_move *= sizeof((D).front.data[0]);                      \
            memmove((D).front.data, (D).front.data + __half, __to_move); \
            (D).back.length += __half;                                   \
            (D).front.length -= __half;                                  \
        }                                                                \
        VEC_POP((D).back, OUT, RES);                                     \
    } while(0)

/**
* Безопасно получает элемент дека по индексу.
* D - дек
* I - индекс (от 0 до length-1)
* OUT - выходное значение
* RES - результат (успех/ошибка)
*/
#define DEQUE_GET(D, I, OUT, RES)                                             \
    do {                                                                      \
        (RES) = false;                                                        \
        if (DEQUE_IS_EMPTY(D) || (I) >= DEQUE_LENGTH(D)) break;               \
        if ((I) < (D).front.length) {                                         \
            /* Элемент в левом векторе (обратный порядок) */                  \
            (OUT) = VEC_GET_UNCHECKED((D).front, (D).front.length - 1 - (I)); \
        } else {                                                              \
            /* Элемент в правом векторе (прямой порядок) */                   \
            (OUT) = VEC_GET_UNCHECKED((D).back, (I) - (D).front.length);      \
        }                                                                     \
        (RES) = true;                                                         \
    } while(0)

#define DEQUE_CLEAR(D)        \
    do {                      \
        VEC_CLEAR((D).front); \
        VEC_CLEAR((D).back);  \
    } while(0)

#define DEQUE_FREE(D)        \
    do {                     \
        VEC_FREE((D).front); \
        VEC_FREE((D).back);  \
    } while(0)

#endif
