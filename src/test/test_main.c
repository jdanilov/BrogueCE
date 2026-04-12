// test_main.c — Test runner for BrogueCE E2E tests

#include "test_harness.h"

// Declare all test suites
extern void suite_movement(void);
extern void suite_combat(void);
extern void suite_status(void);
extern void suite_items(void);
extern void suite_environment(void);
extern void suite_infrastructure(void);

int main(int argc, char *argv[]) {
    printf("BrogueCE Test Suite\n");
    printf("====================\n");

    RUN_SUITE(infrastructure);
    RUN_SUITE(movement);
    RUN_SUITE(combat);
    RUN_SUITE(status);
    RUN_SUITE(items);
    RUN_SUITE(environment);

    printf("\n====================\n");
    printf("Results: %d passed, %d failed\n", _test_state.passed, _test_state.failed);

    return _test_state.failed > 0 ? 1 : 0;
}
