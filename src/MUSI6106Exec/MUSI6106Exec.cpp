
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Vibrato.h"

#include "Fft.h"

#include "FastConv.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{

    std::string             sInputFilePath,                 //!< file paths
        sOutputFilePath, sIrPath;

    static const int            kBlockSize = 1024;
    static const int            iFastConvBlockLength = 8192;
    long long                   iNumFrames = kBlockSize;
    int                         iNumChannels;

    clock_t                     time = 0;

    float** ppfInputAudio = 0;
    float** ppfOutputAudio = 0;

    float** ppfImpulse = 0;

    float** ppfFlushBuffer = 0;

    long long iImpulseLength = 0;

    int iFlushBufferLength = 0;

    CAudioFileIf* phImpluseFile = 0;
    CAudioFileIf* phAudioFile = 0;
    CAudioFileIf* phAudioOutputFile = 0;

    CAudioFileIf::FileSpec_t    stFileSpec, stIrSpec, stOutputFileSpec;

    CFastConv::ConvCompMode_t eCompMode = CFastConv::kTimeDomain;

    CFastConv** pphFastConv = 0;

    bool bUseFastConv = false;

    showClInfo();


    // command line args
    if (argc < 5)
    {
        cout << "Not enough input arguments!\n";
        cout << "arguments: <PATH_TO_AUDIO> <PATH_TO_IR> <PATH_TO_OUTPUT> <USING_FAST_CONV(1 is true)>";
    }
    sInputFilePath = argv[1];
    sIrPath = argv[2];
    sOutputFilePath = argv[3];

    bUseFastConv = static_cast<bool>(std::stoi(argv[4]));

    if (bUseFastConv)
    {
        eCompMode = CFastConv::kFreqDomain;
    }

    ///////////////////////////////////////////////////////////////////////////
    CAudioFileIf::create(phImpluseFile);
    CAudioFileIf::create(phAudioFile);
    CAudioFileIf::create(phAudioOutputFile);

    phImpluseFile->openFile(sIrPath, CAudioFileIf::kFileRead);
    phImpluseFile->getFileSpec(stIrSpec);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    phAudioFile->getFileSpec(stFileSpec);
    phAudioFile->getFileSpec(stOutputFileSpec);

    if (!phAudioFile->isOpen())
    {
        cout << "Input file open error!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        CAudioFileIf::destroy(phImpluseFile);
        return -1;
    }


    phImpluseFile->getLength(iImpulseLength);

    ppfImpulse = new float* [stIrSpec.iNumChannels];

    for (int i = 0; i < stIrSpec.iNumChannels; i++)
    {
        ppfImpulse[i] = new float[iImpulseLength];
    }

    if (bUseFastConv)
    {
        iFlushBufferLength = iFastConvBlockLength + iImpulseLength - 1;
    }
    else
    {
        iFlushBufferLength = iImpulseLength - 1;
    }

    if (abs(stIrSpec.fSampleRateInHz - stFileSpec.fSampleRateInHz) > 1)
    {
        cout << "Impulse and audio sample rate don't match!\n";
        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        CAudioFileIf::destroy(phImpluseFile);
        return -1;
    }

    if (!phImpluseFile->isOpen())
    {
        cout << "Impulse file open error!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        CAudioFileIf::destroy(phImpluseFile);
        return -1;
    }

    phImpluseFile->readData(ppfImpulse, iImpulseLength);

    if (stIrSpec.iNumChannels == stFileSpec.iNumChannels)
    {
        pphFastConv = new CFastConv* [stOutputFileSpec.iNumChannels];
        for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
        {
            pphFastConv[i] = new CFastConv;
            pphFastConv[i]->init(ppfImpulse[i], static_cast<int>(iImpulseLength), iFastConvBlockLength, eCompMode);
        }
    }
    else if (stFileSpec.iNumChannels == 1)
    {
        stOutputFileSpec.iNumChannels = stIrSpec.iNumChannels;
        pphFastConv = new CFastConv * [stOutputFileSpec.iNumChannels];
        for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
        {
            pphFastConv[i] = new CFastConv;
            pphFastConv[i]->init(ppfImpulse[i], static_cast<int>(iImpulseLength), iFastConvBlockLength, eCompMode);
        }
    }
    else if (stIrSpec.iNumChannels == 1)
    {
        pphFastConv = new CFastConv * [stOutputFileSpec.iNumChannels];
        for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
        {
            pphFastConv[i] = new CFastConv;
            pphFastConv[i]->init(ppfImpulse[0], static_cast<int>(iImpulseLength), iFastConvBlockLength, eCompMode);
        }
    }
    else
    {
        cout << "Impulse and input audio channel don't match and neither of which is mono.\n";
        return -1;
    }



    CAudioFileIf::destroy(phImpluseFile);

    for (int i = 0; i < stIrSpec.iNumChannels; i++)
    {
        delete[] ppfImpulse[i];
    }
    delete[] ppfImpulse;

    phAudioOutputFile->openFile(sOutputFilePath, CAudioFileIf::kFileWrite, &stOutputFileSpec);

    
    if (!phAudioOutputFile->isOpen())
    {
        cout << "Output file cannot be initialized!";

        CAudioFileIf::destroy(phAudioFile);
        CAudioFileIf::destroy(phAudioOutputFile);
        return -1;
    }
    
    ////////////////////////////////////////////////////////////////////////////

    // allocate memory
    ppfInputAudio = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfInputAudio[i] = new float[kBlockSize];

    ppfOutputAudio = new float* [stOutputFileSpec.iNumChannels];
    ppfFlushBuffer = new float* [stOutputFileSpec.iNumChannels];
    for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
    {
        ppfOutputAudio[i] = new float[kBlockSize];
        ppfFlushBuffer[i] = new float[iFlushBufferLength];
    }
        



    time = clock();
    // processing
    int iBlockCount = 0;
    while (!phAudioFile->isEof())
    {
        phAudioFile->readData(ppfInputAudio, iNumFrames);

        for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
        {
            if (stFileSpec.iNumChannels == 1)
            {
                pphFastConv[i]->process(ppfOutputAudio[i],ppfInputAudio[0],iNumFrames);
            }
            else
            {
                pphFastConv[i]->process(ppfOutputAudio[i], ppfInputAudio[i], iNumFrames);
            }
        }
        if (bUseFastConv && iBlockCount < iFastConvBlockLength / kBlockSize)
        {
            iBlockCount++;
            continue;
        }
        phAudioOutputFile->writeData(ppfOutputAudio, iNumFrames);
    }
    

    for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
    {
        pphFastConv[i]->flushBuffer(ppfFlushBuffer[i]);
    }

    phAudioOutputFile->writeData(ppfFlushBuffer, iFlushBufferLength);


    phAudioFile->getFileSpec(stFileSpec);

    cout << "\nProcessing of convolution done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioOutputFile);

    
    for (int i = 0; i < stOutputFileSpec.iNumChannels; i++)
    {
        delete pphFastConv[i];
        delete[] ppfFlushBuffer[i];
        delete[] ppfOutputAudio[i];
    }


    for (int i = 0; i < stFileSpec.iNumChannels; i++)
    {
        delete[] ppfInputAudio[i];
    }
    delete[] pphFastConv;
    delete[] ppfFlushBuffer;
    delete[] ppfInputAudio;
    delete[] ppfOutputAudio;
    pphFastConv = 0;
    ppfInputAudio = 0;
    ppfOutputAudio = 0;

    // all done
    return 0;

}


void     showClInfo()
{
    cout << "MUSI6106 Assignment Executable" << endl;
    cout << "(c) 2014-2022 by Alexander Lerch" << endl;
    cout << endl;

    return;
}

