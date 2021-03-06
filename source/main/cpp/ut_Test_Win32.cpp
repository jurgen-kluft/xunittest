#ifdef TARGET_PC

#include "xunittest/private/ut_Config.h"
#include "xunittest/private/ut_Test.h"
#include "xunittest/private/ut_TestList.h"
#include "xunittest/private/ut_TestResults.h"
#include "xunittest/private/ut_TimeHelpers.h"
#include "xunittest/private/ut_AssertException.h"
#include "xunittest/private/ut_StringBuilder.h"
#include "xunittest/private/ut_Stdout.h"
#include <exception>

namespace UnitTest
{
	void Test::run(TestResults& testResults, int const maxTestTimeInMs)
	{
		Timer testTimer;
		testTimer.start();
		testResults.onTestStart(mTestName);

		try
		{
			runImpl(testResults);
		}
		catch (AssertException const& e)
		{
			testResults.onTestFailure(e.filename(), e.lineNumber(), mTestName, e.what());
		}
		catch (std::exception const& e)
		{
			StringBuilder stringBuilder;
			stringBuilder << "Unhandled exception: " << e.what();
			testResults.onTestFailure(mFilename, mLineNumber, mTestName, stringBuilder.getText());
		}
		catch (...)
		{
			testResults.onTestFailure(mFilename, mLineNumber, mTestName, "Unhandled exception: Crash!");
		}
		const int testTimeInMs = testTimer.getTimeInMs();
		if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !mTimeConstraintExempt)
		{
			StringBuilder stringBuilder;
			stringBuilder << "Global time constraint failed. Expected under ";
			stringBuilder << maxTestTimeInMs;
			stringBuilder << "ms but took ";
			stringBuilder << testTimeInMs;
			stringBuilder << "ms.";

			testResults.onTestFailure(mFilename, mLineNumber, mTestName, stringBuilder.getText());
		}
		testResults.onTestEnd(mTestName, testTimeInMs/1000.0f);
	}

	void		TestFixture::run(TestResults& testResults_, int const maxTestTimeInMs)
	{
		Timer testTimer;
		testTimer.start();

		int numTests = 0;
		if (mTests != 0)
		{
			Test* curTest = mTests;
			while (curTest != 0)
			{
				numTests++;
				curTest = curTest->mNext;
			}
			testResults_.onTestFixtureStart(mTestName, numTests);
		}

		mStep = FIXTURE_SETUP;
		try
		{
			// Remember allocation count X
			int iAllocCntX = GetNumAllocations();
			int iMemLeakCnt = 0;
			int iExtraDeallocCnt = 0;
			setup(testResults_);

			if (mTests != 0)
			{
				mStep = FIXTURE_UNITTESTS;
				Test* curTest = mTests;
				while (curTest != 0)
				{
					// Remember allocation count Y
					int iAllocCntY = GetNumAllocations();
					curTest->run(testResults_, maxTestTimeInMs);
					// Compare allocation count with Y
					// If different => memory leak error
					if (iAllocCntY != GetNumAllocations())
					{
						int iAllocCountDifference = (GetNumAllocations() - iAllocCntY);
						
						StringBuilder str;
						if(iAllocCountDifference > 0)
						{
							iMemLeakCnt += iAllocCountDifference;
							str << "memory leak detected, leaked memory allocations: ";
							str << iAllocCountDifference;
						}
						else
						{
							iExtraDeallocCnt += -1*iAllocCountDifference;
							str << "extra memory deallocations detected, unmatching deallocations: ";
							str << -1*iAllocCountDifference;
						}

						testResults_.onTestFailure(curTest->mFilename, curTest->mLineNumber, curTest->mTestName, str.getText());
					}
					curTest = curTest->mNext;
				}
			}

			mStep = FIXTURE_TEARDOWN;
			teardown(testResults_);
			// Compare allocation count with X
			// If different => Fixture memory leak error (probably the combination of Setup() and Teardown()
			if (iAllocCntX != (GetNumAllocations() - iMemLeakCnt))
			{
				StringBuilder str;

				str << "memory leak detected in setup()/teardown(), leaked memory allocations: ";
				str << iMemLeakCnt;
				

				testResults_.onTestFailure(mFilename, mLineNumber, mTestName, str.getText());
			}

			if( iAllocCntX != (GetNumAllocations() - iExtraDeallocCnt))
			{
				StringBuilder str;

				str << "extra deallocations detected in setup()/teardown(), extra deallocations: ";
				str << iExtraDeallocCnt;
				

				testResults_.onTestFailure(mFilename, mLineNumber, mTestName, str.getText());
			}

		}
		catch (std::exception const& e)
		{
			StringBuilder stringBuilder;
			if (mStep == FIXTURE_SETUP)
				stringBuilder << "Unhandled exception in setup of fixture " << mTestName;
			else if (mStep == FIXTURE_TEARDOWN)
				stringBuilder << "Unhandled exception in teardown of fixture " << mTestName;
			else
				stringBuilder << "Unhandled exception in fixture " << mTestName;

			stringBuilder << " : " << e.what();
			testResults_.onTestFailure(mFilename, mLineNumber, mTestName, stringBuilder.getText());
		}
		catch (...)
		{
			StringBuilder stringBuilder;
			if (mStep == FIXTURE_SETUP)
				stringBuilder << "Unhandled exception in setup of fixture " << mTestName;
			else if (mStep == FIXTURE_TEARDOWN)
				stringBuilder << "Unhandled exception in teardown of fixture " << mTestName;
			else
				stringBuilder << "Unhandled exception in fixture " << mTestName;

			testResults_.onTestFailure(mFilename, mLineNumber, mTestName, stringBuilder.getText());
		}

		const int testTimeInMs = testTimer.getTimeInMs();
		if (maxTestTimeInMs > 0 && testTimeInMs > maxTestTimeInMs && !mTimeConstraintExempt)
		{
			StringBuilder stringBuilder;
			stringBuilder << "Fixture time constraint failed. Expected under ";
			stringBuilder << maxTestTimeInMs;
			stringBuilder << "ms but took ";
			stringBuilder << testTimeInMs;
			stringBuilder << "ms.";

			testResults_.onTestFailure(mFilename, mLineNumber, mTestName, stringBuilder.getText());
		}
		testResults_.onTestEnd(mTestName, testTimeInMs/1000.0f);
	}
}

#endif
