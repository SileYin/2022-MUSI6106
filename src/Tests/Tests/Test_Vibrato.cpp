#include "MUSI6106Config.h"

#ifdef WITH_TESTS
#include "Vector.h"
#include "Vibrato.h"
#include "RingBuffer.h"
#include "gtest/gtest.h"
#include "Synthesis.h"

namespace vibrato_test {
    void CHECK_ARRAY_CLOSE(float* buffer1, float* buffer2, int iLength, float fTolerance)
    {
        for (int i = 0; i < iLength; i++)
        {
            float error = abs(buffer1[i] - buffer2[i]);
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
        Vibrato(){
            fDelayInSec = 0.01f;
            fSampleRateInHz = 48000.f;
            fDelayInSample = static_cast<int>(fDelayInSec * fSampleRateInHz);
        }
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
        //init parameter
        float fDelayInSec;
        float fSampleRateInHz;
        int   fDelayInSample;
    };

    TEST_F(RingBuffer, BufferLength)
    {
        EXPECT_EQ(pCRingBuff->getLength(), 17);
    }

    TEST_F(RingBuffer, IntegerDelay)
    {
        for (int i = 0; i < 5; i++)
        {
            pCRingBuff->putPostInc(1.F * i);
        }

        for (int i = 5; i < 30; i++)
        {
            EXPECT_EQ(pCRingBuff->getNumValuesInBuffer(), 5);
            EXPECT_NEAR(pCRingBuff->getPostInc(), i - 5, 1e-3);
            pCRingBuff->putPostInc(1.F * i);
        }
    }

    TEST_F(RingBuffer, FractionalDelay)
    {
        for (int i = 0; i < 5; i++)
        {
            pCRingBuff->putPostInc(1.F * i);
        }

        for (int i = 5; i < 30; i++)
        {
            EXPECT_EQ(pCRingBuff->getNumValuesInBuffer(), 5);
            if(i > 5)
                EXPECT_NEAR(pCRingBuff->get(-0.5), i - 5.5, 1e-3);
            EXPECT_NEAR(pCRingBuff->getPostInc(), i - 5, 1e-3);
            pCRingBuff->putPostInc(1.F * i);
        }
    }

    TEST_F(RingBuffer, Overflowing)
    {
        for (int i = 0; i < 25; i++)
        {
            pCRingBuff->putPostInc(1.F * i);
        }

        for (int i = 25; i < 30; i++)
        {
            EXPECT_EQ(pCRingBuff->getNumValuesInBuffer(), 25 - 17);
            EXPECT_NEAR(pCRingBuff->getPostInc(), i - (25 - 17), 1e-3);
            pCRingBuff->putPostInc(1.F * i);
        }
    }

    TEST_F(RingBuffer, ReadBeforeWrite)
    {
        for (int i = 0; i < 5; i++)
        {
            EXPECT_NEAR(pCRingBuff->getPostInc(), 0, 1e-3);
        }
    }

    TEST_F(RingBuffer, StuckReadHead)
    {
        for (int i = 0; i < 5; i++)
        {
            pCRingBuff->putPostInc(1.F * i);
        }

        for (int i = 5; i < 30; i++)
        {
            pCRingBuff->setReadIdx(0);
            EXPECT_EQ(pCRingBuff->getNumValuesInBuffer(), pCRingBuff->getWriteIdx());
            EXPECT_NEAR(fmod(pCRingBuff->getPostInc(), 17), 0, 1e-3);
            pCRingBuff->putPostInc(1.F * i);
        }
    }

    TEST_F(RingBuffer, StuckWriteHead)
    {
        for (int i = 0; i < 5; i++)
        {
            pCRingBuff->setWriteIdx(0);
            pCRingBuff->putPostInc(1.F * i);
        }

        for (int i = 5; i < 30; i++)
        {
            pCRingBuff->setWriteIdx(0);
            EXPECT_EQ(pCRingBuff->getNumValuesInBuffer(), (17 - pCRingBuff->getReadIdx()) % 17);
            if (pCRingBuff->getReadIdx() == 0)
            {
                EXPECT_NEAR(pCRingBuff->getPostInc(), i - 1, 1e-3);
            }
            else
            {
                EXPECT_NEAR(pCRingBuff->getPostInc(), 0, 1e-3);
            }
            pCRingBuff->putPostInc(1.F * i);
        }
    }

    TEST_F(Vibrato, ZeroAmplitude)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = sin(i / 100);
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        CHECK_ARRAY_CLOSE(pfInputBuffer, pfOutputBuffer + fDelayInSample, 2048 - fDelayInSample, 1e-3);
        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, DCInputEqualOutput)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 10);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.005);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = 1;
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        CHECK_ARRAY_CLOSE(pfInputBuffer + 240, pfOutputBuffer + 240 + 480, 2048 - 480 - 240, 1e-3);

        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        CHECK_ARRAY_CLOSE(pfInputBuffer + 96, pfOutputBuffer + 96 + 480, 2048 - 480 - 96, 1e-3);


        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, VaryBlockSizeWithZeroAmplitude)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
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
        CHECK_ARRAY_CLOSE(pfInputBuffer, pfOutputBuffer + fDelayInSample, 2048 - fDelayInSample, 1e-3);
        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, VaryBlockSizeWithDCInput)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
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
        CHECK_ARRAY_CLOSE(pfInputBuffer + 240, pfOutputBuffer + 240 + fDelayInSample, 2048 - fDelayInSample - 240, 1e-3);

        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 32);
        for (int i = 32; i < 2048; i <<= 1)
        {
            pCVibrato->process(pfInputBuffer + i, pfOutputBuffer + i, i);
        }
        CHECK_ARRAY_CLOSE(pfInputBuffer + 96, pfOutputBuffer + 96 + fDelayInSample, 2048 - fDelayInSample - 96, 1e-3);

        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, ZeroInput)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 10);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.005);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = 0;
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        CHECK_ARRAY_CLOSE(pfInputBuffer + 240, pfOutputBuffer + 240 + fDelayInSample, 2048 - fDelayInSample - 240, 1e-3);

        pCVibrato->reset();
        pCVibrato->init(0.01, 48000);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        CHECK_ARRAY_CLOSE(pfInputBuffer + 96, pfOutputBuffer + 96 + 480, 2048 - 480 - 96, 1e-3);

        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }

    TEST_F(Vibrato, PositiveValueStayPositive)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        for (int i = 0; i < 2048; i++)
        {
            pfInputBuffer[i] = i + 1 / 100;
        }
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        for (int i = 480; i < 2048; i++)
        {
            EXPECT_GE(pfOutputBuffer[i], 0);
        }

        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
    }
    TEST_F(Vibrato, CreateAndDestroy)
    {
        CVibrato* pCVibrato_Test;
        CVibrato::create(pCVibrato_Test);

        // initialize and set values
        pCVibrato_Test->init(fDelayInSec, fSampleRateInHz);
        pCVibrato_Test->setParam(CVibrato::kParamVibratoFrequency, 20);
        pCVibrato_Test->setParam(CVibrato::kParamVibratoRange, 1);
        
        CVibrato::destroy(pCVibrato_Test);

        // check if vibrato is zero
        EXPECT_EQ(pCVibrato_Test, nullptr);
    }


    TEST_F(Vibrato, SinDiffBound)
    {
        pCVibrato->init(fDelayInSec, fSampleRateInHz);
        pfInputBuffer = new float[2048];
        pfOutputBuffer = new float[2048];
        pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 5);
        pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.001);
        CSynthesis::generateSine(pfInputBuffer, 200, 48000, 2048, 1.0, 0.0);
        
        pCVibrato->process(pfInputBuffer, pfOutputBuffer, 2048);
        CHECK_ARRAY_CLOSE(pfInputBuffer, pfOutputBuffer + fDelayInSample, 2048 - fDelayInSample, 2);
        
        
        delete[] pfInputBuffer;
        delete[] pfOutputBuffer;
        
    }




}

#endif //WITH_TESTS
