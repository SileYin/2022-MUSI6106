
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "AudioFileIf.h"
#include "Fft.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    std::string             sInputFilePath,                 //!< file paths
        sOutputFilePath;

    static const int        kBlockSize = 1024;

    clock_t                 time = 0;

    float** ppfAudioData = 0;
    float** ppfFftMag = 0;

    CAudioFileIf* phAudioFile = 0;
    std::fstream            hOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;

    int iFftBlockLength, iFftHopLength;
    CFft::complex_t** pcSpectrogram = 0;

    CFft* phFftCalculator = 0;

    showClInfo();

    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 4)
    {
        cout << "Missing audio input path!";
        return -1;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputFilePath = sInputFilePath + ".txt";
        iFftBlockLength = std::stoi(argv[2]);
        iFftHopLength = std::stoi(argv[3]);
    }

    //////////////////////////////////////////////////////////////////////////////
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    CFft::createInstance(phFftCalculator);
    phFftCalculator->initInstance(iFftBlockLength);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);

    //////////////////////////////////////////////////////////////////////////////
    // open the output text file
    hOutputFile.open(sOutputFilePath.c_str(), std::ios::out);
    if (!hOutputFile.is_open())
    {
        cout << "Text file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }

    //////////////////////////////////////////////////////////////////////////////
    // allocate memory
    ppfAudioData = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[iFftBlockLength];

    ppfFftMag = new float* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfFftMag[i] = new float[static_cast<int>(iFftBlockLength / 2) + 1];

    if (ppfAudioData == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hOutputFile.close();
        return -1;
    }
    if (ppfAudioData[0] == 0)
    {
        CAudioFileIf::destroy(phAudioFile);
        hOutputFile.close();
        return -1;
    }

    pcSpectrogram = new CFft::complex_t* [stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        pcSpectrogram[i] = new CFft::complex_t [iFftBlockLength];

    time = clock();

    //////////////////////////////////////////////////////////////////////////////
    // get audio data and write it to the output text file (one column per channel)
    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = iFftBlockLength;
        long long iCurPosition;

        phAudioFile->getPosition(iCurPosition);

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);

        phAudioFile->setPosition(iCurPosition + static_cast<long long>(iFftHopLength));
        cout << "\r" << "reading and writing";

        for (int i = iNumFrames; i < iFftBlockLength; i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                ppfAudioData[c][i] = 0;
            }
        }

        for (int c = 0; c < stFileSpec.iNumChannels; c++)
        {
            phFftCalculator->doFft(pcSpectrogram[c], ppfAudioData[c]);
            phFftCalculator->getMagnitude(ppfFftMag[c], pcSpectrogram[c]);
        }


        // write
        for (int i = 0; i <= static_cast<int>(iFftBlockLength / 2); i++)
        {
            for (int c = 0; c < stFileSpec.iNumChannels; c++)
            {
                hOutputFile << ppfFftMag[c][i] << " ";
            }
        }
        hOutputFile << endl;
    }

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    //////////////////////////////////////////////////////////////////////////////
    // clean-up (close files and free memory)
    CAudioFileIf::destroy(phAudioFile);
    CFft::destroyInstance(phFftCalculator);
    hOutputFile.close();

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] pcSpectrogram[i];
    delete[] pcSpectrogram;
    pcSpectrogram = 0;
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfFftMag[i];
    delete[] ppfFftMag;
    ppfFftMag = 0;

    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        delete[] ppfAudioData[i];
    delete[] ppfAudioData;
    ppfAudioData = 0;

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

