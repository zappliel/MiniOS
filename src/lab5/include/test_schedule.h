#ifndef _SCHEDULE_TEST_H_
#define _SCHEDULE_TEST_H_
#include "types.h"

/**
 * new content for unit test
*/
#define COUNTER_SEED 29
#define PRIORITY_SEED 17

#define PRIME_PRIORITY 193
#define PRIME_COUNTER_1 97
#define PRIME_COUNTER_2 13

void test_init(int num_tasks);
void schedule_test();

uint64 task_test_priority[5];
uint64 task_test_counter[5];

#endif