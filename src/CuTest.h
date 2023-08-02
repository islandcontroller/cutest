/*!*****************************************************************************
 * @file
 * CuTest.h
 *
 * @copyright Copyright (c) 2023 islandcontroller
 *
 * @brief
 * C Unit-Testing Framework for Embedded Applications
 *
 * A lightweight Unit-Testing framework targeting Embedded Applications running
 * on 32-bit MCU architectures. This source file is licensed under The MIT Li-
 * cense. See https://opensource.org/license/mit/ for full license text.
 *
 * The full framework source code is published at:
 * https://github.com/islandcontroller/cutest
 *
 * @date  24.04.2023
 * @date  01.08.2023  Replaced timestamp type
 * @date  02.08.2023  Added output toggles
 ******************************************************************************/

#ifndef _CUTEST_H_
#define _CUTEST_H_

/*- Header files -------------------------------------------------------------*/
#include <stdint.h>
#include <stdlib.h>
#include <setjmp.h>
#include <time.h>


/*- Common definitions -------------------------------------------------------*/
/*! Max. message length                                                       */
#define CUTEST_MAX_LEN_MESSAGE        256u

/*! Max. number of test cases per group                                       */
#define CUTEST_MAX_NUM_CASES          256u

/*! Max. number of groups per module                                          */
#define CUTEST_MAX_NUM_GROUPS         128u

/*! Max. number of root items                                                 */
#define CUTEST_MAX_NUM_ROOT_ITEM      32u

/*! Project name (override-able)                                              */
#ifndef CUTEST_PROJECT_NAME
#define CUTEST_PROJECT_NAME           "Unnamed Project"
#endif /* CUTEST_PROJECT_NAME */

/*! Output report file name (override-able)                                   */
#ifndef CUTEST_REPORT_FILE
#define CUTEST_REPORT_FILE            "report.html"
#endif /* CUTEST_REPORT_FILE */

/*! Generate report file (override-able)                                      */
#ifndef CUTEST_GENERATE_REPORT
#define CUTEST_GENERATE_REPORT        1u
#endif /* CUTEST_GENERATE_REPORT */

/*! Generate and print test run summary (override-able)                       */
#ifndef CUTEST_GENERATE_SUMMARY
#define CUTEST_GENERATE_SUMMARY       1u
#endif /* CUTEST_GENERATE_SUMMARY */

/*! Print test case results for Eclipse highlighting (override-able)          */
#ifndef CUTEST_PRINT_TESTCASE_RESULT
#define CUTEST_PRINT_TESTCASE_RESULT  1u
#endif /* CUTEST_PRINT_TESTCASE_RESULT */


/*- Type definitions ---------------------------------------------------------*/
/*! Forward declarations                                                      */
struct tag_cutest_case_t;
struct tag_cutest_group_t;
struct tag_cutest_module_t;
struct tag_cutest_relem_t;
struct tag_cutest_root_t;

/*! Pointer declarations                                                      */
typedef struct tag_cutest_case_t* cutest_case_ptr_t;
typedef struct tag_cutest_group_t* cutest_group_ptr_t;
typedef struct tag_cutest_module_t* cutest_module_ptr_t;
typedef struct tag_cutest_relem_t* cutest_relem_ptr_t;
typedef struct tag_cutest_root_t* cutest_root_ptr_t;

/*! Test function                                                             */
typedef void (*cutest_test_fn_t)(cutest_case_ptr_t _tc);

/*! Test result                                                               */
typedef enum
{
  EN_CUTEST_RESULT_UNDEF,           ///< Result undefined
  EN_CUTEST_RESULT_PASS,            ///< Result "passed"
  EN_CUTEST_RESULT_FAIL             ///< Result "failed"
} cutest_result_t;

/*! Test case data container                                                  */
typedef struct tag_cutest_case_t
{
  // Test case descriptions
  const char* pszName;              ///< Test-Case name
  const char* pszFile;              ///< Source file name
  unsigned long ulLine;             ///< Line number

  // Processing
  cutest_test_fn_t pfvTestFn;       ///< Test function
  jmp_buf sEnv;                     ///< Setjmp context buffer

  // Results
  cutest_result_t eResult;          ///< Result code
  char acMessage[CUTEST_MAX_LEN_MESSAGE]; ///< Error or diagnostic message
  const char* pszMsgFile;           ///< Message file name
  unsigned long ulMsgLine;          ///< Message line

  // Output config
  _Bool bPrintResult;               ///< Print run result to stdout
} cutest_case_t;

/*! Test group (set of cases)                                                 */
typedef struct tag_cutest_group_t
{
  // Group description
  const char* pszName;              ///< Group name
  const char* pszFile;              ///< Source file name
  unsigned long ulLine;             ///< Line number

  // Test case list
  cutest_case_ptr_t* const ppItems; ///< Assigned test cases
} cutest_group_t;

/*! Test module (set of groups)                                               */
typedef struct tag_cutest_module_t
{
  // Module description
  const char* pszName;              ///< Module name
  const char* pszFile;              ///< Source file name
  unsigned long ulLine;             ///< Line number

  // Test group list
  cutest_group_ptr_t* const ppItems; ///< Assigned groups
} cutest_module_t;

/*! Test run root element type                                                */
typedef enum
{
  EN_CUTEST_TYPE_CASE,
  EN_CUTEST_TYPE_GROUP,
  EN_CUTEST_TYPE_MODULE
} cutest_type_t;

/*! Test run root element                                                     */
typedef struct tag_cutest_relem_t
{
  cutest_type_t eType;              ///< Element type
  union {
    cutest_module_ptr_t psModule; ///< Module type pointer
    cutest_group_ptr_t psGroup;   ///< Group type pointer
    cutest_case_ptr_t psCase;     ///< Case type pointer
    void* pItem;
  };
} cutest_relem_t;

/*! Test run root                                                             */
typedef struct tag_cutest_root_t
{
  const char* pszName;              ///< Project name

  // Report items
  unsigned long ulCount;            ///< Current number of items
  cutest_relem_t asItems[CUTEST_MAX_NUM_ROOT_ITEM];  ///< List of elements
} cutest_root_t;


/*- Test case, group and module macros ---------------------------------------*/
/*! Test case definition. Usage:
 *
 * test.c:
 *   TEST_CASE(TEST_MyTest)
 *   {
 *     ...
 *     CuAssert...
 *   } // No semicolon - internally, this is a function body                  */
#define TEST_CASE(x)                                                           \
  void _##x##__TestFn(cutest_case_ptr_t);                                      \
  cutest_case_t _##x##__TestCase = {                                           \
    .pszName = #x,                                                             \
    .pszFile = __FILE__,                                                       \
    .ulLine = __LINE__,                                                        \
    .pfvTestFn = _##x##__TestFn,                                               \
    .eResult = EN_CUTEST_RESULT_UNDEF,                                         \
    .acMessage = "",                                                           \
    .pszMsgFile = __FILE__,                                                    \
    .ulMsgLine = __LINE__,                                                     \
    .bPrintResult = CUTEST_PRINT_TESTCASE_RESULT                               \
  };                                                                           \
  cutest_case_ptr_t const x = &_##x##__TestCase;                               \
  void _##x##__TestFn(cutest_case_ptr_t _tc __attribute__((unused)))

/*! External test case declaration. Usage:
 *
 * test.h:
 *   EXTERN_TEST_CASE(TEST_MyTest);                                           */
#define EXTERN_TEST_CASE(x) extern cutest_case_ptr_t const x

/*! Test group definition (set of test cases). Usage:
 *
 * test.c:
 *   TEST_GROUP(TestMyGroup)
 *   {
 *     TEST_MyTest,
 *     ...
 *   }; // Semicolon required - internally, this is an array definition       */
#define TEST_GROUP(x)                                                          \
  extern cutest_case_ptr_t _##x##__GroupItems[CUTEST_MAX_NUM_CASES];           \
  cutest_group_t _##x##__Group = {                                             \
    .pszName = #x,                                                             \
    .pszFile = __FILE__,                                                       \
    .ulLine = __LINE__,                                                        \
    .ppItems = _##x##__GroupItems,                                             \
  };                                                                           \
  cutest_group_ptr_t const x = &_##x##__Group;                                 \
  cutest_case_ptr_t _##x##__GroupItems[CUTEST_MAX_NUM_CASES] =

/*! External test group declaration. Usage:
 *
 * test.h:
 *   EXTERN_TEST_GROUP(TestMyGroup);                                          */
#define EXTERN_TEST_GROUP(x) extern cutest_group_ptr_t const x

/*! Test module definition (set of groups). Usage:
 *
 * test.c:
 *   TEST_MODULE(TestMyModule)
 *   {
 *     TestMyGroup,
 *     ...
 *   }; // Semicolon required - internally, this is an array definition       */
#define TEST_MODULE(x)                                                         \
  extern cutest_group_ptr_t _##x##__ModuleItems[CUTEST_MAX_NUM_GROUPS];        \
  cutest_module_t _##x##__Module = {                                           \
    .pszName = #x,                                                             \
    .pszFile = __FILE__,                                                       \
    .ulLine = __LINE__,                                                        \
    .ppItems = _##x##__ModuleItems                                             \
  };                                                                           \
  cutest_module_ptr_t const x = &_##x##__Module;                               \
  cutest_group_ptr_t _##x##__ModuleItems[CUTEST_MAX_NUM_GROUPS] =

/*! External test module declaration. Usage:
 *
 * test.h:
 *   EXTERN_TEST_MODULE(TestMyModule);                                        */
#define EXTERN_TEST_MODULE(x) extern cutest_module_ptr_t const x


/*- Result evaluation --------------------------------------------------------*/
void CuTest_EvalAssert         (cutest_case_ptr_t,  const char*, unsigned long, _Bool, const char*);
void CuTest_EvalAssertIntEquals(cutest_case_ptr_t,  const char*, unsigned long, intmax_t,    intmax_t);
void CuTest_EvalAssertFltEquals(cutest_case_ptr_t,  const char*, unsigned long, long double, long double, long double);
void CuTest_EvalAssertPtrEquals(cutest_case_ptr_t,  const char*, unsigned long, const void*, const void*);
void CuTest_EvalAssertPtrNotNull(cutest_case_ptr_t, const char*, unsigned long,              const void*);
void CuTest_EvalAssertStrEquals(cutest_case_ptr_t,  const char*, unsigned long, const char*, const char*);
void CuTest_EvalAssertMemEquals(cutest_case_ptr_t,  const char*, unsigned long, const void*, const void*, size_t);

/*! Result evaluation assert macros. Usage example:
 *
 * test.c:
 *   TEST_CASE(...)
 *   {
 *     const int expected = 1;
 *     const int actual = myFunc();
 *
 *     CuAssertIntEquals(expected, actual);
 *   }                                                                        */
#define CuPass()                                        CuTest_EvalAssert         (_tc,  __FILE__, __LINE__, (_Bool)1,           NULL     )
#define CuFail(message)                                 CuTest_EvalAssert         (_tc,  __FILE__, __LINE__, (_Bool)0,           (message))
#define CuAssert(condition, message)                    CuTest_EvalAssert         (_tc,  __FILE__, __LINE__, (_Bool)(condition), (message))
#define CuAssertIntEquals(expected, actual)             CuTest_EvalAssertIntEquals(_tc,  __FILE__, __LINE__, (intmax_t)(expected),    (intmax_t)(actual))
#define CuAssertFltEquals(expected, actual, tolerance)  CuTest_EvalAssertFltEquals(_tc,  __FILE__, __LINE__, (long double)(expected), (long double)(actual), (long double)(tolerance))
#define CuAssertPtrEquals(expected, actual)             CuTest_EvalAssertPtrEquals(_tc,  __FILE__, __LINE__, (const void*)(expected), (const void*)(actual))
#define CuAssertPtrNotNull(actual)                      CuTest_EvalAssertPtrNotNull(_tc, __FILE__, __LINE__,                          (const void*)(actual))
#define CuAssertStrEquals(expected, actual)             CuTest_EvalAssertStrEquals(_tc,  __FILE__, __LINE__, (const char*)(expected), (const char*)(actual))
#define CuAssertMemEquals(expected, actual, size)       CuTest_EvalAssertMemEquals(_tc,  __FILE__, __LINE__, (const void*)(expected), (const void*)(actual), (size_t)(size))


/*- Test run setup -----------------------------------------------------------*/
void CuTest_AppendRootItem(cutest_root_ptr_t, cutest_type_t, void*);
void CuTest_RunTestCase  (cutest_case_ptr_t);
void CuTest_RunTestGroup (cutest_group_ptr_t);
void CuTest_RunTestModule(cutest_module_ptr_t);
void CuTest_PrintRunResults        (const cutest_root_ptr_t, const time_t*);
void CuTest_GenerateRunReport      (const cutest_root_ptr_t, const time_t*, const char*);
cutest_result_t CuTest_GetRunResult(const cutest_root_ptr_t);

/*! Test run setup macros. Usage example:
 *
 * main.c:
 *   int main(void)
 *   {
 *     EXTERN_TEST_MODULE(TestMyModule);
 *     ...
 *
 *     BEGIN_TEST_RUN();
 *     RUN_TEST_MODULE(TestMyModule);
 *     ...
 *     END_TEST_RUN();
 *
 *     return GET_RUN_RESULT();
 *   }                                                                        */
#define BEGIN_TEST_RUN()                                                       \
  cutest_root_t _root = { .pszName = CUTEST_PROJECT_NAME, .ulCount = 0u }

#define RUN_TEST_CASE(x)                                                       \
  CuTest_AppendRootItem(&_root, EN_CUTEST_TYPE_CASE, x);                       \
  CuTest_RunTestCase(x)

#define RUN_TEST_GROUP(x)                                                      \
  CuTest_AppendRootItem(&_root, EN_CUTEST_TYPE_GROUP, x);                      \
  CuTest_RunTestGroup(x)

#define RUN_TEST_MODULE(x)                                                     \
  CuTest_AppendRootItem(&_root, EN_CUTEST_TYPE_MODULE, x);                     \
  CuTest_RunTestModule(x)

#define END_TEST_RUN()                                                         \
  time_t _ts = time(NULL);                                                     \
  if (CUTEST_GENERATE_SUMMARY) CuTest_PrintRunResults(&_root, &_ts);             \
  if (CUTEST_GENERATE_REPORT)  CuTest_GenerateRunReport(&_root, &_ts, CUTEST_REPORT_FILE)

#define GET_RUN_RESULT()                                                       \
  ((CuTest_GetRunResult(&_root) == EN_CUTEST_RESULT_PASS) ? EXIT_SUCCESS : EXIT_FAILURE)

#endif /* _CUTEST_H_ */
