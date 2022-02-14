
// standard headers

// project headers
#include "MUSI6106Config.h"

#include "ErrorDef.h"
#include "Util.h"

#include "CombFilterIf.h"
#include "CombFilter.h"

static const char*  kCMyProjectBuildDate = __DATE__;


CCombFilterIf::CCombFilterIf () :
    m_bIsInitialized(false),
    m_pCCombFilter(0),
    m_fSampleRate(0)
{
    // this should never hurt
    this->reset ();
}


CCombFilterIf::~CCombFilterIf ()
{
    this->reset ();
}

const int  CCombFilterIf::getVersion (const Version_t eVersionIdx)
{
    int iVersion = 0;

    switch (eVersionIdx)
    {
    case kMajor:
        iVersion    = MUSI6106_VERSION_MAJOR; 
        break;
    case kMinor:
        iVersion    = MUSI6106_VERSION_MINOR; 
        break;
    case kPatch:
        iVersion    = MUSI6106_VERSION_PATCH; 
        break;
    case kNumVersionInts:
        iVersion    = -1;
        break;
    }

    return iVersion;
}
const char*  CCombFilterIf::getBuildDate ()
{
    return kCMyProjectBuildDate;
}

Error_t CCombFilterIf::create (CCombFilterIf*& pCCombFilter)
{
    pCCombFilter = new CCombFilterIf;
    if (!pCCombFilter)
        return Error_t::kMemError;
    return Error_t::kNoError;
}

Error_t CCombFilterIf::destroy (CCombFilterIf*& pCCombFilter)
{
    delete pCCombFilter;
    pCCombFilter = 0;
    return Error_t::kNoError;
}

Error_t CCombFilterIf::init (CombFilterType_t eFilterType, float fMaxDelayLengthInS, float fSampleRateInHz, int iNumChannels)
{
    if (m_bIsInitialized)
    {
        return Error_t::kFunctionIllegalCallError;
    }
    m_fSampleRate = fSampleRateInHz;
    int iDelayLength = static_cast<int>(fMaxDelayLengthInS * fSampleRateInHz);
    if (eFilterType == CombFilterType_t::kCombFIR)
    {
        m_pCCombFilter = new CCombFIR(iDelayLength, iNumChannels);
        m_bIsInitialized = true;
    }
    else if (eFilterType == CombFilterType_t::kCombIIR)
    {
        m_pCCombFilter = new CCombIIR(iDelayLength, iNumChannels);
        m_bIsInitialized = true;
    }
    else
    {
        return Error_t::kFunctionInvalidArgsError;
    }
    return Error_t::kNoError;
}

Error_t CCombFilterIf::reset ()
{
    if (!m_bIsInitialized)
    {
        return Error_t::kNotInitializedError;
    }
    delete m_pCCombFilter;
    m_pCCombFilter = 0;
    m_bIsInitialized = false;
    return Error_t::kNoError;
}

Error_t CCombFilterIf::process (float **ppfInputBuffer, float **ppfOutputBuffer, int iNumberOfFrames)
{
    if (!m_bIsInitialized)
    {
        return Error_t::kNotInitializedError;
    }
    if (!ppfInputBuffer || !ppfOutputBuffer)
    {
        return Error_t::kMemError;
    }
    assert(iNumberOfFrames > 0);
    m_pCCombFilter->combFilter(ppfInputBuffer, ppfOutputBuffer, iNumberOfFrames);
    return Error_t::kNoError;
}

Error_t CCombFilterIf::setParam (FilterParam_t eParam, float fParamValue)
{
    if (!m_bIsInitialized)
    {
        return Error_t::kNotInitializedError;
    }
    if (eParam == FilterParam_t::kParamGain)
    {
        assert(fabs(fParamValue) <= 1);
        m_pCCombFilter->setGain(fParamValue);
    }
    else if (eParam == FilterParam_t::kParamDelay)
    {
        assert(fParamValue > 0);
        int iDelayLength = static_cast<int>(fParamValue / m_fSampleRate);
        m_pCCombFilter->setDelayLength(iDelayLength);
    }
    else
    {
        return Error_t::kFunctionInvalidArgsError;
    }
    return Error_t::kNoError;
}

float CCombFilterIf::getParam (FilterParam_t eParam) const
{
    return 0;
}
