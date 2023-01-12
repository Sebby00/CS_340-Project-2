#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>

int InpArray[10000],indexInp = 0, dividedArr[10][1000],slotOne = 0,workerIndex = 0, sumResultsArr[10],avgResultsArr[10],expResultsArr[10];
pthread_mutex_t lockInp;
pthread_t ptid[10];
pthread_t wtid[3];

/*A function that gets the length 
of a char array by iterating through 
the entire char and returning that value*/
int getLength(char* string1)
{
	int i = 0;
	while(string1[i] != '\0')
	{
		i++;
	}
	return i;
}

/*A function that fills the inpArray[10000]
and puts it into a global variable to be
accessed by all the threads*/
void fillInp(char *argv)
{
    FILE    *textfile;
    char    line[40];
    int i = 0;

    
    //tries to open a file
    textfile = fopen(argv, "r");
    if(textfile == NULL)
    {
        printf("error opening");
    }   

    //reads in the values into an array
    while(fgets(line, 40 , textfile)){
        InpArray[i] = atoi(line);
        i++;
    }
    fclose(textfile);
}


/*This is the sum thread instructions that calculates
the arithmetic average of 1000 numbers */
void* sumWorkerPthreads(void* arg)
{
    int myposition = *(int*)arg, sum = 0;

    for(int i = 0; i < 1000;i++)
    {
        sum += dividedArr[myposition][i];
    }   
    
    //stores results into a global array
    sumResultsArr[myposition] = sum;
    return NULL;   
}


/*This is the average thread instruction
that computes the average of its index 
of 1000 integers*/
void* avgWorkerPthreads(void* arg)
{
    int myposition = *(int*)arg, holder = 0, avg;

    for(int i = 0; i < 1000;i++)
    {
        holder += dividedArr[myposition][i];
    }   
    avg  = holder/1000;
    
    //stores results in global array
    avgResultsArr[myposition] = avg;
    return NULL; 
}


/*This is the root of sums thread instruction
which computes the root of sum for its 
index of 1000 integers*/
void* prodWorkerPthreads(void* arg)
{
    int myposition = *(int*)arg, prod = 0;
    
    for(int i = 0; i < 1000;i++)
    {
        prod += (dividedArr[myposition][i]) * (dividedArr[myposition][i]);
    }   
    prod = sqrt(prod);

    //stores results into global array
    expResultsArr[myposition] = prod;
    return NULL;     
}


/*This is the intrstructions for the 
children thread of the main program
it divided the array into 10 different
1000 integer chunks and creates its
worker threads to perform computations*/
void* childrenPthreads(void* arg)
{
    int error;
    int giveWorkerIndex = *(int*)arg;

    //critical section for dividing the array
    pthread_mutex_lock(&lockInp);
    for(int j = 0; j < 1000; j++)
    {
        dividedArr[slotOne][j] = InpArray[indexInp];
        indexInp++;
    }
    slotOne++;
    pthread_mutex_unlock(&lockInp);

    /*Creating the 3 worker threads  while also passing the index to work on*/
    error = pthread_create(&(wtid[0]), NULL, &sumWorkerPthreads, (void *)&giveWorkerIndex);
       if (error != 0)
       {
            printf("\nThread can't be created : [%s]", strerror(error));
       }

    error = pthread_create(&(wtid[1]), NULL, &avgWorkerPthreads, (void *)&giveWorkerIndex);
       if (error != 0)
       {
            printf("\nThread can't be created : [%s]", strerror(error));
       }
        
    error = pthread_create(&(wtid[2]), NULL, &prodWorkerPthreads, (void *)&giveWorkerIndex);
       if (error != 0)
       {
            printf("\nThread can't be created : [%s]", strerror(error));
       }   

    /*Forces the child thread to
     wait for the workers to end*/
    for(int i = 0; i < 3; i++)
    {
        pthread_join(wtid[i], NULL);
    }
    return NULL; 
}



int main(int argc,char *argv[])
{
    //call of function to fill Inp
    fillInp(argv[1]);

    int error,i,f1;


        //creating the 10 children threads
        for (i = 0; i < 10; i++)
        {
            error = pthread_create(&(ptid[i]), NULL, &childrenPthreads, (void *)&i);
            if (error != 0)
            {
                printf("\nThread can't be created : [%s]", strerror(error));
            }
            pthread_join(ptid[i], NULL);
        }


        /*Declared after the join due to the fact if done before
        the join these values will be 0*/
        int minAvg = avgResultsArr[0] ,minSum = sumResultsArr[0],minExp = expResultsArr[0],
        minSumThread, minAvgThread, minExpThread;


        /*Finding the minimum of each
        grandchild thread results*/
        for(int i = 0; i < 10; i++)
        {
            if(sumResultsArr[i] < minSum)
            {
                minSum = sumResultsArr[i];
                minSumThread = i;
            }
            
            if(avgResultsArr[i] < minAvg)
            {
                minAvg = avgResultsArr[i];
                minAvgThread = i;
            }

            if(expResultsArr[i] < minExp)
            {
                minExp = expResultsArr[i];
                minExpThread = i;
            }
        }
    
        //creating the output textfile
        if ((f1 = creat("workerOutput.txt", 0644)) == -1)
        {
            printf("Can't create %s \n", argv[2]);
            return 0;
        }
   
        /*Writing the results of the threads to 
        a output textfile through concatentation
        and integer to char array conversion*/
        for(int i = 0; i < 10; i++)
        {
            /*Pre-written messages
            to be used when writing
            to the text file*/
            char thread [50] = "\nTHREAD_NUMBER - ";
            char average [50] = "\nAVERAGE - ";
            char sum [50] ="\nSum - ";
            char exponential [50] = "\nROOT_SUM_SQUARES - ";


            /*These variables
            will be used to 
            store integers*/
            char iStr[20];           
            char avgStr[20];
            char sumStr[20];
            char expStr[20];
            char spaceUnder[5] = "\n";


            /*Converting the integers
            into a char and storing 
            them into an array*/
            sprintf(iStr, "%d", i+1);
            sprintf(avgStr, "%d", avgResultsArr[i]);
            sprintf(sumStr, "%d", sumResultsArr[i]);
            sprintf(expStr, "%d", expResultsArr[i]);
            

            /*Concatenating the
            newly converted integers
            to a char array with
            a pre-written message*/
            strcat(thread,iStr);
            strcat(average,avgStr);
            strcat(sum,expStr);
            strcat(exponential,expStr);
            

            /*writing the concatenated results
            to the text file*/
            write(f1, thread, getLength(thread)); 
            write(f1, average, getLength(thread));
            write(f1, sum, getLength(sum));  
            write(f1, exponential, getLength(exponential)); 
            write(f1,spaceUnder,getLength(spaceUnder));
        }
           
           /*Variables to help write the minimum 
           of all returned results for the 3 operations*/
            char minAverage [50] = "\nMIN_OF_ARITH_AVG- ", minSummation [50] = "\nMIN_OF_SUM - ", minExponential [50] = "\nMIN_ROOT_SUM_SQUARES - ",
            numAvg[20],numSum[20],numExp[20];

            
            /*converting integers to a 
            char array*/
            sprintf(numAvg, "%d", minAvg);
            sprintf(numSum, "%d", minSum);
            sprintf(numExp, "%d", minExp);
            

            /*Concatenating the newly
            converted integers to char
            to the pre-written messages*/
            strcat(minAverage,numAvg);
            strcat(minSummation,numSum);
            strcat(minExponential,numExp);


            /*Writing the cancatenated results to the textfile*/
            write(f1, minAverage, getLength(minAverage)); 
            write(f1, minSummation, getLength(minSummation));  
            write(f1,minExponential,getLength(minExponential));


            /*Closing the text
            file*/
            if (close(f1) < 0)
            {
                perror("c1");
                exit(1);
            }

	return 0;	
}

