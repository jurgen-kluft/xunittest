#include "xunittest\xunittest.h"
#include "xunittest\private\ut_TestReporter.h"
#include "xunittest\private\ut_Stdout.h"

UNITTEST_SUITE_LIST(xUnitTestUnitTest);

UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestAssertHandler);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestStringBuilder);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestCpp);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestCheckMacros);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestChecks);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestTest);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestMacros);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestTestResults);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestTimeConstraint);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestThreadSuite);
UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestCountingAllocator);

// UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestTestList);		//*
// UNITTEST_SUITE_DECLARE(xUnitTestUnitTest, TestTestRunner);		//*

#ifdef TARGET_PC
class UnitTestAllocator : public UnitTest::Allocator
{
public:
	int		mNumAllocations;

	UnitTestAllocator()
		: mNumAllocations(0)
	{
	}

	void*	Allocate(int size)
	{
		++mNumAllocations;
		return _aligned_malloc(size, 4);
	}

	void	Deallocate(void* ptr)
	{
		--mNumAllocations;
		_aligned_free(ptr);
	}

	void	Release()
	{
		if (mNumAllocations != 0)
		{
			UnitTest::Stdout::Trace("ERROR: System Allocator is being released but still has allocations that are not freed\n");
			mNumAllocations = 0;
		}
	}
};
#endif

bool	gRunUnitTest(UnitTest::TestReporter& reporter)
{
#ifdef TARGET_PC
	UnitTestAllocator unittestAllocator;
	UnitTest::SetAllocator(&unittestAllocator);

	int r = UNITTEST_SUITE_RUN(reporter, xUnitTestUnitTest);

	unittestAllocator.Release();
	UnitTest::SetAllocator(NULL);

	return r == 0;
#else
	return false;
#endif
}
