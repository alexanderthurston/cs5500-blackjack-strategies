#include <iostream>
#include <mpi.h>

#define MCW MPI_COMM_WORLD
const int numOfSims = 10000;

int gameSim(int yourTotal, int numAces, int dealerCard, int hit)
{
    //win = 2, tie = 1, lose = 0
    int win = 1;
    return win;
}

int simulateBlackJack(int simCode)
{
    int hit = simCode % 2;
    int dealerCard = ((simCode % 20) / 2) + 2; //11's are aces
    int yourTotal = simCode / 20;
    int numAces = 0;
    if (yourTotal < 17)
    {
        yourTotal = 20 - yourTotal;
    }
    else
    {
        yourTotal = 26 - yourTotal;
        numAces = 1;
    }
    std::cout << "I will simulate with dealer showing " << dealerCard << ". I have a (non-ace) total of " << yourTotal << " and " << numAces << " aces, and I will ";
    if (hit)
    {
        std::cout << "not hit." << std::endl;
    }
    else
    {
        std::cout << "hit." << std::endl;
    }
    int winScore = 0;
    for (int i = 0; i < numOfSims; i++)
    {
        winScore += gameSim(yourTotal, numAces, dealerCard, hit);
    }
    return winScore;
}

int main(int argc, char** argv) 
{
    const int numSim = 500;
    const int killCode = -1;
    int rank, size, recieved, flag, rankRecieved;
    int simArray[3] = { 0 };
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    if (!rank) 
    { // Master Node 
        MPI_Status status;
        MPI_Request request;
        int simsDone = 0;
        int sendCount = 0;
        int resultArray[numSim] = { 0 };
        for (int i = 1; i < size; i++)
        {
            MPI_Send(&sendCount, 1, MPI_INT, i, 0, MCW);
            sendCount++;
        }
        while (simsDone < numSim)
        {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MCW, &flag, &status);
            if (flag)
            {
                MPI_Irecv(&simArray, 3, MPI_INT, MPI_ANY_SOURCE, 0, MCW, &request);
                resultArray[simArray[0]] = simArray[1];
                simsDone++;
                rankRecieved = simArray[2]; //set the rank based off what was recieved
                if (sendCount < numSim)
                {
                    MPI_Send(&sendCount, 1, MPI_INT, rankRecieved, 0, MCW);
                    sendCount++;
                }
                else
                {
                    MPI_Send(&killCode, 1, MPI_INT, rankRecieved, 0, MCW);
                }
            }
        }
    // Print Results
        //for (int i = 0; i < simsDone; i++)
        //{
        //    std::cout << i << ":" << resultArray[i] << std::endl;
        //}


    }
    else {   // Slave Nodes
        while (1) {
            MPI_Recv(&recieved, 1, MPI_INT, MPI_ANY_SOURCE, 0, MCW, MPI_STATUS_IGNORE);
            if (recieved == killCode)break;
            simArray[0] = recieved;
            simArray[1] = simulateBlackJack(recieved);
            simArray[2] = rank;
            MPI_Send(&simArray, 3, MPI_INT, 0, 0, MCW);
        }
    }
    MPI_Finalize();
    return 0;
}