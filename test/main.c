/*!****************************************************************************
 * @file
 * main.c
 *
 * @copyright Copyright (c) 2023 islandcontroller
 *
 * @brief
 * CuTest Dev Project Demo
 *
 * The full framework source code is published at:
 * https://github.com/islandcontroller/cutest
 *
 * @date  26.04.2023
 ******************************************************************************/

/*- Header files -------------------------------------------------------------*/
#include "CuTest.h"


/*- Sample test definitions --------------------------------------------------*/
TEST_CASE(TEST_Module_MyGroup_Case1)
{
  CuPass();
}

TEST_CASE(TEST_Module_MyGroup_Case2)
{
  CuFail("always fails");
}

TEST_GROUP(TestMyModule_MyGroup)
{
  TEST_Module_MyGroup_Case1,
  TEST_Module_MyGroup_Case2
};

TEST_MODULE(TestMyModule)
{
  TestMyModule_MyGroup
};

/*!****************************************************************************
 * @brief
 * Test Runner main application
 *
 * @date  26.04.2023
 ******************************************************************************/
int main(void)
{
  BEGIN_TEST_RUN();
  RUN_TEST_MODULE(TestMyModule);
  END_TEST_RUN();

  return GET_RUN_RESULT();
}
