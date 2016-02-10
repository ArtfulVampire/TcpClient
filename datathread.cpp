#include "datathread.h"

using namespace std;
using namespace std::chrono;
using namespace enc;

DataThread::DataThread()
{
}

DataThread::~DataThread()
{
}

DataThread::DataThread(QTcpSocket * inSocket, bool fullData)
{
    str.setDevice(inSocket);
    str.setByteOrder(QDataStream::LittleEndian); // least significant bytes first
    this->fullDataFlag = fullData;
    cout << "thread: constructed" << endl;
}

void DataThread::run()
{
    if(str.device()->bytesAvailable() < 64) // 1 slice, const
    {
        cout << "thread: not enough data" << endl;
        this->usleep(100);
        run();
    }
    if(fullDataFlag)
    {
        // pre read
        int packSize;
        str >> packSize;

        DWORD packId;
        str >> packId;

        cout << "packSize = " << packSize << "\t"
             << "packId = " << packId << endl;

        qint32 sliceNumber;
        str >> sliceNumber;

        qint32 numOfChans;
        str >> numOfChans;

        qint32 numOfSlices; // not really 'long'
        str >> numOfSlices;

        static std::chrono::high_resolution_clock::time_point t1;
        static std::chrono::high_resolution_clock::time_point t2;


        cout << sliceNumber << '\t';
        cout << numOfChans << '\t';
        cout << numOfSlices << endl;

        if(numOfSlices == 1000)
        {
            t1 = std::chrono::high_resolution_clock::now();
        }
        else if(numOfSlices == 2000)
        {
            t2 = std::chrono::high_resolution_clock::now();
            cout << "time difference(msec) = " <<
                    duration_cast<milliseconds>(t2-t1).count() << endl;
            exit(0);
        }

//        else if(sliceNumber % 100 == 0)
//        {
//            cout << sliceNumber << endl;
//        }


//        std::vector<short> oneSlice(numOfChans);
        short tmp;
        for(int i = 0; i < numOfSlices; ++i)
        {
            for(int j = 0; j < numOfChans; ++j)
            {
//                str >> oneSlice[j];
                str >> tmp;
            }
//            eegData.push_back(oneSlice);
//            ++WholeNumOfSlices;
        }
        /// test
#if 0
        if(eegData.size() > 1000)
        {
//            std::ofstream ostr("/media/Files/Data/list.txt");
            std::ofstream ostr("D:\\MichaelAtanov\\Real-time\\TcpClient\\pew.txt");
            ostr << "NumOfSlices " << eegData.size();
            ostr << "NumOfChannels " << numOfChans << endl;
            for(auto slice : eegData)
            {
                for(auto val : slice)
                {
                    ostr << val << '\t';
                }
                ostr << '\n';
            }
            ostr.close();
            exit(0);
        }
#endif
    }
    run();
}
