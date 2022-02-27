#ifndef __Lfo_Hdr__
#define __Lfo_Hdr__

#include "RingBuffer.h"
#include <cmath>

class CWavetableLFO
{
public:
	CWavetableLFO(int iWavetableLength, int iSampleRate, float fFrequency, float fAmplitude):
		m_iSampleRate(iSampleRate),
		m_fReadIdx(0.f)
	{
		double const l_dPi = 3.14159265358979323846;
		pCWavetableRingBuff = new CRingBuffer<float>(iWavetableLength);
		for (int i = 0; i < iWavetableLength; i++)
		{
			pCWavetableRingBuff->putPostInc(sin(2 * l_dPi * i / iWavetableLength));
		}
		m_fAmplitude = fAmplitude;
		m_fReadInc = static_cast<float>(iSampleRate) / static_cast<float>(iWavetableLength) / fFrequency;
	}
	~CWavetableLFO()
	{
		delete pCWavetableRingBuff;
		pCWavetableRingBuff = 0;
	}
	void setReadPhase(float fPhaseInDegrees)
	{
		m_fReadIdx = fPhaseInDegrees / 360 * static_cast<float>(pCWavetableRingBuff->getLength());
	}
	int getWavetableLength() const
	{
		return pCWavetableRingBuff->getLength();
	}
	void setLFOFrequency(float fFrequency, float fPhaseInDegrees = (0.f))
	{
		m_fReadInc = static_cast<float>(m_iSampleRate) / static_cast<float>(pCWavetableRingBuff->getLength()) / fFrequency;
		setReadPhase(fPhaseInDegrees);
	}
	int getLFOFrequency() const
	{
		return m_fReadInc * static_cast<float>(m_iSampleRate) / static_cast<float>(pCWavetableRingBuff->getLength());
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
		l_fCurrentValue = m_fAmplitude * pCWavetableRingBuff->get(m_fReadIdx);
		m_fReadIdx += m_fReadInc;
		while (m_fReadIdx > pCWavetableRingBuff->getLength())
		{
			m_fReadIdx = m_fReadIdx - pCWavetableRingBuff->getLength();
		}
		return l_fCurrentValue;
	}
private:
	CRingBuffer<float>* pCWavetableRingBuff = 0;
	int m_iSampleRate;
	float m_fReadIdx;
	float m_fAmplitude;
	float m_fReadInc;
};



#endif // !__Lfo_Hdr__
