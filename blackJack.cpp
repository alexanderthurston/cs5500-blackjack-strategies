#include <iostream>
#include <mpi.h>
#include <ctime>
#include <fstream>

#define MCW MPI_COMM_WORLD
const int numOfSims = 1000000;

int gameSim(int yourTotal, int numAces, int dealerCard, int hit)
{
    //win = 2, tie = 1, lose = 0
    int win = 1;
    int dealerAceCount = 0;
    if (hit == 0) 
    {
        int newCard = rand() % 13 + 1;
        if (newCard == 1)
        {
            numAces++;
        }
        else
        {
            if (newCard > 10)
            {
                newCard = 10;
            }
            yourTotal += newCard;
        }

    }
    for (int i = 0; i < numAces; i++)
    {
        yourTotal += 11;
    }
    while (yourTotal > 21 && numAces > 0)
    {
        yourTotal -= 10;
        numAces--;
    }
    if (dealerCard == 11)
    {
        dealerAceCount++;
    }
    if (yourTotal < 22)
    {
        while (dealerCard < 18 && dealerCard < yourTotal)
        {
            int newCard = rand() % 13 + 1;
            if (newCard == 1)
            {
                dealerAceCount++;
                dealerCard += 11;
            }
            else
            {
                if (newCard > 10)
                {
                    newCard = 10;
                }
                dealerCard += newCard;
            }
            if (dealerCard > 21 && dealerAceCount > 0)
            {
                dealerAceCount--;
                dealerCard -= 10;
            }
        }
    }
    if ((dealerCard > 21 && yourTotal > 21) || (dealerCard == yourTotal))
    {
        win = 1; //tie
    }
    else
    {
        if ((dealerCard > 21) || ((yourTotal > dealerCard) && yourTotal < 22))
        {
            win = 2; //win (double your money)
        }
        else
        {
            win = 0; // lose
        }
    }
    return win;
}

int simulateBlackJack(int simCode)
{
    int hit = simCode % 2;
    int dealerCard = ((simCode % 20) / 2) + 2; //11's are aces
    int yourTotal = simCode / 20;
    int numAces = 0;
    if (yourTotal < 7)
    {
        yourTotal = 17 - yourTotal;
    }
    else
    {
        yourTotal = 15 - yourTotal;
        numAces = 1;
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
    const int numSim = 280;
    const int killCode = -1;
    int rank, size, recieved, flag, rankRecieved;
    int simArray[3] = { 0 };
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MCW, &rank);
    MPI_Comm_size(MCW, &size);
    std::srand(std::time(nullptr) + rank);
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
        std::ofstream fout;
        fout.open("SimOut.txt");
        for (int i = 0; i < simsDone; i+= 2)
        {
            int hit = i % 2;
            int dealerCard = ((i % 20) / 2) + 2; //11's are aces
            int yourTotal = i / 20;
            int numAces = 0;
            if (yourTotal < 7)
            {
                yourTotal = 17 - yourTotal;
            }
            else
            {
                yourTotal = 15 - yourTotal;
                numAces = 1;
            }
            std::cout << "Simulated with dealer showing " << dealerCard << ". I have a (non-ace) total of " << yourTotal << " and " << numAces << " aces." ;
            std::cout << " With a win rate of " << (float)resultArray[i] / (float)(numOfSims) << " when I hit and " << (float)resultArray[i+1] / (float)(numOfSims) << " when I dont" << std::endl;
            fout << dealerCard << '\t' << yourTotal << '\t' << numAces << '\t' << (float)resultArray[i] / (float)(numOfSims) << '\t' << (float)resultArray[i + 1] / (float)(numOfSims) << std::endl;
        }
        fout.close();
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