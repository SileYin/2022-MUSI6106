#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Synthesis.h"

#include "Vector.h"
#include "FastConv.h"

#include "gtest/gtest.h"


namespace fastconv_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class FastConv: public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pCFastConv = new CFastConv;
        }

        virtual void TearDown()
        {
            delete m_pCFastConv;
            m_pCFastConv = 0;
        }

        CFastConv *m_pCFastConv = 0;
    };

    TEST_F(FastConv, TimeDomainIdentifyTest)
    {
        float pfTestImpulse[51] = { 0 };
        float pfTestInput[10] = { 0 };
        float pfTestOutput[10] = { 0 };
        pfTestInput[3] = 1;
        std::srand(0);
        for (int i = 0; i < 51; i++)
        {
            pfTestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }

        m_pCFastConv->init(pfTestImpulse, 51, 1024, CFastConv::kTimeDomain);
        m_pCFastConv->process(pfTestOutput, pfTestInput, 10);

        CHECK_ARRAY_CLOSE(pfTestOutput + 3, pfTestImpulse, 7, 1e-3);
    }

    TEST_F(FastConv, TimeDomainFlushIdentifyTest)
    {
        float pfTestImpulse[51] = { 0 };
        float pfTestInput[10] = { 0 };
        float pfTestOutput[10] = { 0 };
        float pfTestFlush[50] = { 0 };
        pfTestInput[3] = 1;
        for (int i = 0; i < 51; i++)
        {
            pfTestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }

        m_pCFastConv->init(pfTestImpulse, 51, 1024, CFastConv::kTimeDomain);
        m_pCFastConv->process(pfTestOutput, pfTestInput, 10);
        m_pCFastConv->flushBuffer(pfTestFlush);

        CHECK_ARRAY_CLOSE(pfTestOutput + 3, pfTestImpulse, 7, 1e-3);
        CHECK_ARRAY_CLOSE(pfTestFlush, pfTestImpulse + 7, 51 - 7, 1e-3);
    }

    TEST_F(FastConv, TimeDomainDifferBlockSize)
    {
        float* pfTestImpulse{ new float[16384]{0} };
        float* pfTestInput{ new float[10000]{0} };
        float* pfTestOutput{ new float[10000]{0} };
        float* pfTestFlush{ new float[16383]{0} };
        int piBlockSizes[8] = { 1, 13, 1023, 2048, 1, 17, 5000, 1897 };
        int piStartIdx[8] = { 0 };
        for (int i = 1; i < 8; i++)
        {
            piStartIdx[i] = piStartIdx[i - 1] + piBlockSizes[i - 1];
        }
        pfTestInput[3] = 1;
        for (int i = 0; i < 16384; i++)
        {
            pfTestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
        m_pCFastConv->init(pfTestImpulse, 16384, 1024, CFastConv::kTimeDomain);

        for (int i = 0; i < 8; i++)
        {
            m_pCFastConv->process(pfTestOutput + piStartIdx[i], pfTestInput + piStartIdx[i], piBlockSizes[i]);
        }
        CHECK_ARRAY_CLOSE(pfTestOutput + 3, pfTestImpulse, 10000 - 3, 1e-3);
        m_pCFastConv->flushBuffer(pfTestFlush);
        CHECK_ARRAY_CLOSE(pfTestFlush, pfTestImpulse + 10000 - 3, 16384 - (10000 - 3), 1e-3);

        delete[] pfTestImpulse;
        delete[] pfTestInput;
        delete[] pfTestOutput;
        delete[] pfTestFlush;
 
    }


    TEST_F(FastConv, FreqDomainIdentifyTest)
    {
        float pfTestImpulse[51] = { 0 };
        float pfTestInput[128] = { 0 };
        float pfTestOutput[128] = { 0 };
        pfTestInput[3] = 1;
        std::srand(12);
        for (int i = 0; i < 51; i++)
        {
            pfTestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }

        m_pCFastConv->init(pfTestImpulse, 51, 32, CFastConv::kFreqDomain);
        m_pCFastConv->process(pfTestOutput, pfTestInput, 128);

        CHECK_ARRAY_CLOSE(pfTestOutput + 32 + 3, pfTestImpulse, 51, 1e-3);
    }

    TEST_F(FastConv, FreqDomainFlushIdentifyTest)
    {
        float pfTestImpulse[51] = { 0 };
        float pfTestInput[128] = { 0 };
        float pfTestOutput[128] = { 0 };
        float pfTestFlush[82] = { 0 };
        pfTestInput[3] = 1;
        for (int i = 0; i < 51; i++)
        {
            pfTestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }

        m_pCFastConv->init(pfTestImpulse, 51, 32, CFastConv::kFreqDomain);
        m_pCFastConv->process(pfTestOutput, pfTestInput, 10);
        m_pCFastConv->flushBuffer(pfTestFlush);

        CHECK_ARRAY_CLOSE(pfTestFlush + 3 + 32 - 10, pfTestImpulse, 51, 1e-3);
    }

    TEST_F(FastConv, FreqDomainDifferBlockSize)
    {
        const int blockSize = 1024;
        float* pfTestImpulse{ new float[16384]{0} };
        float* pfTestInput{ new float[10000]{0} };
        float* pfTestOutput{ new float[10000]{0} };
        float* pfTestFlush{ new float[16383 + blockSize]{0} };
        int piBlockSizes[8] = { 1, 13, 1023, 2048, 1, 17, 5000, 1897 };
        int piStartIdx[8] = { 0 };
        for (int i = 1; i < 8; i++)
        {
            piStartIdx[i] = piStartIdx[i - 1] + piBlockSizes[i - 1];
        }
        pfTestInput[3] = 1;
        for (int i = 0; i < 16384; i++)
        {
            pfTestImpulse[i] = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        }
        m_pCFastConv->init(pfTestImpulse, 16384, 1024, CFastConv::kFreqDomain);

        for (int i = 0; i < 8; i++)
        {
            m_pCFastConv->process(pfTestOutput + piStartIdx[i], pfTestInput + piStartIdx[i], piBlockSizes[i]);
        }
        CHECK_ARRAY_CLOSE(pfTestOutput + 3 + blockSize, pfTestImpulse, 10000 - 3 - blockSize, 1e-3);
        m_pCFastConv->flushBuffer(pfTestFlush);
        CHECK_ARRAY_CLOSE(pfTestFlush + blockSize, pfTestImpulse + 10000 - 3, 16384 - (10000 - 3), 1e-3);

        delete[] pfTestImpulse;
        delete[] pfTestInput;
        delete[] pfTestOutput;
        delete[] pfTestFlush;

    }

}

#endif //WITH_TESTS

