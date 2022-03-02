#ifndef __Lfo_Hdr__
#define __Lfo_Hdr__

#include "RingBuffer.h"
#include <cmath>

class CWavetableLFO
{
public:
    CWavetableLFO(int iWavetableLength, int iSampleRate, float fFrequency, float fAmplitude):
        m_iSampleRate(iSampleRate),
        m_dReadIdx(0)
    {
        double const l_dPi = 3.14159265358979323846;
        assert(iWavetableLength > 0);
        pCWavetableRingBuff = new CRingBuffer<float>(iWavetableLength);
        for (int i = 0; i < iWavetableLength; i++)
        {
            pCWavetableRingBuff->putPostInc(sin(2 * l_dPi * i / iWavetableLength));
        }
        m_fAmplitude = fAmplitude; 
        m_dReadInc = static_cast<float>(pCWavetableRingBuff->getLength()) / static_cast<float>(m_iSampleRate) * fFrequency;
    }
    ~CWavetableLFO()
    {
        delete pCWavetableRingBuff;
        pCWavetableRingBuff = 0;
    }
    void setReadPhase(float fPhaseInDegrees)
    {
        m_dReadIdx = fPhaseInDegrees / 360 * static_cast<float>(pCWavetableRingBuff->getLength());
    }
    int getWavetableLength() const
    {
        return pCWavetableRingBuff->getLength();
    }
    void setLFOFrequency(float fFrequency, float fPhaseInDegrees = (0.f))
    {
        m_dReadInc = static_cast<float>(pCWavetableRingBuff->getLength()) / static_cast<float>(m_iSampleRate) * fFrequency;
        setReadPhase(fPhaseInDegrees);
    }
    int getLFOFrequency() const
    {
        return m_dReadInc * static_cast<float>(m_iSampleRate) / static_cast<float>(pCWavetableRingBuff->getLength());
    }
    void setLFOAmplitude(float fAmplitude)
    {
        m_fAmplitude = fAmplitude;
    }
    float getLFOAmplitude() const
    {
        return m_fAmplitude;
    }
    float getPostInc()
    {
        float l_fCurrentValue;
        l_fCurrentValue = m_fAmplitude * pCWavetableRingBuff->get(m_dReadIdx);
        m_dReadIdx += m_dReadInc;
        while (m_dReadIdx > pCWavetableRingBuff->getLength())
        {
            m_dReadIdx = m_dReadIdx - pCWavetableRingBuff->getLength();
        }
        return l_fCurrentValue;
    }
private:
    CRingBuffer<float>* pCWavetableRingBuff = 0;
    int m_iSampleRate;
    double m_dReadIdx;
    float m_fAmplitude;
    double m_dReadInc;
};



#endif // !__Lfo_Hdr__
