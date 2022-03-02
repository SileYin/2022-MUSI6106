
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"
#include "AudioFileIf.h"
#include "RingBuffer.h"
#include "Vibrato.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{

    std::string sInputFilePath,                 //!< file paths
                sOutputFilePath,
                sOutputAudPath;

    static const int kBlockSize = 1024;

    clock_t time = 0;

    float **ppfAudioData = 0;
    float **ppfAudioDataOut = 0;
    CAudioFileIf *phAudioFile = 0;
    CAudioFileIf *phAudioFileOut = 0;
    std::fstream hTestOutputFile;
    CAudioFileIf::FileSpec_t stFileSpec;
    CVibrato *pCVibrato=0;

    showClInfo();
    //////////////////////////////////////////////////////////////////////////////
    // parse command line arguments
    if (argc < 2)
    {
        cout<<"Input not enough! Please specify input and output path"<<endl;
    }
    else
    {
        sInputFilePath = argv[1];
        sOutputAudPath =  argv[2];
    }
    
    // open the input wave file
    CAudioFileIf::create(phAudioFile);
    phAudioFile->openFile(sInputFilePath, CAudioFileIf::kFileRead);
    if (!phAudioFile->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFile);
        return -1;
    }
    phAudioFile->getFileSpec(stFileSpec);
    
    //create output file
    CAudioFileIf::create(phAudioFileOut);
    phAudioFileOut->openFile(sOutputAudPath, CAudioFileIf::kFileWrite, &stFileSpec);
    if (!phAudioFileOut->isOpen())
    {
        cout << "Wave file open error!";
        CAudioFileIf::destroy(phAudioFileOut);
        return -1;
    }
    
    // allocate memory
    ppfAudioData = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioData[i] = new float[kBlockSize];
    
    ppfAudioDataOut = new float*[stFileSpec.iNumChannels];
    for (int i = 0; i < stFileSpec.iNumChannels; i++)
        ppfAudioDataOut[i] = new float[kBlockSize];
    
    
    CVibrato::create(pCVibrato);
    pCVibrato->init(0.01, 44100);
    pCVibrato->setParam(CVibrato::kParamVibratoFrequency, 20);
    pCVibrato->setParam(CVibrato::kParamVibratoRange, 0.002);
    


    while (!phAudioFile->isEof())
    {
        // set block length variable
        long long iNumFrames = kBlockSize;

        // read data (iNumOfFrames might be updated!)
        phAudioFile->readData(ppfAudioData, iNumFrames);
        cout << "\r" << "reading audio"<<endl;
        
        // vibrato
        for (int i = 0; i < stFileSpec.iNumChannels; i++)
            pCVibrato->process(ppfAudioData[i], ppfAudioDataOut[i], iNumFrames);
        
        cout << "\r" << "Write audio"<<endl;
        phAudioFileOut->writeData(ppfAudioDataOut, iNumFrames);
        
    }
    
    

    cout << "\nreading/writing done in: \t" << (clock() - time) * 1.F / CLOCKS_PER_SEC << " seconds." << endl;

    // clean-up (close files and free memory)
    phAudioFileOut->closeFile();
    CAudioFileIf::destroy(phAudioFile);
    CAudioFileIf::destroy(phAudioFileOut);

    for (int i = 0; i < stFileSpec.iNumChannels; i++){
        delete[] ppfAudioData[i];
        delete[] ppfAudioDataOut[i];
    }
    delete[] ppfAudioData;
    delete[] ppfAudioDataOut;
    ppfAudioData = 0;
    ppfAudioDataOut = 0;
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
