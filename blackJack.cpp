#include <iostream>
#include <mpi.h>

#define MCW MPI_COMM_WORLD

int simulateBlackJack(int simCode)
{
    int hit = simCode % 2;
    int dealerCard = ((simCode % 20) / 2) + 2; //11's are aces
    int yourTotal = simCode / 20;
    int numAces = 0;
    if (yourTotal < 10)
    {
        yourTotal = 17 - yourTotal;
    }
    else
    {
        yourTotal = 18 - yourTotal;
        numAces = 1;
    }
    return simCode;
}

int main(int argc, char** argv) 
{
    const int simSize = 10000;
    const int numSim = 340;
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
        for (int i = 0; i < simsDone; i++)
        {
            std::cout << i << ":" << resultArray[i] << std::endl;
        }


    }
    else {   // Slave Nodes
        int simRes;
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