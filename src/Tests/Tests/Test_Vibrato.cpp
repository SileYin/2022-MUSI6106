#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Vector.h"
#include "Vibrato.h"
#include "RingBuffer.h"
#include "gtest/gtest.h"


namespace vibrato_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            EXPECT_NEAR(buffer1[i], buffer2[i], fTolerance);
        }
    }

    class RingBuffer : public testing::Test
    {
    protected:
        void SetUp() override
        {
            pCRingBuff = new CRingBuffer<float>(kBlockSize);
        }

        virtual void TearDown()
        {
            delete pCRingBuff;
            pCRingBuff = 0;
        }

        CRingBuffer<float>* pCRingBuff = 0;
        static const int kBlockSize = 17;
    };

    class Vibrato : public testing::Test
    {
    protected:
        void SetUp() override
        {
            CVibrato::create(pCVibrato);
        }

        virtual void TearDown()
        {
            CVibrato::destroy(pCVibrato);
        }

        float* pfInputBuffer;
        float* pfOutputBuffer;
        CVibrato* pCVibrato;
    };



    TEST_F(RingBuffer, IntegerDelay)
    {
        for (int i = 0; i < 5; i++)
        {
            pCRingBuff->putPostInc(i);
        }

        for (int i = 5; i < 30; i++)
        {
            EXPECT_TRUE(pCRingBuff->getNumValuesInBuffer() == 5);
            EXPECT_TRUE(abs(pCRingBuff->getPostInc() - (i - 5)) < 1e-3);
            pCRingBuff->putPostInc(i);
        }
    }

    TEST_F(RingBuffer, FractionalDelay)
    {
        for (int i = 0; i < 5; i++)
        {
            pCRingBuff->putPostInc(i);
        }

        for (int i = 5; i < 30; i++)
        {
            EXPECT_TRUE(pCRingBuff->getNumValuesInBuffer() == 5);
            EXPECT_TRUE(abs(pCRingBuff->get(0.5) - (i - 4.5)) < 1e-3);
            EXPECT_TRUE(abs(pCRingBuff->getPostInc() - (i - 5)) < 1e-3);
            pCRingBuff->putPostInc(i);
        }
    }

    TEST_F(Vibrato, ZeroAmplitude)
    {
        pCVibrato->init(0.01, 48000);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = sin(i / 100);
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        for (int i = 0; i < 2048 - 480; i++)
        {
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i+480]) < 1e-3);
        }
        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, DCInputEqualOutput)
    {
        pCVibrato->init(0.01, 48000);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 10);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.005);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = 1;
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        for (int i = 240; i < 2048 - 480; i++)
        {
            float l_fError = abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]);
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }

        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        for (int i = 96; i < 2048 - 480; i++)
        {
            float l_fError = abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]);
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }

        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, VaryBlockSizeWithZeroAmplitude)
    {
        pCVibrato->init(0.01, 48000);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = sin(i / 100);
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 32);
        for (int i = 32; i < 2048; i <<= 1)
        {
            pCVibrato->process(pfInputBuffer + i, pfOutputBuffer + i, i);
        }
        for (int i = 0; i < 2048 - 480; i++)
        {
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }
        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, VaryBlockSizeWithDCInput)
    {
        pCVibrato->init(0.01, 48000);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 10);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.005);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = 1;
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 32);
        for (int i = 32; i < 2048; i <<= 1)
        {
            pCVibrato->process(pfInputBuffer + i, pfOutputBuffer + i, i);
        }
        for (int i = 240; i < 2048 - 480; i++)
        {
            float l_fError = abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]);
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }

        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 32);
        for (int i = 32; i < 2048; i <<= 1)
        {
            pCVibrato->process(pfInputBuffer + i, pfOutputBuffer + i, i);
        }
        for (int i = 96; i < 2048 - 480; i++)
        {
            float l_fError = abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]);
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }

        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, ZeroInput)
    {
        pCVibrato->init(0.01, 48000);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 10);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.005);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = 0;
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        for (int i = 0; i < 2048 - 480; i++)
        {
            float l_fError = abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]);
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }

        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        for (int i = 0; i < 2048 - 480; i++)
        {
            float l_fError = abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]);
            EXPECT_TRUE(abs(pfInputBuffer[i] - pfOutputBuffer[i + 480]) < 1e-3);
        }

        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }
}

#endif //WITH_TESTS
