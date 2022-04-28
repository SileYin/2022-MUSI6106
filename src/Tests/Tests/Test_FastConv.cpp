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


namespace alex_fastconv_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class AlexFastConv : public testing::Test
    {
    protected:
        void SetUp() override
        {
            m_pfInput = new float[m_iInputLength];
            m_pfIr = new float[m_iIRLength];
            m_pfOutput = new float[m_iInputLength + m_iIRLength];

            CVectorFloat::setZero(m_pfInput, m_iInputLength);
            m_pfInput[0] = 1;

            CSynthesis::generateNoise(m_pfIr, m_iIRLength);
            m_pfIr[0] = 1;

            CVectorFloat::setZero(m_pfOutput, m_iInputLength + m_iIRLength);

            m_pCFastConv = new CFastConv();
        }

        virtual void TearDown()
        {
            m_pCFastConv->reset();
            delete m_pCFastConv;

            delete[] m_pfIr;
            delete[] m_pfOutput;
            delete[] m_pfInput;
        }

        float* m_pfInput = 0;
        float* m_pfIr = 0;
        float* m_pfOutput = 0;

        int m_iInputLength = 83099;
        int m_iIRLength = 60001;

        CFastConv* m_pCFastConv = 0;
    };

    TEST_F(AlexFastConv, Params)
    {
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(0, 1));
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 0));
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 10, -1));
        EXPECT_EQ(false, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 10, 7));
        EXPECT_EQ(true, Error_t::kNoError == m_pCFastConv->init(m_pfIr, 10, 4));
        EXPECT_EQ(true, Error_t::kNoError == m_pCFastConv->reset());
    }

    TEST_F(AlexFastConv, Impulse)
    {
        // impulse with impulse
        int iBlockLength = 4;
        m_pCFastConv->init(m_pfIr, 1, iBlockLength);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(1.F, m_pfOutput[iBlockLength], 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMin(m_pfOutput, m_iInputLength), 1e-6F);
        EXPECT_NEAR(1.F, CVectorFloat::getMax(m_pfOutput, m_iInputLength), 1e-6F);

        // impulse with dc
        for (auto i = 0; i < 4; i++)
            m_pfOutput[i] = 1;
        iBlockLength = 8;
        m_pCFastConv->init(m_pfOutput, 4, iBlockLength);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(0.F, CVectorFloat::getMean(m_pfOutput, 8), 1e-6F);
        EXPECT_NEAR(1.F, CVectorFloat::getMean(&m_pfOutput[8], 4), 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMean(&m_pfOutput[12], 400), 1e-6F);

        // impulse with noise
        iBlockLength = 8;
        m_pCFastConv->init(m_pfIr, 27, iBlockLength);

        for (auto i = 0; i < m_iInputLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], 27, 1e-6F);
        CHECK_ARRAY_CLOSE(&m_pfInput[1], &m_pfOutput[iBlockLength + 27], 10, 1e-6F);

        // noise with impulse
        iBlockLength = 8;
        m_pCFastConv->init(m_pfInput, 27, iBlockLength);

        for (auto i = 0; i < m_iIRLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfIr[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], m_iIRLength - iBlockLength, 1e-6F);
    }
    TEST_F(AlexFastConv, ImpulseTime)
    {
        // impulse with impulse
        int iBlockLength = 4;
        m_pCFastConv->init(m_pfIr, 1, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(1.F, m_pfOutput[0], 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMin(m_pfOutput, m_iInputLength), 1e-6F);
        EXPECT_NEAR(1.F, CVectorFloat::getMax(m_pfOutput, m_iInputLength), 1e-6F);

        // impulse with dc
        for (auto i = 0; i < 4; i++)
            m_pfOutput[i] = 1;
        iBlockLength = 8;
        m_pCFastConv->init(m_pfOutput, 4, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < 500; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        EXPECT_NEAR(1.F, CVectorFloat::getMean(&m_pfOutput[0], 4), 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMean(&m_pfOutput[4], 400), 1e-6F);

        // impulse with noise
        iBlockLength = 8;
        m_pCFastConv->init(m_pfIr, 27, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < m_iInputLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[0], 27, 1e-6F);
        CHECK_ARRAY_CLOSE(&m_pfInput[1], &m_pfOutput[27], 10, 1e-6F);

        // noise with impulse
        iBlockLength = 8;
        m_pCFastConv->init(m_pfInput, 27, iBlockLength, CFastConv::kTimeDomain);

        for (auto i = 0; i < m_iIRLength; i++)
            m_pCFastConv->process(&m_pfOutput[i], &m_pfIr[i], 1);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[0], m_iIRLength, 1e-6F);
    }

    TEST_F(AlexFastConv, BlockLengths)
    {
        // impulse with noise
        int iBlockLength = 4;

        for (auto j = 0; j < 10; j++)
        {
            m_pCFastConv->init(m_pfIr, 51, iBlockLength);

            for (auto i = 0; i < m_iInputLength; i++)
                m_pCFastConv->process(&m_pfOutput[i], &m_pfInput[i], 1);

            CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], 51 - iBlockLength, 1e-6F);

            iBlockLength <<= 1;
        }
    }

    TEST_F(AlexFastConv, InputLengths)
    {
        // impulse with noise
        int iBlockLength = 4096;

        int iCurrIdx = 0,
            aiInputLength[] = {
            4095,
            17,
            32157,
            99,
            4097,
            1,
            42723

        };

        m_pCFastConv->init(m_pfIr, m_iIRLength, iBlockLength);

        for (auto i = 0; i < 7; i++)
        {
            m_pCFastConv->process(&m_pfOutput[iCurrIdx], &m_pfInput[iCurrIdx], aiInputLength[i]);
            iCurrIdx += aiInputLength[i];
        }

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], m_iIRLength, 1e-6F);
        EXPECT_NEAR(0.F, CVectorFloat::getMean(&m_pfOutput[m_iIRLength + iBlockLength], 10000), 1e-6F);

    }

    TEST_F(AlexFastConv, FlushBuffer)
    {
        // impulse with noise
        int iBlockLength = 8;
        int iIrLength = 27;

        CVectorFloat::setZero(m_pfOutput, m_iInputLength + m_iIRLength);
        m_pCFastConv->init(m_pfIr, iIrLength, iBlockLength);

        m_pCFastConv->process(m_pfOutput, m_pfInput, 1);

        m_pCFastConv->flushBuffer(&m_pfOutput[1]);

        EXPECT_NEAR(0.F, CVectorFloat::getMean(m_pfOutput, iBlockLength), 1e-6F);
        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[iBlockLength], iIrLength, 1e-6F);

        // same for time domain
        CVectorFloat::setZero(m_pfOutput, m_iInputLength + m_iIRLength);
        m_pCFastConv->init(m_pfIr, iIrLength, iBlockLength, CFastConv::kTimeDomain);

        m_pCFastConv->process(m_pfOutput, m_pfInput, 1);

        m_pCFastConv->flushBuffer(&m_pfOutput[1]);

        CHECK_ARRAY_CLOSE(m_pfIr, &m_pfOutput[0], iIrLength, 1e-6F);
    }
}

#endif //WITH_TESTS

