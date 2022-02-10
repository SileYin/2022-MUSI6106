#include "CombFilter.h"

CCombFilterBase::CCombFilterBase(int iDelayInSamples, int iNumChannels)
	:m_fGain(0)
{
	m_iDelayLength = iDelayInSamples;
	m_iNumChannels = iNumChannels;
	pCRingBuff = new CRingBuffer<float>*[m_iNumChannels];
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		pCRingBuff[i] = new CRingBuffer<float>(m_iDelayLength);
	}
}


CCombFilterBase::~CCombFilterBase()
{
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		delete pCRingBuff[i];
	}
	delete [] pCRingBuff;
	pCRingBuff = 0;
}

Error_t CCombFIR::combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		for (int j = 0; j < iNumberOfFrames; ++j)
		{
			ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + m_fGain * pCRingBuff[i]->getPostInc();
			pCRingBuff[i]->putPostInc(ppfInputBuffer[i][j]);
		}
	}
}

Error_t CCombIIR::combFilter(float** ppfInputBuffer, float** ppfOutputBuffer, int iNumberOfFrames)
{
	for (int i = 0; i < m_iNumChannels; ++i)
	{
		for (int j = 0; j < iNumberOfFrames; ++j)
		{
			ppfOutputBuffer[i][j] = ppfInputBuffer[i][j] + m_fGain * pCRingBuff[i]->getPostInc();
			pCRingBuff[i]->putPostInc(ppfOutputBuffer[i][j]);
		}
	}
}
