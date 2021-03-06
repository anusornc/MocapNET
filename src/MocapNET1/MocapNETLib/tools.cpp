#include "tools.h"

#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>


unsigned long tickBaseMN = 0;


char directoryExists(const char * folder)
{
 struct stat sb;
 if (stat(folder, &sb) == 0 && S_ISDIR(sb.st_mode)) { return 1; } 
 return 0;
}


int executeCommandLineNum(const char *  command , char * what2GetBack , unsigned int what2GetBackMaxSize,unsigned int lineNumber)
{
    /* Open the command for reading. */
    FILE * fp = popen(command, "r");
    if (fp == 0 )
        {
            fprintf(stderr,"Failed to run command (%s) \n",command);
            return 0;
        }

    /* Read the output a line at a time - output it. */
    unsigned int i=0;
    while (fgets(what2GetBack, what2GetBackMaxSize , fp) != 0)
        {
            ++i;
            if (lineNumber==i)
                {
                    break;
                }
        }
    /* close */
    pclose(fp);
    return 1;
}

/**
 * @brief Simple way to get our CPU model name..
 * @retval 1=Success/0=Failure
 */
int getCPUName(char * str,unsigned int maxLength)
{
     if (!executeCommandLineNum("cat /proc/cpuinfo | grep \"model name\" | cut -d':' -f2 | tr -s ' '|  uniq",str,maxLength,0))
       {
         fprintf(stderr,"Could not get our CPU model name\n");
         return 0;
       }
     return 1;  
}

/**
 * @brief Simple way to get our GPU model name..
 * @retval 1=Success/0=Failure
 */
int getGPUName(char * str,unsigned int maxLength)
{ 
     if (!executeCommandLineNum("lspci | grep VGA  | cut -d':' -f3 | tr -s ' '|  uniq",str,maxLength,0))
       {
         fprintf(stderr,"Could not get our GPU model name\n");
         return 0;
       }
     return 1;  
}


unsigned long GetTickCountMicrosecondsMN()
{
    struct timespec ts;
    if ( clock_gettime(CLOCK_MONOTONIC,&ts) != 0)
        {
            return 0;
        }

    if (tickBaseMN==0)
        {
            tickBaseMN = ts.tv_sec*1000000 + ts.tv_nsec/1000;
            return 0;
        }

    return ( ts.tv_sec*1000000 + ts.tv_nsec/1000 ) - tickBaseMN;
}


unsigned long GetTickCountMillisecondsMN()
{
    return (unsigned long) GetTickCountMicrosecondsMN()/1000;
}


float convertStartEndTimeFromMicrosecondsToFPS(unsigned long startTime, unsigned long endTime)
{
    float timeInMilliseconds =  (float) (endTime-startTime)/1000;
    if (timeInMilliseconds ==0.0)
        {
            timeInMilliseconds=0.00001;    //Take care of division by null..
        }
    return (float) 1000/timeInMilliseconds;
}

char fileExists(const char * filename)
{
 FILE *fp = fopen(filename,"r");
 if( fp ) { /* exists */ fclose(fp); return 1; }
 return 0;
}

int getImageWidthHeight(const char * filename,unsigned int * width , unsigned int * height)
{
    if (!fileExists(filename))
    {
      fprintf(stderr,"File %s does not exist, unable to get its dimensions..\n",filename);
      return 0;  
    }
    
    unsigned int retrievedWidth=0;
    unsigned int retrievedHeight=0;

    char commandToExecute[1024]= {0};
    char result[1024]= {0};
    int  results=0;

    snprintf(commandToExecute,1024,"identify -format \"%%w\" %s",filename);
    if ( executeCommandLineNum(commandToExecute,result,1024,0) )
        {
            retrievedWidth = atoi(result);
            ++results;
        }

    snprintf(commandToExecute,1024,"identify -format \"%%h\" %s",filename);
    if ( executeCommandLineNum(commandToExecute,result,1024,0) )
        {
            retrievedHeight = atoi(result);
            ++results;
        }

    if (
        (retrievedWidth!=0) &&
        (retrievedHeight!=0)
       )
        {
            *width  = retrievedWidth;
            *height = retrievedHeight;
        }



    return (results==2);
}
