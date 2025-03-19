#ifndef TOASTY_H
#define TOASTY_H

#include <stdio.h>

typedef void (*TestFunc)();

typedef struct {
    const char* name;
    TestFunc func;
} TestCase;

int RunTests();

void toasty__RegisterTest(const char* name, TestFunc func);
void toasty__IncrementFail();
const char* toasty__GetCurrentTestName();

#define TEST(name) \
    void name(); \
    __attribute__((constructor)) void register_##name() { \
        toasty__RegisterTest(#name, name); \
    } \
    void name()

#define FAIL(msg, file, line) do { \
    printf("\033[1;91m[FAIL]\033[m %s %s:%d: %s\n", toasty__GetCurrentTestName(), file, line, msg); \
    toasty__IncrementFail(); \
    return; \
} while(0)

#define TEST_ASSERT_TRUE(condition) if (!(condition)) { \
    FAIL("Condition is false", __FILE__, __LINE__); \
}

#define TEST_ASSERT_FALSE(condition) if ((condition)) { \
    FAIL("Condition is true", __FILE__, __LINE__); \
}

#define TEST_ASSERT_EQUAL(expected, actual) if ((expected) != (actual)) { \
    char msg[128]; \
    snprintf(msg, sizeof(msg), "Expected %d, but got %d", (int)(expected), (int)(actual)); \
    FAIL(msg, __FILE__, __LINE__); \
}

#define TEST_ASSERT_NULL(pointer) if ((pointer)) { \
    FAIL("Pointer is not null", __FILE__, __LINE__); \
}

#define TEST_ASSERT_NOT_NULL(pointer) if (!(pointer)) { \
    FAIL("Pointer is null", __FILE__, __LINE__); \
}

#endif // TOASTY_H

#ifdef TOASTY_IMPLEMENTATION

#include <stdlib.h>

#ifndef MAX_TESTS
#define MAX_TESTS 100
#endif // MAX_TESTS

static TestCase toasty__tests[MAX_TESTS];
static size_t toasty__testCount = 0;
static size_t toasty__testsPassed = 0;
static size_t toasty__testsFailed = 0;
static const char* toasty__currentTestName = NULL;

void toasty__RegisterTest(const char *name, TestFunc func) {
    if (toasty__testCount < MAX_TESTS) {
        toasty__tests[toasty__testCount++] = (TestCase){ name, func };
    }
    else {
        printf(
            "Max number of tests exceeded. (current amount: %zu)\n" \
            "Increase the max number using the following macro in the file with implementation:\n" \
            "#define MAX_TESTS n\n", \
            toasty__testCount \
        );
        exit(EXIT_FAILURE);
    }
}

void toasty__IncrementFail() {
    ++toasty__testsFailed;
}

const char* toasty__GetCurrentTestName() {
    return toasty__currentTestName;
}

int RunTests() {
    printf("\033[1;96mRunning %zu tests...\033[m\n", toasty__testCount);
    for (size_t i = 0; i < toasty__testCount; ++i) {
        toasty__currentTestName = toasty__tests[i].name;
        size_t beforeFailCount = toasty__testsFailed;
        toasty__tests[i].func();
        if (beforeFailCount == toasty__testsFailed) {
            printf("\033[1;92m[PASS]\033[m %s\n", toasty__currentTestName);
            ++toasty__testsPassed;
        }
    }

    printf("\n\033[1;96m========== TEST SUMMARY ==========\033[m\n");
    printf(
        "\033[1mTotal:\033[m %zu | \033[1mPassed:\033[m %zu | \033[1mFailed:\033[m %zu\n",
        toasty__testCount, toasty__testsPassed, toasty__testsFailed
    );
    if (toasty__testsFailed > 0) {
        printf("\033[1;91mSome tests failed!\033[m\n");
        return EXIT_FAILURE;
    } else {
        printf("\033[1;92mAll tests passed!\033[m\n");
        return EXIT_SUCCESS;
    }
}

#endif // TOASTY_IMPLEMENTATION
