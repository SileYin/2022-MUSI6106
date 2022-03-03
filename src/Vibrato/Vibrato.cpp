#include "Vibrato.h"
#include "Lfo.h"

CVibrato::CVibrato():
    m_bIsInitialized(false),
    m_fSampleRate(0),
    m_fDelayTimeInS(0),
    m_fVibratoRangeInS(0),
    m_fVibratoFrequencyInHz(0)
{
    this->reset();
}

CVibrato::~CVibrato()
{
    this->reset();
}

Error_t CVibrato::create(CVibrato*& pCVibrato)
{
    pCVibrato = new CVibrato;
    if (!pCVibrato)
        return Error_t::kMemError;
    return Error_t::kNoError;
}

Error_t CVibrato::destroy(CVibrato*& pCVibrato)
{
    if (pCVibrato->m_bIsInitialized)
        pCVibrato->reset();
    delete pCVibrato;
    pCVibrato = 0;
    return Error_t::kNoError;
}

Error_t CVibrato::init(float fDelayTimeInS, float fSampleRateInHz, int iLFOWavetableLength)
{
    if (m_bIsInitialized)
    {
        return Error_t::kFunctionIllegalCallError;
    }

    m_fSampleRate = fSampleRateInHz;
    m_fDelayTimeInS = fDelayTimeInS;

    int iMaxDelayLength = static_cast<int>(2 * std::round(m_fDelayTimeInS * m_fSampleRate)) + 2;

    m_pCLFO = new CWavetableLFO(iLFOWavetableLength, m_fSampleRate, m_fVibratoFrequencyInHz, m_fVibratoRangeInS * m_fSampleRate);
    m_pCRingBuff = new CRingBuffer<float>(iMaxDelayLength);
    m_pCRingBuff->setReadIdx(static_cast<int>(std::round(m_fDelayTimeInS * m_fSampleRate)) + 2);
    m_bIsInitialized = true;

    return Error_t::kNoError;
}

Error_t CVibrato::reset()
{
    if (!m_bIsInitialized)
    {
        return Error_t::kNotInitializedError;
    }
    delete m_pCLFO;
    m_pCLFO = 0;
    delete m_pCRingBuff;
    m_pCRingBuff = 0;
    m_bIsInitialized = false;
    return Error_t::kNoError;
}

Error_t CVibrato::setParam(VibratoParam_t eParam, float fParamValue)
{
    if (!m_bIsInitialized)
    {
        return Error_t::kNotInitializedError;
    }
    if (eParam == CVibrato::kParamVibratoFrequency)
    {
        m_fVibratoFrequencyInHz = fParamValue;
        m_pCLFO->setLFOFrequency(m_fVibratoFrequencyInHz);
    }
    else if (eParam == CVibrato::kParamVibratoRange)
    {
        m_fVibratoRangeInS = fParamValue;
        m_pCLFO->setLFOAmplitude(m_fVibratoRangeInS * m_fSampleRate);
    }
    else
    {
        return Error_t::kFunctionInvalidArgsError;
    }
    return Error_t::kNoError;
}

float CVibrato::getParam(VibratoParam_t eParam) const
{
    if (!m_bIsInitialized)
    {
        return -1;
    }
    if (eParam == CVibrato::kParamVibratoFrequency)
    {
        return m_pCLFO->getLFOFrequency();
    }
    else if (eParam == CVibrato::kParamVibratoRange)
    {
        return m_fSampleRate * m_pCLFO->getLFOAmplitude();
    }
    else
    {
        return -1;
    }
}

Error_t CVibrato::process(float* pfInputBuffer, float* pfOutputBuffer, int iNumberOfFrames)
{
    if (!m_bIsInitialized)
    {
        return Error_t::kNotInitializedError;
    }

    // technically in this case it is number of samples (but called on a block-by-block basis)
    for (int i = 0; i < iNumberOfFrames; i++)
    {
        m_pCRingBuff->putPostInc(pfInputBuffer[i]);
        pfOutputBuffer[i] = m_pCRingBuff->get(m_pCLFO->getPostInc());
        m_pCRingBuff->getPostInc();
    }

    return Error_t::kNoError;
}