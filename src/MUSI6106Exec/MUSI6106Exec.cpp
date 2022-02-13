
#include <iostream>
#include <ctime>
#include <cmath>
#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "CombFilterIf.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();
void testFreqMatch(CCombFilterIf::CombFilterType_t eFilterType, int iCycleInSamples = 100, int iNumChannels = 2, int iTestLength = 1024, int iSampleRateInHz = 44100);
void testVaryBlockSize(CCombFilterIf::CombFilterType_t eFilterType, int iCycleInSamples = 100, int iSampleRateInHz = 44100);
void combFilterVaryTestBlock(CCombFilterIf* phCombFilter, float** &ppfInputTestSignal, float** &ppfOutputTestSignal, const float* inputTestSequence, float** outputTestSequence, int& iCurBlockHead, int iBlockLength);
void testZeroInput(CCombFilterIf::CombFilterType_t eFilterType, int iDelayInSample = 50, int iNumChannels = 2, int iTestLength = 1024, int iSampleRateInHz = 44100);
void testDCInput(CCombFilterIf::CombFilterType_t eFilterType, int iDelayInSample = 1, int iNumChannels = 2, int iTestLength = 1024, int iSampleRateInHz = 44100);

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath,
                sCombFilterType;

    float fCombFilterGain, fCombFilterDelayInS;


    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfInputAudioData = 0;
    float **ppfOutputAudioData = 0;

    CAudioFileIf *phInputAudioFile = 0;
    CAudioFileIf *phOutputAudioFile = 0;
    std::fstream hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    CCombFilterIf* phCombFilter = 0;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout << "No arguments inputed, running tests." << endl;
        testFreqMatch(CCombFilterIf::kCombFIR);
        testFreqMatch(CCombFilterIf::kCombIIR);
        testVaryBlockSize(CCombFilterIf::kCombFIR);
        testVaryBlockSize(CCombFilterIf::kCombIIR);
        testZeroInput(CCombFilterIf::kCombFIR);
        testZeroInput(CCombFilterIf::kCombIIR);
        testDCInput(CCombFilterIf::kCombFIR);
        testDCInput(CCombFilterIf::kCombIIR);
        return 0;
    }
    else if (argc == 6)
    {
        sInputFilePath = argv[1];
        sOutputFilePath = argv[2];
        sCombFilterType = argv[3];
        fCombFilterDelayInS = std::stof(argv[4]);
        fCombFilterGain = std::stof(argv[5]);
    }
    else
    {
        cout << "Not enough arguments." << endl;
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phInputAudioFile);
    phInputAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phInputAudioFile->isOpen())
    {
        cout << "Input wave file open error!";
        CAudioFileIf::destroy(phInputAudioFile);
        return -1;
    }
    phInputAudioFile->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // create and open the output wave file
    hOutputFile.open(sOutputFilePath, std::ios::out | std::ios::app);
    hOutputFile.close();
    CAudioFileIf::create(phOutputAudioFile);
    phOutputAudioFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phOutputAudioFile->isOpen())
    {
        cout << "Output wave file open error!";
        CAudioFileIf::destroy(phInputAudioFile);
        CAudioFileIf::destroy(phOutputAudioFile);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // create comb filter instance
    CCombFilterIf::create(phCombFilter);
    if (sCombFilterType == "FIR")
    {
        phCombFilter->init(CCombFilterIf::kCombFIR, fCombFilterDelayInS, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    else if (sCombFilterType == "IIR")
    {
        phCombFilter->init(CCombFilterIf::kCombIIR, fCombFilterDelayInS, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    else
    {
        cout << "Unknown filter type, defaulting FIR." << endl;
        phCombFilter->init(CCombFilterIf::kCombFIR, fCombFilterDelayInS, stFileSpec.fSampleRateInHz, stFileSpec.iNumChannels);
    }
    phCombFilter->setParam(CCombFilterIf::kParamGain, fCombFilterGain);

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfInputAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfInputAudioData[i] = new float[kBlockSize];
    ppfOutputAudioData = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfOutputAudioData[i] = new float[kBlockSize];

    if ((ppfInputAudioData == 0)||(ppfOutputAudioData == 0))
    {
        CAudioFileIf::destroy(phInputAudioFile);
        CAudioFileIf::destroy(phOutputAudioFile);
        return -1;
    }
    if ((ppfInputAudioData[0] == 0)||(ppfOutputAudioData[0] == 0))
    {
        CAudioFileIf::destroy(phInputAudioFile);
        CAudioFileIf::destroy(phOutputAudioFile);
        return -1;
    }
    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phInputAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;
        phInputAudioFile->readData(ppfInputAudioData, iNumFrames);
        phCombFilter->process(ppfInputAudioData, ppfOutputAudioData, kBlockSize);
        phOutputAudioFile->writeData(ppfOutputAudioData, iNumFrames);
        cout << "time elapsed: " << (clock() - time) * 1.F / CLOCKS_PER_SEC << "s." << endl;
    }

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CCombFilterIf::destroy(phCombFilter);
    CAudioFileIf::destroy(phInputAudioFile);
    CAudioFileIf::destroy(phOutputAudioFile);

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfInputAudioData[i];
    delete[] ppfInputAudioData;
    ppfInputAudioData = 0;
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfOutputAudioData[i];
    delete[] ppfOutputAudioData;
    ppfOutputAudioData = 0;
    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout  << endl;

    return;
}

void testFreqMatch(CCombFilterIf::CombFilterType_t eFilterType, int iCycleInSamples, int iNumChannels, int iTestLength, int iSampleRateInHz)
{
    if (eFilterType == CCombFilterIf::kCombFIR) 
    {
        cout << "Running FIR comb filter frequency match test... ";
    }
    else if (eFilterType == CCombFilterIf::kCombIIR) 
    {
        cout << "Running IIR comb filter frequency match test... ";
    }
    const float fMathPi = 3.14159265358979f;
    bool bTestPassedFlag = true;
    float** ppfInputTestSignal;
    float** ppfOutputTestSignal;
    CCombFilterIf* phCombFilter = 0;


    CCombFilterIf::create(phCombFilter);
    ppfInputTestSignal = new float* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
        ppfInputTestSignal[i] = new float[iTestLength];
    ppfOutputTestSignal = new float* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
        ppfOutputTestSignal[i] = new float[iTestLength];
    phCombFilter->init(eFilterType, static_cast<float>(iCycleInSamples) / static_cast<float>(iSampleRateInHz), static_cast<float>(iSampleRateInHz), iNumChannels);
    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        phCombFilter->setParam(CCombFilterIf::kParamGain, -1);
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        phCombFilter->setParam(CCombFilterIf::kParamGain, 1);
    }
    for (int i = 0; i < iNumChannels; i++)
    {
        for (int j = 0; j < iTestLength; j++)
        {
            ppfInputTestSignal[i][j] = sin(2 * fMathPi / static_cast<float>(iCycleInSamples) * static_cast<float>(j));
        }
    }

    phCombFilter->process(ppfInputTestSignal, ppfOutputTestSignal, iTestLength);


    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        for (int i = 0; i < iNumChannels; i++)
        {
            for (int j = iCycleInSamples; j < iTestLength; j++)
            {
                if (abs(ppfOutputTestSignal[i][j]) > 1e-4) bTestPassedFlag = false;
            }
        }
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        for (int i = 0; i < iNumChannels; i++)
        {
            for (int j = iCycleInSamples; j < iTestLength; j++)
            {
                if (abs(ppfInputTestSignal[i][j]) > 1e-4) 
                    if (abs(ppfOutputTestSignal[i][j]/ppfInputTestSignal[i][j]) < 0.5) bTestPassedFlag = false;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    for (int i = 0; i < iNumChannels; i++)
        delete[] ppfInputTestSignal[i];
    delete[] ppfInputTestSignal;
    for(int i = 0; i < iNumChannels; i++)
        delete[] ppfOutputTestSignal[i];
    delete[] ppfOutputTestSignal;
    if (bTestPassedFlag)
    {
        cout << "passed!" << endl;
    }
    else
    {
        cout << "not passed." << endl;
    }
}

void testVaryBlockSize(CCombFilterIf::CombFilterType_t eFilterType, int iCycleInSamples, int iSampleRateInHz)
{
    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        cout << "Running FIR comb filter vary block size frequency match test... ";
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        cout << "Running IIR comb filter vary block size frequency match test... ";
    }
    const float fMathPi = 3.14159265358979f;
    bool bTestPassedFlag = true;
    float inputTestSequence[1024];
    float** outputTestSequence;
    int iCurBlockHead = 0;
    float** ppfInputTestSignal;
    float** ppfOutputTestSignal;
    CCombFilterIf* phCombFilter = 0;

    outputTestSequence = new float* [2];
    for (int i = 0; i < 2; i++)
        outputTestSequence[i] = new float[1024];

    CCombFilterIf::create(phCombFilter);
    phCombFilter->init(eFilterType, static_cast<float>(iCycleInSamples) / static_cast<float>(iSampleRateInHz), static_cast<float>(iSampleRateInHz), 2);
    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        phCombFilter->setParam(CCombFilterIf::kParamGain, -1);
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        phCombFilter->setParam(CCombFilterIf::kParamGain, 1);
    }
    for (int i = 0; i < 1024; i++)
    {
        inputTestSequence[i] = sin(2 * fMathPi / static_cast<float>(iCycleInSamples) * static_cast<float>(i));
    }

    combFilterVaryTestBlock(phCombFilter, ppfInputTestSignal, ppfOutputTestSignal, inputTestSequence, outputTestSequence, iCurBlockHead, 32);
    combFilterVaryTestBlock(phCombFilter, ppfInputTestSignal, ppfOutputTestSignal, inputTestSequence, outputTestSequence, iCurBlockHead, 32);
    combFilterVaryTestBlock(phCombFilter, ppfInputTestSignal, ppfOutputTestSignal, inputTestSequence, outputTestSequence, iCurBlockHead, 64);
    combFilterVaryTestBlock(phCombFilter, ppfInputTestSignal, ppfOutputTestSignal, inputTestSequence, outputTestSequence, iCurBlockHead, 128);
    combFilterVaryTestBlock(phCombFilter, ppfInputTestSignal, ppfOutputTestSignal, inputTestSequence, outputTestSequence, iCurBlockHead, 768);

    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = iCycleInSamples; j < 1024; j++)
            {
                if (abs(outputTestSequence[i][j]) > 1e-4) bTestPassedFlag = false;
            }
        }
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        for (int i = 0; i < 2; i++)
        {
            for (int j = iCycleInSamples; j < 1024; j++)
            {
                if (inputTestSequence[j] != 0.f)
                    if (abs(outputTestSequence[i][j] / inputTestSequence[j]) < 1) bTestPassedFlag = false;
            }
        }
    }
    CCombFilterIf::destroy(phCombFilter);
    for (int i = 0; i < 2; i++)
        delete[] outputTestSequence[i];
    delete[] outputTestSequence;
    if (bTestPassedFlag)
    {
        cout << "passed!" << endl;
    }
    else
    {
        cout << "not passed." << endl;
    }
}

void combFilterVaryTestBlock(CCombFilterIf* phCombFilter, float** &ppfInputTestSignal, float** &ppfOutputTestSignal, const float* inputTestSequence, float** outputTestSequence, int& iCurBlockHead, int iBlockLength)
{
    ppfInputTestSignal = new float* [2];
    for (int i = 0; i < 2; i++)
        ppfInputTestSignal[i] = new float[iBlockLength];
    ppfOutputTestSignal = new float* [2];
    for (int i = 0; i < 2; i++)
        ppfOutputTestSignal[i] = new float[iBlockLength];

    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < iBlockLength; j++)
        {
            ppfInputTestSignal[i][j] = inputTestSequence[j + iCurBlockHead];
        }
    }
    phCombFilter->process(ppfInputTestSignal, ppfOutputTestSignal, iBlockLength);
    for (int i = 0; i < 2; i++)
    {
        for (int j = 0; j < iBlockLength; j++)
        {
            outputTestSequence[i][j + iCurBlockHead] = ppfOutputTestSignal[i][j];
        }
    }

    for (int i = 0; i < 2; i++)
        delete[] ppfInputTestSignal[i];
    delete[] ppfInputTestSignal;
    for (int i = 0; i < 2; i++)
        delete[] ppfOutputTestSignal[i];
    delete[] ppfOutputTestSignal;
    iCurBlockHead += iBlockLength;
}

void testZeroInput(CCombFilterIf::CombFilterType_t eFilterType, int iDelayInSample, int iNumChannels, int iTestLength, int iSampleRateInHz)
{
    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        cout << "Running FIR comb filter zero input test... ";
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        cout << "Running IIR comb filter zero input test... ";
    }
    bool bTestPassedFlag = true;
    float** ppfInputTestSignal;
    float** ppfOutputTestSignal;
    CCombFilterIf* phCombFilter = 0;


    CCombFilterIf::create(phCombFilter);
    ppfInputTestSignal = new float* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
        ppfInputTestSignal[i] = new float[iTestLength];
    ppfOutputTestSignal = new float* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
        ppfOutputTestSignal[i] = new float[iTestLength];
    phCombFilter->init(eFilterType, static_cast<float>(iDelayInSample) / static_cast<float>(iSampleRateInHz), static_cast<float>(iSampleRateInHz), iNumChannels);
    for (int i = 0; i < iNumChannels; i++)
    {
        for (int j = 0; j < iTestLength; j++)
        {
            ppfInputTestSignal[i][j] = 0;
        }
    }

    phCombFilter->process(ppfInputTestSignal, ppfOutputTestSignal, iTestLength);



    for (int i = 0; i < iNumChannels; i++)
    {
        for (int j =0; j < iTestLength; j++)
        {
            if (abs(ppfOutputTestSignal[i][j]) > 1e-4) bTestPassedFlag = false;
        }
    }

    CCombFilterIf::destroy(phCombFilter);
    for (int i = 0; i < iNumChannels; i++)
        delete[] ppfInputTestSignal[i];
    delete[] ppfInputTestSignal;
    for (int i = 0; i < iNumChannels; i++)
        delete[] ppfOutputTestSignal[i];
    delete[] ppfOutputTestSignal;
    if (bTestPassedFlag)
    {
        cout << "passed!" << endl;
    }
    else
    {
        cout << "not passed." << endl;
    }
}

void testDCInput(CCombFilterIf::CombFilterType_t eFilterType, int iDelayInSample, int iNumChannels, int iTestLength, int iSampleRateInHz)
{
    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        cout << "Running FIR comb filter DC removal test... ";
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        cout << "Running IIR comb filter DC estimator test... ";
    }
    bool bTestPassedFlag = true;
    float** ppfInputTestSignal;
    float** ppfOutputTestSignal;
    CCombFilterIf* phCombFilter = 0;


    CCombFilterIf::create(phCombFilter);
    ppfInputTestSignal = new float* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
        ppfInputTestSignal[i] = new float[iTestLength];
    ppfOutputTestSignal = new float* [iNumChannels];
    for (int i = 0; i < iNumChannels; i++)
        ppfOutputTestSignal[i] = new float[iTestLength];
    phCombFilter->init(eFilterType, static_cast<float>(iDelayInSample) / static_cast<float>(iSampleRateInHz), static_cast<float>(iSampleRateInHz), iNumChannels);
    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        phCombFilter->setParam(CCombFilterIf::kParamGain, -1.0f);
    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        phCombFilter->setParam(CCombFilterIf::kParamGain, .95f);
    }
    for (int i = 0; i < iNumChannels; i++)
    {
        for (int j = 0; j < iTestLength; j++)
        {
            ppfInputTestSignal[i][j] = 1;
        }
    }

    phCombFilter->process(ppfInputTestSignal, ppfOutputTestSignal, iTestLength);


    if (eFilterType == CCombFilterIf::kCombFIR)
    {
        for (int i = 0; i < iNumChannels; i++)
        {
            for (int j = 100; j < iTestLength; j++)
            {
                if (abs(ppfOutputTestSignal[i][j]) > 1e-4) bTestPassedFlag = false;
            }
        }

    }
    else if (eFilterType == CCombFilterIf::kCombIIR)
    {
        for (int i = 0; i < iNumChannels; i++)
        {
            for (int j = 100; j < iTestLength; j++)
            {
                if (abs(ppfOutputTestSignal[i][j]) < 0.9) bTestPassedFlag = false;
            }
        }

    }

    CCombFilterIf::destroy(phCombFilter);
    for (int i = 0; i < iNumChannels; i++)
        delete[] ppfInputTestSignal[i];
    delete[] ppfInputTestSignal;
    for (int i = 0; i < iNumChannels; i++)
        delete[] ppfOutputTestSignal[i];
    delete[] ppfOutputTestSignal;
    if (bTestPassedFlag)
    {
        cout << "passed!" << endl;
    }
    else
    {
        cout << "not passed." << endl;
    }
}