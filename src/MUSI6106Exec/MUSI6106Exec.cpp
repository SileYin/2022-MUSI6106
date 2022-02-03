
#include <iostream>
#include <ctime>

#include "MUSI6106Config.h"

#include "RingBuffer.h"

using std::cout;
using std::endl;

// local function declarations
void    showClInfo ();

/////////////////////////////////////////////////////////////////////////////////
// main function
int main(int argc, char* argv[])
{
    CRingBuffer<float>* pCRingBuff = 0; 
    
    static const int kBlockSize = 17;

    showClInfo();

    pCRingBuff = new CRingBuffer<float>(kBlockSize);

    std::cout << "Trivial test, buffer length:  " << pCRingBuff->getLength() << std::endl;


    std::cout << "Test 1: Wrap around" << std::endl;

    for (int i = 0; i < 5; i++)
    {
        pCRingBuff->putPostInc(1.F*i);
    }

    for (int i = 5; i < 30; i++)
    {
        std::cout << i << " ";
        std::cout << pCRingBuff->getNumValuesInBuffer() << " "; // should be five
        std::cout << pCRingBuff->getPostInc() << std::endl; // should be i-5
        pCRingBuff->putPostInc(1.F*i);
    }

    std::cout << "Test 2: Overflowing" << std::endl;

    pCRingBuff->reset();

    for (int i = 0; i < 25; i++)
    {
        pCRingBuff->putPostInc(1.F * i);
    }

    for (int i = 25; i < 30; i++)
    {
        std::cout << i << " ";
        std::cout << pCRingBuff->getNumValuesInBuffer() << " "; // should be five
        std::cout << pCRingBuff->getPostInc() << std::endl; // should be i-5
        pCRingBuff->putPostInc(1.F * i);
    }

    std::cout << "Test 3: Read before write" << std::endl;

    pCRingBuff->reset();

    for (int i = 0; i < 5; i++)
    {
        std::cout << pCRingBuff->getPostInc() << std::endl;
    }

    for (int i = 1; i < 25; i++)
    {
        std::cout << i << " ";
        std::cout << pCRingBuff->getNumValuesInBuffer() << " "; // should be five
        std::cout << pCRingBuff->getPostInc() << std::endl; // should be i-5
        pCRingBuff->putPostInc(1.F * i);
    }

    std::cout << "Test 4: Stuck the read pointer" << std::endl;

    pCRingBuff->reset();

    for (int i = 0; i < 5; i++)
    {
        pCRingBuff->putPostInc(1.F * i);
    }

    for (int i = 5; i < 30; i++)
    {
        pCRingBuff->setReadIdx(0);
        std::cout << i << " ";
        std::cout << pCRingBuff->getNumValuesInBuffer() << " "; // should be five
        std::cout << pCRingBuff->getPostInc() << std::endl; // should be i-5
        pCRingBuff->putPostInc(1.F * i);
    }

    std::cout << "Test 5: Stuck the write pointer" << std::endl;

    pCRingBuff->reset();

    for (int i = 0; i < 5; i++)
    {
        pCRingBuff->setWriteIdx(0);
        pCRingBuff->putPostInc(1.F * i);
    }

    for (int i = 5; i < 30; i++)
    {
        pCRingBuff->setWriteIdx(0);
        std::cout << i << " ";
        std::cout << pCRingBuff->getNumValuesInBuffer() << " "; // should be five
        std::cout << pCRingBuff->getPostInc() << std::endl; // should be i-5
        pCRingBuff->putPostInc(1.F * i);
    }

    delete pCRingBuff;
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
