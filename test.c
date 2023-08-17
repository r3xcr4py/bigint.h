#include <CUnit/CUnit.h>
#include <CUnit/Basic.h>

#define BIGINT_IMPLEMENTATION
#include "bigint.h"

void test_bigint_new(void) {
    bigint num = bigint_new("abc");
    CU_ASSERT_PTR_NULL(num.digits);
    CU_ASSERT(num.length == 0);
    bigint_free(&num);

    num = bigint_new("123");
    CU_ASSERT(num.length == 3);
    CU_ASSERT(num.digits[0] == 3);
    CU_ASSERT(num.digits[1] == 2);
    CU_ASSERT(num.digits[2] == 1);
    bigint_free(&num);
}

void test_bigint_free(void) {
    bigint num = bigint_new("123");
    bigint_free(&num);
    CU_ASSERT(num.length == 0);
    CU_ASSERT_PTR_NULL(num.digits);
}

void test_bigint_clone(void) {
    bigint num = bigint_new("123");
    bigint num_clone = bigint_clone(&num);
    CU_ASSERT_PTR_NOT_NULL(&num_clone);
    CU_ASSERT(num_clone.length == num.length);
    for (size_t i = 0; i < num.length; i++) {
        CU_ASSERT(num.digits[i] == num_clone.digits[i]);
    }

    num_clone.digits[0] = 4;
    CU_ASSERT(num.digits[0] != num_clone.digits[0]);

    num.digits[0] = 5;
    CU_ASSERT(num.digits[0] != num_clone.digits[0]);
    
    bigint_free(&num);
    bigint_free(&num_clone);
}

void test_bigint_realloc(void) {
    bigint num = bigint_new("123");
    bigint_realloc(&num, 0);
    CU_ASSERT_PTR_NULL(num.digits);
    CU_ASSERT(num.length == 0);
    bigint_free(&num);
    
    num = bigint_new("123");
    bigint_realloc(&num, 2);
    CU_ASSERT_PTR_NOT_NULL(num.digits);
    CU_ASSERT(num.length == 2);
    CU_ASSERT(num.digits[0] == 2);
    CU_ASSERT(num.digits[1] == 1);
    bigint_free(&num);

    num = bigint_new("123");
    bigint_realloc(&num, 3);
    CU_ASSERT_PTR_NOT_NULL(num.digits);
    CU_ASSERT(num.length == 3);
    CU_ASSERT(num.digits[0] == 3);
    CU_ASSERT(num.digits[1] == 2);
    CU_ASSERT(num.digits[2] == 1);
    bigint_free(&num);

    num = bigint_new("123");
    bigint_realloc(&num, 5);
    CU_ASSERT_PTR_NOT_NULL(num.digits);
    CU_ASSERT(num.length == 5);
    CU_ASSERT(num.digits[0] == 3);
    CU_ASSERT(num.digits[1] == 2);
    CU_ASSERT(num.digits[2] == 1);
    CU_ASSERT(num.digits[3] == 0);
    CU_ASSERT(num.digits[4] == 0);
    bigint_free(&num);
}

void test_bigint_eq(void) {
    bigint a = bigint_new("1");
    bigint b = bigint_new("1");
    CU_ASSERT(bigint_eq(&a, &b));
    bigint_free(&a);
    bigint_free(&b);

    a = bigint_new("01");
    b = bigint_new("00000001");
    CU_ASSERT(bigint_eq(&a, &b));
    bigint_free(&a);
    bigint_free(&b);

    a = bigint_new("012300");
    b = bigint_new("000012300");
    CU_ASSERT(bigint_eq(&a, &b));
    bigint_free(&a);
    bigint_free(&b);

    a = bigint_new("123");
    b = bigint_new("321");
    CU_ASSERT_FALSE(bigint_eq(&a, &b));
    bigint_free(&a);
    bigint_free(&b);

    a = bigint_new("0");
    b = bigint_new("0");
    CU_ASSERT(bigint_eq(&a, &b));

    bigint_free(&a);
    bigint_free(&b);
    CU_ASSERT_FALSE(bigint_eq(&a, &b));
}

void test_bigint_gt(void) {
    bigint a = bigint_new("100");
    bigint b = bigint_clone(&a);
    CU_ASSERT_FALSE(bigint_gt(&a, &b));
    CU_ASSERT_FALSE(bigint_gt(&a, &a));
    bigint_free(&a);
    bigint_free(&b);

    a = bigint_new("00000100");
    b = bigint_new("00100");
    CU_ASSERT_FALSE(bigint_gt(&a, &b));
    bigint_free(&a);
    bigint_free(&b);

    a = bigint_new("101");
    b = bigint_new("100");
    CU_ASSERT(bigint_gt(&a, &b));
    CU_ASSERT_FALSE(bigint_gt(&b, &a));
    bigint_free(&a);
    bigint_free(&b);
}

bool test_operation(void (*op)(bigint*, const bigint*), char* a_val, char* b_val, char* expected_val, size_t n_times) {
    bigint a, b, expected;
    a = bigint_new(a_val);
    b = bigint_new(b_val);
    expected = bigint_new(expected_val);

    for (size_t i = 0; i < n_times; i++)
        op(&a, &b);

    bool result = bigint_eq(&a, &expected);

    bigint_free(&a);
    bigint_free(&b);
    bigint_free(&expected);

    return result;
}

void test_bigint_add(void) {
    CU_ASSERT(test_operation(bigint_add, "0", "1", "1", 1));
    CU_ASSERT(test_operation(bigint_add, "0", "123456789123456789", "123456789123456789000000", 1e6));
    CU_ASSERT(test_operation(bigint_add, "0", "0", "0", 1));
}

void test_bigint_sub(void) {
    CU_ASSERT(test_operation(bigint_sub, "1", "0", "1", 1));
    CU_ASSERT(test_operation(bigint_sub, "100", "99", "1", 1));
    CU_ASSERT(test_operation(bigint_sub, "100", "100", "0", 1));
    CU_ASSERT(test_operation(bigint_sub, "123456789123456789000000", "123456789123456789", "0", 1e6));
}

void test_bigint_mul(void) {
    CU_ASSERT(test_operation(bigint_mul, "123456789987654321", "0", "0", 1));
    CU_ASSERT(test_operation(bigint_mul, "0", "123456789987654321", "0", 1));
    CU_ASSERT(test_operation(bigint_mul, "123456789987654321", "1", "123456789987654321", 1));
    CU_ASSERT(test_operation(bigint_mul, "123456789987654321", "2", "246913579975308642", 1));
    CU_ASSERT(test_operation(bigint_mul, "123456789987654321", "100", "12345678998765432100", 1));
}

void test_bigint_pow(void) {
    CU_ASSERT(test_operation(bigint_pow, "123456789", "0", "1", 1)); 
    CU_ASSERT(test_operation(bigint_pow, "123456789", "1", "123456789", 1)); 
    CU_ASSERT(test_operation(bigint_pow, "12345", "100", "14096439339183491166601553169309047526517979573772621675186880097101514883771279088675870214353060689415236739897781744867543850081841308508369545628865009483321119978601279878481873655865603345618917514602978765331940575318348748763822033479067105224096895463900041801357287309352642205539877489444394374651484492440510702227144201018980637306651590616693974026882156015683111860425924533046782016754150390625", 1)); 
}

int main() {
    CU_initialize_registry();
    CU_pSuite suite = CU_add_suite("Default", NULL, NULL);

    CU_add_test(suite, "bigint_new", test_bigint_new);
    CU_add_test(suite, "bigint_free", test_bigint_free);
    CU_add_test(suite, "bigint_clone", test_bigint_clone);
    CU_add_test(suite, "bigint_realloc", test_bigint_realloc);

    CU_add_test(suite, "bigint_eq", test_bigint_eq);
    CU_add_test(suite, "bigint_gt", test_bigint_gt);

    CU_add_test(suite, "bigint_add", test_bigint_add);
    CU_add_test(suite, "bigint_sub", test_bigint_sub);
    CU_add_test(suite, "bigint_mul", test_bigint_mul);
    CU_add_test(suite, "bigint_pow", test_bigint_pow);

    CU_basic_run_tests();
    CU_cleanup_registry();
    return 0;
}
