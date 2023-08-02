/*!*****************************************************************************
 * @file
 * CuTest.c
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

/*- Header files -------------------------------------------------------------*/
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"


/*- Macro definitions --------------------------------------------------------*/
/*! Framework version identifier                                              */
#ifndef CUTEST_VERSION
#define CUTEST_VERSION                "unknown"
#endif /* CUTEST_VERSION */

/*! Noreturn attribute for functions using longjmp                            */
#define _NORETURN                     __attribute__((noreturn))

/*! Summary char for "passed" test cases                                      */
#define CUTEST_SUMMARY_CHR_PASSED     '.'

/*! Summary char for "failed" test cases                                      */
#define CUTEST_SUMMARY_CHR_FAILED     'F'

/*! Summary char for invalid test cases                                       */
#define CUTEST_SUMMARY_CHR_INVALID    '?'

/*! Maxium timestamp string length                                            */
#define CUTEST_TIMESTAMP_MAX_LEN      24u


/*- Type definitions ---------------------------------------------------------*/
/*! Statistics counters                                                       */
typedef struct tag_cutest_stats_t
{
  unsigned long ulTotal;            ///< Total test cases
  unsigned long ulPassed;           ///< Number of "passed" test cases
  unsigned long ulFailed;           ///< Number of "failed" test cases
} cutest_stats_t;


/*- Prototypes ---------------------------------------------------------------*/
static void           CuTestAssertPassed(cutest_case_ptr_t psTc);
static void _NORETURN CuTestAssertFailed(cutest_case_ptr_t psTc);

static void           CuTestPrintSummaryTape_Case(const cutest_case_ptr_t psCase);
static void           CuTestPrintSummaryTape_Group(const cutest_group_ptr_t psGroup);
static void           CuTestPrintSummaryTape_Module(const cutest_module_ptr_t psModule);
static void           CuTestPrintSummary(const cutest_root_ptr_t psRoot);
static void           CuTestPrintDetails(const cutest_root_ptr_t psRoot);

static cutest_stats_t CuTestGetStats_Case(const cutest_case_ptr_t psCase);
static cutest_stats_t CuTestGetStats_Group(const cutest_group_ptr_t psGroup);
static cutest_stats_t CuTestGetStats_Module(const cutest_module_ptr_t psModule);
static cutest_stats_t CuTestGetStats(const cutest_root_ptr_t psRoot);

static void           CuTestGenerateReport_CaseHeader(FILE* f);
static void           CuTestGenerateReport_CaseLine(FILE* f, unsigned long* pulNum, const cutest_case_ptr_t psCase);
static void           CuTestGenerateReport_CaseFooter(FILE* f);
static void           CuTestGenerateReport_Group(FILE* f, unsigned long* pulNum, const cutest_group_ptr_t psGroup);
static void           CuTestGenerateReport_Module(FILE* f, unsigned long* pulNum, const cutest_module_ptr_t psModule);

static const char*    CuTestGetTimestampString(const time_t* pTime);


/*- Private variables --------------------------------------------------------*/
/*! Buffer for ISO8601-formatted timestamp string                             */
static char acTimestampBuffer[CUTEST_TIMESTAMP_MAX_LEN + 1u];


/*- Local functions ----------------------------------------------------------*/
/*!****************************************************************************
 * @brief
 * Assign 'passed' state and continue
 *
 * @param[in] psTc        Test case data
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestAssertPassed(cutest_case_ptr_t psTc)
{
  assert(psTc != NULL);

  psTc->eResult = EN_CUTEST_RESULT_PASS;
}

/*!****************************************************************************
 * @brief
 * Assign 'failed' state and exit by means of longjmp
 *
 * @note longjmp to calling handler
 * @param[in] psTc        Test case data
 * @date  26.04.2023
 ******************************************************************************/
static void __attribute__((noreturn)) CuTestAssertFailed(cutest_case_ptr_t psTc)
{
  assert(psTc != NULL);

  psTc->eResult = EN_CUTEST_RESULT_FAIL;
  longjmp(psTc->sEnv, 0);

  __builtin_unreachable();
}

/*!****************************************************************************
 * @brief
 * Output summary results tape printout for one test case
 *
 * @param[in] psCase      Test case data
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintSummaryTape_Case(const cutest_case_ptr_t psCase)
{
  assert(psCase != NULL);

  switch (psCase->eResult)
  {
    case EN_CUTEST_RESULT_PASS: putchar(CUTEST_SUMMARY_CHR_PASSED); break;
    case EN_CUTEST_RESULT_FAIL: putchar(CUTEST_SUMMARY_CHR_FAILED); break;
    default:                    putchar(CUTEST_SUMMARY_CHR_INVALID);
  }
}

/*!****************************************************************************
 * @brief
 * Process summary results tape printout for all test cases in a group
 *
 * @param[in] psGroup     Test case group
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintSummaryTape_Group(const cutest_group_ptr_t psGroup)
{
  assert(psGroup != NULL);
  assert(psGroup->ppItems != NULL);

  for (unsigned long i = 0; i < CUTEST_MAX_NUM_CASES; ++i)
  {
    const cutest_case_ptr_t psCase = psGroup->ppItems[i];

    if (psCase != NULL) CuTestPrintSummaryTape_Case(psCase);
    else break;
  }
}

/*!****************************************************************************
 * @brief
 * Process summary results tape printout for all groups in a module
 *
 * @param[in] psModule    Test module
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintSummaryTape_Module(const cutest_module_ptr_t psModule)
{
  assert(psModule != NULL);
  assert(psModule->ppItems != NULL);

  for (unsigned long i = 0; i < CUTEST_MAX_NUM_GROUPS; ++i)
  {
    const cutest_group_ptr_t psGroup = psModule->ppItems[i];

    if (psGroup != NULL) CuTestPrintSummaryTape_Group(psGroup);
    else break;
  }
}

/*!****************************************************************************
 * @brief
 * Print test run summary
 *
 * @param[in] psRoot      Test run root
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintSummary(const cutest_root_ptr_t psRoot)
{
  assert(psRoot != NULL);
  assert(psRoot->ulCount < CUTEST_MAX_NUM_ROOT_ITEM);

  // Header
  printf("Summary (%c=fail, %c=pass, %c=invalid):\n\t", CUTEST_SUMMARY_CHR_FAILED, CUTEST_SUMMARY_CHR_PASSED, CUTEST_SUMMARY_CHR_INVALID);

  // Print result "tape"
  for (unsigned long i = 0; i < psRoot->ulCount; ++i)
  {
    const cutest_relem_ptr_t psItem = &psRoot->asItems[i];
    if (psItem->pItem != NULL) switch (psItem->eType)
    {
      case EN_CUTEST_TYPE_CASE:   CuTestPrintSummaryTape_Case(psItem->psCase);      break;
      case EN_CUTEST_TYPE_GROUP:  CuTestPrintSummaryTape_Group(psItem->psGroup);    break;
      case EN_CUTEST_TYPE_MODULE: CuTestPrintSummaryTape_Module(psItem->psModule);  break;
      default:;
    }
    else break;
  }

  // Trailer
  printf("\r\n");
}

/*!****************************************************************************
 * @brief
 * Print test run details for a single test case
 *
 * @param[in] *pulNum     Detail counter
 * @param psCase          Test case data
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintDetails_Case(unsigned long* pulNum, const cutest_case_ptr_t psCase)
{
  assert(pulNum != NULL);
  assert(psCase != NULL);

  switch (psCase->eResult)
  {
    case EN_CUTEST_RESULT_FAIL:
      *pulNum += 1;
      printf("\t%ld) %s -- %s:%ld: %s\n", *pulNum, psCase->pszName, psCase->pszMsgFile, psCase->ulMsgLine, psCase->acMessage);
      break;

    case EN_CUTEST_RESULT_UNDEF:
      *pulNum += 1;
      printf("\t%ld) %s -- %s:%ld: not evaluated\n", *pulNum, psCase->pszName, psCase->pszFile, psCase->ulLine);
      break;

    default:;
  }
}

/*!****************************************************************************
 * @brief
 * Process printing test run details for all test cases in a group
 *
 * @param[in] *pulNum     Detail counter
 * @param psGroup         Test case group
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintDetails_Group(unsigned long* pulNum, const cutest_group_ptr_t psGroup)
{
  assert(psGroup != NULL);
  assert(psGroup->ppItems != NULL);

  for (unsigned long i = 0; i < CUTEST_MAX_NUM_CASES; ++i)
  {
    const cutest_case_ptr_t psCase = psGroup->ppItems[i];
    if (psCase != NULL) CuTestPrintDetails_Case(pulNum, psCase);
  }
}

/*!****************************************************************************
 * @brief
 * Process printing test run details for all groups in a module
 *
 * @param[in] *pulNum     Detail counter
 * @param psModule        Test module
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintDetails_Module(unsigned long* pulNum, const cutest_module_ptr_t psModule)
{
  assert(psModule != NULL);
  assert(psModule->ppItems != NULL);

  for (unsigned long i = 0; i < CUTEST_MAX_NUM_GROUPS; ++i)
  {
    const cutest_group_ptr_t psGroup = psModule->ppItems[i];
    if (psGroup != NULL) CuTestPrintDetails_Group(pulNum, psGroup);
  }
}

/*!****************************************************************************
 * @brief
 * Print test run details
 *
 * @param[in] psRoot      Test run root
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestPrintDetails(const cutest_root_ptr_t psRoot)
{
  assert(psRoot != NULL);
  assert(psRoot->ulCount < CUTEST_MAX_NUM_ROOT_ITEM);

  cutest_stats_t sStats = CuTestGetStats(psRoot);
  if (sStats.ulPassed == sStats.ulTotal)
  {
    printf("\nResult:\n\tPASS");
  }
  else
  {
    unsigned long ulInvalid = sStats.ulTotal - sStats.ulPassed - sStats.ulFailed;
    printf("\nDetails (%ld fails, %ld invalid):\n", sStats.ulFailed, ulInvalid);

    unsigned long ulNum = 0;
    for (unsigned long i = 0; i < psRoot->ulCount; ++i)
    {
      const cutest_relem_ptr_t psItem = &psRoot->asItems[i];
      if (psItem->pItem != NULL) switch (psItem->eType)
      {
        case EN_CUTEST_TYPE_CASE:   CuTestPrintDetails_Case(&ulNum, psItem->psCase);      break;
        case EN_CUTEST_TYPE_GROUP:  CuTestPrintDetails_Group(&ulNum, psItem->psGroup);    break;
        case EN_CUTEST_TYPE_MODULE: CuTestPrintDetails_Module(&ulNum, psItem->psModule);  break;
        default:;
      }
    }

    printf("\nResult:\n\tFAIL");
  }

  printf(" (%ld runs, %ld passes, %ld fails)\n", sStats.ulTotal, sStats.ulPassed, sStats.ulFailed);
}

/*!****************************************************************************
 * @brief
 * Calculate statistics for a single test case
 *
 * @param[in] psCase      Test case data
 * @return  (cutest_stats_t)  Statistics counters
 * @date  26.04.2023
 ******************************************************************************/
static cutest_stats_t CuTestGetStats_Case(const cutest_case_ptr_t psCase)
{
  assert(psCase != NULL);

  return (cutest_stats_t){
    .ulTotal = 1,
    .ulFailed = (psCase->eResult == EN_CUTEST_RESULT_FAIL) ? 1 : 0,
    .ulPassed = (psCase->eResult == EN_CUTEST_RESULT_PASS) ? 1 : 0,
  };
}

/*!****************************************************************************
 * @brief
 * Calculate statistics for a group of test cases
 *
 * @param[in] psGroup     Test group
 * @return  (cutest_stats_t)  Statistics counters
 * @date  26.04.2023
 ******************************************************************************/
static cutest_stats_t CuTestGetStats_Group(const cutest_group_ptr_t psGroup)
{
  assert(psGroup != NULL);
  assert(psGroup->ppItems != NULL);

  cutest_stats_t sGroup = { 0, 0, 0 };
  for (unsigned long i = 0; i < CUTEST_MAX_NUM_CASES; ++i)
  {
    const cutest_case_ptr_t psCase = psGroup->ppItems[i];
    if (psCase != NULL)
    {
      cutest_stats_t sCase = CuTestGetStats_Case(psCase);
      sGroup.ulTotal += sCase.ulTotal;
      sGroup.ulFailed += sCase.ulFailed;
      sGroup.ulPassed += sCase.ulPassed;
    }
  }

  return sGroup;
}

/*!****************************************************************************
 * @brief
 * Calculate statistics for a test module
 *
 * @param[in] psModule    Test module
 * @return  (cutest_stats_t)  Statistics counters
 * @date  26.04.2023
 ******************************************************************************/
static cutest_stats_t CuTestGetStats_Module(const cutest_module_ptr_t psModule)
{
  assert(psModule != NULL);
  assert(psModule->ppItems != NULL);

  cutest_stats_t sModule = { 0, 0, 0 };
  for (unsigned long i = 0; i < CUTEST_MAX_NUM_GROUPS; ++i)
  {
    const cutest_group_ptr_t psGroup = psModule->ppItems[i];
    if (psGroup != NULL)
    {
      cutest_stats_t sGroup = CuTestGetStats_Group(psGroup);
      sModule.ulTotal += sGroup.ulTotal;
      sModule.ulFailed += sGroup.ulFailed;
      sModule.ulPassed += sGroup.ulPassed;
    }
  }

  return sModule;
}

/*!****************************************************************************
 * @brief
 * Calculate statistics for a test run
 *
 * @param[in] psRoot      Test run root
 * @return  (cutest_stats_t)  Statistics counters
 * @date  26.04.2023
 ******************************************************************************/
static cutest_stats_t CuTestGetStats(const cutest_root_ptr_t psRoot)
{
  assert(psRoot != NULL);
  assert(psRoot->ulCount < CUTEST_MAX_NUM_ROOT_ITEM);

  cutest_stats_t sRun = { 0, 0, 0 };
  for (unsigned long i = 0; i < psRoot->ulCount; ++i)
  {
    cutest_stats_t sItem = { 0, 0, 0 };
    const cutest_relem_ptr_t psItem = &psRoot->asItems[i];
    if (psItem->pItem != NULL) switch (psItem->eType)
    {
      case EN_CUTEST_TYPE_CASE:   sItem = CuTestGetStats_Case(psItem->psCase);      break;
      case EN_CUTEST_TYPE_GROUP:  sItem = CuTestGetStats_Group(psItem->psGroup);    break;
      case EN_CUTEST_TYPE_MODULE: sItem = CuTestGetStats_Module(psItem->psModule);  break;
      default:;
    }

    sRun.ulTotal += sItem.ulTotal;
    sRun.ulFailed += sItem.ulFailed;
    sRun.ulPassed += sItem.ulPassed;
  }

  return sRun;
}

/*!****************************************************************************
 * @brief
 * Emit test case table header
 *
 * @param[out] *f         Output file
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestGenerateReport_CaseHeader(FILE* f)
{
  assert(f != NULL);

  fprintf(f, "<table border=\"1\"><tr><th>Nr.</th><th>Name</th><th>File</th><th>Result</th><th>Message</th></tr>");
}

/*!****************************************************************************
 * @brief
 * Emit test case table line
 *
 * @param[out] *f         Output file
 * @param[inout] *pulNum  Test case counter
 * @param[in] psCase      Test case data
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestGenerateReport_CaseLine(FILE* f, unsigned long* pulNum, const cutest_case_ptr_t psCase)
{
  assert(f != NULL);
  assert(pulNum != NULL);
  assert(psCase != NULL);

  *pulNum += 1;

  const char* pszColor;
  const char* pszResult;
  _Bool bPrintMsg;
  switch (psCase->eResult)
  {
    case EN_CUTEST_RESULT_PASS: pszColor = "lime";    pszResult = "pass";    bPrintMsg = 0; break;
    case EN_CUTEST_RESULT_FAIL: pszColor = "red";     pszResult = "fail";    bPrintMsg = 1; break;
    default:                    pszColor = "silver";  pszResult = "invalid", bPrintMsg = 0;
  }

  const char* pszFile = bPrintMsg ? psCase->pszMsgFile : psCase->pszFile;
  unsigned long ulLine = bPrintMsg ? psCase->ulMsgLine : psCase->ulLine;
  const char* pszMessage = bPrintMsg ? psCase->acMessage : "";
  fprintf(f, "<tr><td>%ld</td><td>%s</td><td><a href=\"%s#L%ld\">%s#L%ld</a></td><td style=\"background-color: %s\">%s</td><td>%s</td></tr>", *pulNum, psCase->pszName, pszFile, ulLine, pszFile, ulLine, pszColor, pszResult, pszMessage);
}

/*!****************************************************************************
 * @brief
 * Emit test case table footer
 *
 * @param[out] *f         Output file
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestGenerateReport_CaseFooter(FILE* f)
{
  assert(f != NULL);

  fprintf(f, "</table>");
}

/*!****************************************************************************
 * @brief
 * Emit group heading and process contained test cases
 *
 * @param[out] *f         Output file
 * @param[inout] *pulNum  Test case counter
 * @param[in] psGroup     Test case group
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestGenerateReport_Group(FILE* f, unsigned long* pulNum, const cutest_group_ptr_t psGroup)
{
  assert(f != NULL);
  assert(psGroup != NULL);
  assert(psGroup->ppItems != NULL);

  fprintf(f, "<h3>%s</h3>", psGroup->pszName);

  CuTestGenerateReport_CaseHeader(f);
  for (unsigned long i = 0; i < CUTEST_MAX_NUM_CASES; ++i)
  {
    const cutest_case_ptr_t psCase = psGroup->ppItems[i];
    if (psCase != NULL) CuTestGenerateReport_CaseLine(f, pulNum, psCase);
  }
  CuTestGenerateReport_CaseFooter(f);
}

/*!****************************************************************************
 * @brief
 * Emit module heading and process contained groups
 *
 * @param[out] *f         Output file
 * @param[inout] *pulNum  Test case conuter
 * @param[in] psModule    Test module
 * @date  26.04.2023
 ******************************************************************************/
static void CuTestGenerateReport_Module(FILE* f, unsigned long* pulNum, const cutest_module_ptr_t psModule)
{
  assert(f != NULL);
  assert(psModule != NULL);
  assert(psModule->ppItems != NULL);

  fprintf(f, "<h2>%s</h2>", psModule->pszName);

  for (unsigned long i = 0; i < CUTEST_MAX_NUM_GROUPS; ++i)
  {
    const cutest_group_ptr_t psGroup = psModule->ppItems[i];
    if (psGroup != NULL) CuTestGenerateReport_Group(f, pulNum, psGroup);
  }
}

/*!****************************************************************************
 * @brief
 * Helper function for generating ISO8601-formatted timestamp strings
 *
 * @param[in] *pTime      Input timestamp
 * @return  (const char*) Timestamp string buffer
 * @date  01.08.2023
 ******************************************************************************/
static const char* CuTestGetTimestampString(const time_t* pTime)
{
  assert(pTime != NULL);

  const struct tm* psTm = gmtime(pTime);
  assert(psTm != NULL);

  size_t ulLen = strftime(acTimestampBuffer, sizeof(acTimestampBuffer), "%FT%T%z", psTm);
  assert(ulLen < sizeof(acTimestampBuffer));
  acTimestampBuffer[ulLen] = '\0';

  return acTimestampBuffer;
}


/*- Result evaluation functions ----------------------------------------------*/
/*!****************************************************************************
 * @brief
 * Evaluate generic boolean condition
 *
 * @note longjmp on condition result 'false'
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] bCondition  Asserted condition
 * @param[in] *pszMessage Error message (optional)
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_EvalAssert(cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, _Bool bCondition, const char* pszMessage)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);

  if (bCondition)
  {
    CuTestAssertPassed(psTc);
  }
  else
  {
    const char* pszFmt;
    if ((pszMessage != NULL) && (pszMessage[0] != '\0')) pszFmt = "%s";
    else                                                 pszFmt = "assert failed.";

    snprintf(psTc->acMessage, sizeof(psTc->acMessage), pszFmt, pszMessage);
    psTc->pszMsgFile = pszFile;
    psTc->ulMsgLine = ulLine;
    CuTestAssertFailed(psTc);
  }
}

/*!****************************************************************************
 * @brief
 * Evaluate integers to be equal
 *
 * @note longjmp on mismatch
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] llExpected  Expected value
 * @param[in] llActual    Actual value
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_EvalAssertIntEquals(cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, intmax_t llExpected, intmax_t llActual)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);

  if (llActual == llExpected)
  {
    CuTestAssertPassed(psTc);
  }
  else
  {
    snprintf(psTc->acMessage, sizeof(psTc->acMessage), "expected <%jd>, but was <%jd>", llExpected, llActual);
    psTc->pszMsgFile = pszFile;
    psTc->ulMsgLine = ulLine;
    CuTestAssertFailed(psTc);
  }
}

/*!****************************************************************************
 * @brief
 * Evaluate long double-precision floating point numbers to be equal within the
 * specificed tolerance band
 *
 * @note longjmp if deviation exceeds limit
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] llfExpected Expected value
 * @param[in] llfActual   Actual value
 * @param[in] llfTolerance  Maximum allowed deviation between both values
 * @date  26.04.2023
 * @date  27.04.2023  Renamed to ..FltEquals to match macro invocation
 ******************************************************************************/
void CuTest_EvalAssertFltEquals(cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, long double llfExpected, long double llfActual, long double llfTolerance)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);
  assert(!isnan(llfTolerance));

  long double llfDeviation = fabsl(llfActual - llfExpected);
  if (llfDeviation > llfTolerance)
  {
    snprintf(psTc->acMessage, sizeof(psTc->acMessage), "expected <%Lf>, but was <%Lf> (Deviation <%Lf> exceeds <%Lf>)", llfExpected, llfActual, llfDeviation, llfTolerance);
    psTc->pszMsgFile = pszFile;
    psTc->ulMsgLine = ulLine;
    CuTestAssertFailed(psTc);
  }
  else
  {
    CuTestAssertPassed(psTc);
  }
}

/*!****************************************************************************
 * @brief
 * Evaluate pointers to be equal
 *
 * @note longjmp on mismatch
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] *pExpected  Expected value
 * @param[in] *pActual    Actual value
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_EvalAssertPtrEquals (cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, const void* pExpected, const void* pActual)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);

  if (pExpected == pActual)
  {
    CuTestAssertPassed(psTc);
  }
  else
  {
    const char* pszFmt;
    const void* p1;
    const void* p2;

    if (pExpected == NULL)    pszFmt = "expected <NULL>, but was <%p>", p1 = pActual,   p2 = NULL;
    else if (pActual == NULL) pszFmt = "expected <%p>, but was <NULL>", p1 = pExpected, p2 = NULL;
    else                      pszFmt = "expected <%p>, but was <%p>",   p1 = pExpected, p2 = pActual;

    snprintf(psTc->acMessage, sizeof(psTc->acMessage), pszFmt, p1, p2);
    psTc->pszMsgFile = pszFile;
    psTc->ulMsgLine = ulLine;
    CuTestAssertFailed(psTc);
  }
}

/*!****************************************************************************
 * @brief
 * Evaluate pointer to not be NULL
 *
 * @note longjmp on NULL
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] *pActual    Actual value
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_EvalAssertPtrNotNull(cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, const void* pActual)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);

  if (pActual != NULL)
  {
    CuTestAssertPassed(psTc);
  }
  else
  {
    snprintf(psTc->acMessage, sizeof(psTc->acMessage), "<NULL> unexpected");
    psTc->pszMsgFile = pszFile;
    psTc->ulMsgLine = ulLine;
    CuTestAssertFailed(psTc);
  }
}

/*!****************************************************************************
 * @brief
 * Evaluate strings to be equal
 *
 * @note longjmp on NULL or mismatch
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] *pszExpected  Expected string (non-null)
 * @param[in] *pszActual    Actual string
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_EvalAssertStrEquals(cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, const char* pszExpected, const char* pszActual)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);
  assert(pszExpected != NULL);

  if ((pszActual != NULL) && (strcmp(pszExpected, pszActual) == 0))
  {
    CuTestAssertPassed(psTc);
  }
  else
  {
    const char* pszFmt;
    if (pszActual == NULL) pszFmt = "expected <%s>, but was <NULL>";
    else                   pszFmt = "expected <%s>, but was <%s>";

    snprintf(psTc->acMessage, sizeof(psTc->acMessage), pszFmt, pszExpected, pszActual);
    psTc->pszMsgFile = pszFile;
    psTc->ulMsgLine = ulLine;
    CuTestAssertFailed(psTc);
  }
}

/*!****************************************************************************
 * @brief
 * Evaluate memory contents to be equal
 *
 * @note longjmp on mismatch
 * @param[in] psTc        Test case data
 * @param[in] *pszFile    File name
 * @param[in] ulLine      Line number
 * @param[in] *pExpected  Expected data (non-null)
 * @param[in] *pActual    Actual data
 * @param[in] uSize       Size of data in bytes
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_EvalAssertMemEquals (cutest_case_ptr_t psTc, const char* pszFile, unsigned long ulLine, const void* pExpected, const void* pActual, size_t uSize)
{
  assert(psTc != NULL);
  assert(pszFile != NULL);
  assert(pExpected != NULL);

  for (size_t i = 0; i < uSize; ++i)
  {
    uint8_t ucExpected = *(const uint8_t*)(pExpected + i);
    uint8_t ucActual = *(const uint8_t*)(pActual + i);

    if (ucExpected != ucActual)
    {
      snprintf(psTc->acMessage, sizeof(psTc->acMessage), "mismatch at offset <%d>: expected <0x%02X>, but was <0x%02X>", i, (unsigned)ucExpected, (unsigned)ucActual);
      psTc->pszMsgFile = pszFile;
      psTc->ulMsgLine = ulLine;
      CuTestAssertFailed(psTc);
    }
  }

  // All bytes matching
  CuTestAssertPassed(psTc);
}


/*- Test run management ------------------------------------------------------*/
/*!****************************************************************************
 * @brief
 * Append root entry for a new test run item
 *
 * The root item list is used for report generation.
 *
 * @param[inout] psRoot   Test run root
 * @param[in] eType       Item type (Test case, group or module)
 * @param[in] *pItem      Item data structure pointer
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_AppendRootItem(cutest_root_ptr_t psRoot, cutest_type_t eType, void* pItem)
{
  assert(psRoot != NULL);
  assert(pItem != NULL);
  assert(psRoot->ulCount < CUTEST_MAX_NUM_ROOT_ITEM);

  psRoot->asItems[psRoot->ulCount++] = (cutest_relem_t){ .eType = eType, .pItem = pItem };
}

/*!****************************************************************************
 * @brief
 * Run test case
 *
 * @param[in] psTc        Test case to be run
 * @date  26.04.2023
 * @date  02.08.2023  Added error parser message toggle
 ******************************************************************************/
void CuTest_RunTestCase(cutest_case_ptr_t psTc)
{
  assert(psTc != NULL);
  assert(psTc->pfvTestFn != NULL);

  // Reset result buffers
  psTc->eResult = EN_CUTEST_RESULT_UNDEF;
  memset(psTc->acMessage, '\0', sizeof(psTc->acMessage));

  // Set return point and execute test case
  if (setjmp(psTc->sEnv) == 0) psTc->pfvTestFn(psTc);

  // Print results for Eclipse error parser
  if (psTc->bPrintResult) switch (psTc->eResult)
  {
    case EN_CUTEST_RESULT_PASS: printf("%s:%ld:0: info: %s passed.\n",           psTc->pszFile,    psTc->ulLine,    psTc->pszName);   break;
    case EN_CUTEST_RESULT_FAIL: printf("%s:%ld:0: error: %s failed.\n ",         psTc->pszFile,    psTc->ulLine,    psTc->pszName);
                                printf("%s:%ld:0: error: %s\n ",                 psTc->pszMsgFile, psTc->ulMsgLine, psTc->acMessage); break;
    default:                    printf("%s:%ld:0: warning: %s not evaluated.\n", psTc->pszFile,    psTc->ulLine,    psTc->pszName);   break;
  }
}

/*!****************************************************************************
 * @brief
 * Run all test cases in a group
 *
 * @param[in] psGroup     Test case group
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_RunTestGroup(cutest_group_ptr_t psGroup)
{
  assert(psGroup != NULL);
  assert(psGroup->ppItems != NULL);

  // Iterate through available test cases
  for (unsigned long i = 0; i < CUTEST_MAX_NUM_CASES; ++i)
  {
    cutest_case_ptr_t psCase = psGroup->ppItems[i];
    if (psCase != NULL) CuTest_RunTestCase(psCase);
  }
}

/*!****************************************************************************
 * @brief
 * Run all groups in a module
 *
 * @param[in] psModule    Test module
 * @date  26.04.2023
 ******************************************************************************/
void CuTest_RunTestModule(cutest_module_ptr_t psModule)
{
  assert(psModule != NULL);
  assert(psModule->ppItems != NULL);

  // Iterate through available groups
  for (unsigned long i = 0; i < CUTEST_MAX_NUM_GROUPS; ++i)
  {
    cutest_group_ptr_t psGroup = psModule->ppItems[i];
    if (psGroup != NULL) CuTest_RunTestGroup(psGroup);
  }
}

/*!****************************************************************************
 * @brief
 * Print test run results to stdout
 *
 * @param[in] psRoot      Test run root
 * @param[in] *pTime      Build timestamp
 * @date  26.04.2023
 * @date  01.08.2023  Replaced timestamp type
 ******************************************************************************/
void CuTest_PrintRunResults(const cutest_root_ptr_t psRoot, const time_t* pTime)
{
  assert(psRoot != NULL);
  assert(pTime != NULL);

  printf("\n");
  printf("=================== Unit Test Report ===================\n");
  printf("Framework version:  " CUTEST_VERSION "\n");
  printf("Project:            %s\n\n", psRoot->pszName);
  CuTestPrintSummary(psRoot);
  CuTestPrintDetails(psRoot);
  printf("\n");
  printf("Done.\t %s\n", CuTestGetTimestampString(pTime));
  printf("========================================================\n");
}

/*!****************************************************************************
 * @brief
 * Generate HTML report file
 *
 * @param[in] psRoot      Test run root
 * @param[in] *pTime      Build timestamp
 * @param[in] *pszFile    Output filename
 * @date  26.04.2023
 * @date  01.08.2023  Replaced timestamp type
 ******************************************************************************/
void CuTest_GenerateRunReport(const cutest_root_ptr_t psRoot, const time_t* pTime, const char* pszFile)
{
  assert(psRoot != NULL);
  assert(psRoot->pszName != NULL);
  assert(psRoot->ulCount < CUTEST_MAX_NUM_ROOT_ITEM);
  assert(pTime != NULL);
  assert(pszFile != NULL);

  // Open output file
  FILE* f = fopen(pszFile, "w");
  if (f == NULL) return;

  // Header
  fprintf(f,
    "<!DOCTYPE html>\n"
    "<html>\n"
    "    <head>\n"
    "        <title>Unit Test Report</title>\n"
    "    </head>\n"
    "    <body>\n"
    "        <h1>Unit Test Report &ndash; %s</h1><hr/>"
    "        <p><b>Framework Version:</b> CuTest " CUTEST_VERSION "<br/>"
    "           <b>Test run completed at:</b> %s</p>\n",
    psRoot->pszName, CuTestGetTimestampString(pTime)
  );

  // Test results
  unsigned long ulNum = 0;
  for (unsigned long i = 0; i < psRoot->ulCount; ++i)
  {
    const cutest_relem_ptr_t psItem = &psRoot->asItems[i];
    if (psItem != NULL) switch (psItem->eType)
    {
      case EN_CUTEST_TYPE_CASE:
        CuTestGenerateReport_CaseHeader(f);
        CuTestGenerateReport_CaseLine(f, &ulNum, psItem->psCase);
        CuTestGenerateReport_CaseFooter(f);
        break;

      case EN_CUTEST_TYPE_GROUP:
        CuTestGenerateReport_Group(f, &ulNum, psItem->psGroup);
        break;

      case EN_CUTEST_TYPE_MODULE:
        CuTestGenerateReport_Module(f, &ulNum, psItem->psModule);
        break;
    }
  }

  // Statistics
  cutest_stats_t sStats = CuTestGetStats(psRoot);
  fprintf(f,
    "        <hr/><p>%ld runs, %ld passes, %ld fails\n</p>"
    "    </body>\n"
    "</html>", sStats.ulTotal, sStats.ulPassed, sStats.ulFailed
  );
  fclose(f);
}

/*!****************************************************************************
 * @brief
 * Evaluate overall run result
 *
 * @param[in] psRoot      Test run root
 * @return  (cutest_result_t) PASS, if all test cases are marked as "passed".
 * @date  26.04.2023
 ******************************************************************************/
cutest_result_t CuTest_GetRunResult(const cutest_root_ptr_t psRoot)
{
  cutest_stats_t sRun = CuTestGetStats(psRoot);
  return (sRun.ulPassed == sRun.ulTotal) ? EN_CUTEST_RESULT_PASS : EN_CUTEST_RESULT_FAIL;
}
