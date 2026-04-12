// test_main.c — Test runner for BrogueCE E2E tests

#include "test_harness.h"

// Declare all test suites
extern void suite_movement(void);
extern void suite_combat(void);
extern void suite_status(void);
extern void suite_items(void);
extern void suite_environment(void);
extern void suite_infrastructure(void);
extern void suite_diagonal(void);
extern void suite_terrain(void);
extern void suite_levels(void);
extern void suite_fire_gas(void);
extern void suite_weapons(void);
extern void suite_ranged(void);
extern void suite_monsters(void);
extern void suite_item_usage(void);
extern void suite_combat_math(void);
extern void suite_status_effects(void);
extern void suite_vision(void);
extern void suite_lifecycle(void);
extern void suite_edge_cases(void);
extern void suite_keys(void);
extern void suite_allies(void);

int main(int argc, char *argv[]) {
    printf("BrogueCE Test Suite\n");
    printf("====================\n");

    RUN_SUITE(infrastructure);
    RUN_SUITE(movement);
    RUN_SUITE(combat);
    RUN_SUITE(status);
    RUN_SUITE(items);
    RUN_SUITE(environment);
    RUN_SUITE(diagonal);
    RUN_SUITE(terrain);
    RUN_SUITE(levels);
    RUN_SUITE(fire_gas);
    RUN_SUITE(weapons);
    RUN_SUITE(ranged);
    RUN_SUITE(monsters);
    RUN_SUITE(item_usage);
    RUN_SUITE(combat_math);
    RUN_SUITE(status_effects);
    RUN_SUITE(vision);
    RUN_SUITE(lifecycle);
    RUN_SUITE(edge_cases);
    RUN_SUITE(keys);
    RUN_SUITE(allies);

    printf("\n====================\n");
    printf("Results: %d passed, %d failed\n", _test_state.passed, _test_state.failed);

    return _test_state.failed > 0 ? 1 : 0;
}
