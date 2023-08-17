#ifndef BIGINT_H_
#define BIGINT_H_

#include <stdbool.h>
#include <stddef.h>

#ifndef BIGINT_ASSERT
#include <assert.h>
#define BIGINT_ASSERT assert
#endif // BIGINT_ASSERT

typedef struct {
    char* digits;
    size_t length;
} bigint;

bigint bigint_new(char* value);
void bigint_realloc(bigint *num, size_t length);
bigint bigint_clone(const bigint *num);
void bigint_free(bigint *num);

void bigint_add(bigint *a, const bigint *b);
void bigint_sub(bigint *a, const bigint *b);
void bigint_mul(bigint *a, const bigint *b);

void bigint_pow(bigint *a, const bigint *b);

bool bigint_eq(const bigint *a, const bigint *b);
bool bigint_gt(const bigint *a, const bigint *b);
bool bigint_ge(const bigint *a, const bigint *b);

void bigint_print(const bigint *num);

#endif // BIGINT_H_

#ifdef BIGINT_IMPLEMENTATION

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

bigint bigint_new(char* value) {
    size_t value_len = strlen(value);
    for (size_t i = 0; i < value_len; i++) {
        if (!isdigit(value[i])) {
            return (bigint) { NULL, 0 };
        }
    }

    bigint number;
    number.length = value_len; 
    number.digits = (char*) malloc(number.length * sizeof(char));
    if (number.digits == NULL) {
        BIGINT_ASSERT(0 && "Can't allocate memory");
    }
    for (size_t i = 0; i < number.length; i++) {
        number.digits[i] = value[number.length-1-i]-'0'; 
    }
    return number;
}

void bigint_realloc(bigint *num, size_t length) {
    if (num->length == length) return;
    if (length == 0) {
        bigint_free(num);
        return;
    }

    char* tmp = (char*) malloc(sizeof(char) * num->length);
    for (size_t i = 0; i < num->length; i++) {
        tmp[i] = num->digits[i];
    }
    
    size_t old_len = num->length;

    num->digits = (char*) realloc(num->digits, length * sizeof(char)); 
    num->length = length;
    if (num->digits == NULL) {
        BIGINT_ASSERT(0 && "Can't allocate memory");
    }
    if (num->length > old_len) {
        for (size_t i = 0; i < old_len; i++) {
            num->digits[i] = tmp[i];
        }
        memset(num->digits+old_len, 0, num->length - old_len);
    } else {
        for (size_t i = old_len - num->length; i < old_len; i++) {
            num->digits[i-(old_len - num->length)] = tmp[i];
        }
    }
        
}

bigint bigint_clone(const bigint *num) {
    bigint cloned;
    cloned.length = num->length; 
    cloned.digits = (char*) malloc(cloned.length * sizeof(char));
    if (cloned.digits == NULL) {
        BIGINT_ASSERT(0 && "Can't allocate memory");
    }

    for (size_t i = 0; i < num->length; i++) {
        cloned.digits[i] = num->digits[i];
    }

    return cloned;
}

void bigint_free(bigint *num) {
    num->length = 0;
    free(num->digits);
    num->digits = NULL;
}

void bigint_print(const bigint *num) {
    for (size_t i = num->length; i > 0; i--) {
        printf("%d", num->digits[i-1]);
    }
    printf("\n");
}

bool bigint_eq(const bigint *a, const bigint *b) {
    if (a->length == 0 || b->length == 0 || a->digits == NULL || b->digits == NULL) {
        return false;
    }

    size_t a_end, b_end, i;
    
    i = a->length;
    while (i > 0 && a->digits[i-1] == 0) i--;
    a_end = i; 

    i = b->length;
    while (i > 0 && b->digits[i-1] == 0) i--;
    b_end = i; 

    if (a_end == 0 || b_end == 0) return a_end == b_end;
    if (a_end != b_end) return false;

    for (size_t i = a_end; i > 0; i--) { 
        if (a->digits[i-1] != b->digits[i-1]) 
            return false;
    }
    return true;
}

bool bigint_gt(const bigint *a, const bigint *b) {
    if (a->length == 0 || b->length == 0 || a->digits == NULL || b->digits == NULL) {
        return false;
    }

    size_t a_end, b_end, i;
    
    i = a->length;
    while (i > 0 && a->digits[i-1] == 0) i--;
    a_end = i; 

    i = b->length;
    while (i > 0 && b->digits[i-1] == 0) i--;
    b_end = i; 

    if (a_end != b_end || a_end == 0 || b_end == 0) {
        return a_end > b_end;
    }

    for (size_t i = a_end; i > 0; i--) { 
        if (a->digits[i-1] != b->digits[i-1]) 
            return a->digits[i-1] > b->digits[i-1];
    }

    return false;
}

bool bigint_ge(const bigint *a, const bigint *b) {
    return bigint_eq(a, b) || bigint_gt(a, b);
}

void bigint_add(bigint *a, const bigint *b) {
    if (b->length > a->length) {
        bigint_realloc(a, b->length);       
    }
    int sum;
    int carry = 0;
    size_t i = 0;
    while (i < a->length || carry != 0) {
        if (i >= a->length) {
            bigint_realloc(a, i+1); 
        }
        sum = (b->length > i ? b->digits[i] : 0) + a->digits[i] + carry;
        if (sum > 9) {
            carry = sum / 10;
            sum %= 10;
        } else {
            carry = 0;
        }
        a->digits[i] = sum;
        i++;
    }
}

/*
 * Subtraction by addition: https://www.mathsisfun.com/numbers/subtraction-by-addition.html
 */
void bigint_sub(bigint *a, const bigint *b) {
    BIGINT_ASSERT(bigint_ge(a, b) && "Negative numbers are not supported yet");

    if (bigint_eq(a, b)) {
        bigint_free(a);
        *a = bigint_new("0");
        return;
    }

    bigint b_complement = bigint_clone(b);

    bool looking_for_lsd = true;
    for (size_t i = 0; i < b->length; i++) {
        if (looking_for_lsd) {
            if (b->digits[i] != 0) {
                looking_for_lsd = false;
                b_complement.digits[i] = 10 - b->digits[i];
            } else {
                b_complement.digits[i] = 0;
            }
        } else {
            b_complement.digits[i] = 9 - b->digits[i];
        }
    }

    bigint zero = bigint_new("0");
    if (bigint_eq(&b_complement, &zero)) {
        bigint_free(&b_complement);
        b_complement = bigint_new("10");
    }
    bigint_free(&zero);

    // we need to pad 'b_complement' to become the same length as 'a'
    if (b_complement.length < a->length) {
        int old_len = b_complement.length;
        bigint_realloc(&b_complement, a->length);
        for (size_t i = old_len; i < a->length; i++) {
            b_complement.digits[i] = 9;
        }
    }

    bigint_add(a, &b_complement);
    a->digits[a->length-1] = 0;
    a->length--;

    bigint_free(&b_complement);

    // remove leading zeros
    size_t i = a->length;
    while (i > 0 && a->digits[i-1] == 0) i--; 
    a->length = i == 0 ? 1 : i;
    bigint_realloc(a, a->length);
}

void bigint_mul(bigint *a, const bigint *b) {
    bigint zero = bigint_new("0");
    if (bigint_eq(b, &zero)) {
        free(a->digits);
        *a = bigint_new("0");
        bigint_free(&zero);
        return;
    } else if (bigint_eq(a, &zero)) {
        return;
    }
    bigint_free(&zero);

    bigint b_clone = bigint_clone(b);
    bigint a_clone = bigint_clone(a); 
    bigint one = bigint_new("1");

    while (bigint_gt(&b_clone, &one)) {
        bigint_add(a, &a_clone);
        bigint_sub(&b_clone, &one);
    }

    bigint_free(&one);
    bigint_free(&a_clone);
    bigint_free(&b_clone);
}

void bigint_pow(bigint *a, const bigint *b) {
    bigint zero = bigint_new("0");
    if (bigint_eq(b, &zero)) {
        free(a->digits);
        *a = bigint_new("1");
        bigint_free(&zero);
        return;
    }
    bigint_free(&zero);

    bigint b_clone = bigint_clone(b);
    bigint a_clone = bigint_clone(a); 
    bigint one = bigint_new("1");
   
    while (bigint_gt(&b_clone, &one)) {
        bigint_mul(a, &a_clone);
        bigint_sub(&b_clone, &one);
    }

    bigint_free(&one);
    bigint_free(&a_clone);
    bigint_free(&b_clone);
}

#endif // BIGINT_IMPLEMENTATION
