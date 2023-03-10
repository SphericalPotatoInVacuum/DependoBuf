#include <check.h>
#include "../src/parser/scanner.h"

const int EXIT_FAILURE = 1;
const int EXIT_SUCCESS = 0;

START_TEST(test_init) {
    int token;
    int prevline;

    init(&token, &prevline);

    ck_assert_int_eq(token, 0);
    ck_assert_int_eq(prevline, -1);
}
END_TEST

Suite *parser_suite(void) {
    Suite *s;
    TCase *tc_init;

    s = suite_create("Parser");

    /* Core test case */
    tc_init = tcase_create("Init");
    tcase_add_test(tc_init, test_init);
    suite_add_tcase(s, tc_init);

    return s;
}

int main(void) {
    int number_failed;
    Suite *s;
    SRunner *sr;

    s = parser_suite();
    sr = srunner_create(s);

    srunner_run_all(sr, CK_NORMAL);
    number_failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
