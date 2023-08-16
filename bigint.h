#ifndef BIGINT_H_
#define BIGINT_H_

#include <stdbool.h>

typedef struct {
    char* digits;
    int length;
} bigint;

bigint bigint_new(char* value);
void bigint_realloc(bigint *num, int length);
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
#include <assert.h>

bigint bigint_new(char* value) {
    bigint number;
    number.length = strlen(value);
    number.digits = malloc(number.length * sizeof(char));
    if (number.digits == NULL) {
        assert(0 && "Can't allocate memory");
    }
    for (int i = 0; i < number.length; i++) {
        number.digits[i] = value[number.length-1-i]-'0'; 
    }
    return number;
}

void bigint_realloc(bigint *num, int length) {
    if (num->length == length) return;

    char* tmp = malloc(sizeof(char) * num->length);
    for (int i = 0; i < num->length; i++) {
        tmp[i] = num->digits[i];
    }
    
    int old_len = num->length;

    num->digits = realloc(num->digits, length * sizeof(char)); 
    num->length = length;
    if (num->digits == NULL) {
        assert(0 && "Can't allocate memory");
    }
    if (num->length > old_len) {
        for (int i = 0; i < old_len; i++) {
            num->digits[i] = tmp[i];
        }
        memset(num->digits+old_len, 0, num->length - old_len);
    } else {
        for (int i = old_len - num->length; i < old_len; i++) {
            num->digits[i-(old_len - num->length)] = tmp[i];
        }
    }
        
}

bigint bigint_clone(const bigint *num) {
    bigint new;
    new.length = num->length; 
    new.digits = malloc(new.length * sizeof(char));
    if (new.digits == NULL) {
        assert(0 && "Can't allocate memory");
    }

    for (int i = 0; i < num->length; i++) {
        new.digits[i] = num->digits[i];
    }

    return new;
}

void bigint_free(bigint *num) {
    num->length = 0;
    free(num->digits);
}

void bigint_print(const bigint *num) {
    for (int i = num->length-1; i >= 0; i--) {
        printf("%d", num->digits[i]);
    }
    printf("\n");
}

bool bigint_eq(const bigint *a, const bigint *b) {
    int a_end, b_end, i;
    
    i = a->length-1;
    while (i >= 0 && a->digits[i] == 0) i--;
    a_end = i < 0 ? 0 : i+1; 

    i = b->length-1;
    while (i >= 0 && b->digits[i] == 0) i--;
    b_end = i < 0 ? 0 : i+1; 

    if (a_end == 0 || b_end == 0) return a_end == b_end;
    if (a_end != b_end) return false;

    for (int i = a_end; i >= 0; i--) { 
        if (a->digits[i] != b->digits[i]) 
            return false;
    }
    return true;
}

bool bigint_gt(const bigint *a, const bigint *b) {
    int a_end, b_end, i;
    
    i = a->length-1;
    while (i >= 0 && a->digits[i] == 0) i--;
    a_end = i < 0 ? 0 : i+1; 

    i = b->length-1;
    while (i >= 0 && b->digits[i] == 0) i--;
    b_end = i < 0 ? 0 : i+1; 

    if (a_end != b_end || a_end == 0 || b_end == 0) {
        return a_end > b_end;
    }

    for (int i = a_end; i >= 0; i--) { 
        if (a->digits[i] != b->digits[i]) 
            return a->digits[i] > b->digits[i];
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
    int i = 0;
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
    assert(bigint_ge(a, b) && "Negative numbers are not supported yet");

    if (bigint_eq(a, b)) {
        bigint_free(a);
        *a = bigint_new("0");
        return;
    }

    bigint b_complement = bigint_clone(b);

    bool looking_for_lsd = true;
    for (int i = 0; i < b->length; i++) {
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

    // we need to pad 'b_complement' to become the same length as 'a'
    if (b_complement.length < a->length) {
        int old_len = b_complement.length;
        bigint_realloc(&b_complement, a->length);
        for (int i = old_len; i < a->length; i++) {
            b_complement.digits[i] = 9;
        }
    }

    bigint_add(a, &b_complement);
    a->digits[a->length-1] = 0;
    a->length--;

    // remove leading zeros
    int i = a->length-1;
    while (i >= 0 && a->digits[i] == 0) i--; 
    a->length = i < 0 ? 1 : i+1;
    bigint_realloc(a, a->length);
}

void bigint_mul(bigint *a, const bigint *b) {
    bigint zero = bigint_new("0");
    if (bigint_eq(b, &zero)) {
        free(a->digits);
        *a = bigint_new("0");
        bigint_free(&zero);
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
