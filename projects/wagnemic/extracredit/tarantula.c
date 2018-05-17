#include "stdlib.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LINE_COUNT_LARGER_THAN_DOMINION 50000

#define ANSI_COLOR_RED1     "\x1b[38;5;196;48;5;16m"
#define ANSI_COLOR_RED2     "\x1b[38;5;202;48;5;16m"
#define ANSI_COLOR_RED3     "\x1b[38;5;208;48;5;16m"
#define ANSI_COLOR_RED4     "\x1b[38;5;214;48;5;16m"
#define ANSI_COLOR_RED5     "\x1b[38;5;220;48;5;16m"
#define ANSI_COLOR_YELLOW   "\x1b[33;5;226;48;5;16m"
#define ANSI_COLOR_GREEN5   "\x1b[38;5;190;48;5;16m"
#define ANSI_COLOR_GREEN4   "\x1b[38;5;154;48;5;16m"
#define ANSI_COLOR_GREEN3   "\x1b[38;5;118;48;5;16m"
#define ANSI_COLOR_GREEN2   "\x1b[38;5;82;48;5;16m"
#define ANSI_COLOR_GREEN1   "\x1b[38;5;46;48;5;16m"
#define ANSI_COLOR_GRAY     "\x1b[38;5;243;48;5;16m"
#define ANSI_COLOR_RESET    "\x1b[0m"

// this assumes a dominion.c.gcov exits in current directory
// this will populate wasLineRun array with 0s and 1s
// 0 indicates the line at the given index was not run during the test that made dominion.c.gcov
// 1 indicates the line at the given index was run during the test that made dominion.c.gcov
// returns the number of lines in dominion.c detected by gcov
int discoverExecutedLines(int wasLineRun[LINE_COUNT_LARGER_THAN_DOMINION])
{
    char runInfo[10]; // will contain number of times the line was run
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i = -5; // first line with info starts on line 6, so starting at this value means i will be 0 when it gets to first line with info
    int j;
    
    fp = fopen("dominion.c.gcov", "r");

    while ((read = getline(&line, &len, fp)) != -1) {
        // line is the char* for the current line
        
        // first 5 lines don't have info we want and anything after LINE_COUNT_LARGER_THAN_DOMINION is useuless too
        if (i >= 0 && i < LINE_COUNT_LARGER_THAN_DOMINION)
        {
            // store run info (first 9 chars of each line)
            for (j = 0; j < 9; ++j)
                runInfo[j] = line[j];
            runInfo[9] = 0;
            
            // line was run if we can get a nonzero integer out of runInfo using strtol
            if (strtol(runInfo, 0, 10) > 0)
                wasLineRun[i] = 1;
            else
                wasLineRun[i] = 0;
        }
        ++i;
    }

    fclose(fp);
    if (line)
        free(line);
    
    return i;
}

// prints each line of dominion code based on the hue of each line
// hue values determined by the values provided in huePerLine
void printDominionCodeBasedOnHue(double huePerLine[LINE_COUNT_LARGER_THAN_DOMINION])
{
    
    FILE * fp;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;
    int i = 0;
    
    fp = fopen("dominion.c", "r");

    while ((read = getline(&line, &len, fp)) != -1) {
        // line is the char* for the current line
        
        // only care about lines 0 thru LINE_COUNT_LARGER_THAN_DOMINION-1
        if (i >= 0 && i < LINE_COUNT_LARGER_THAN_DOMINION)
        {
            // print a line with a color based on given hue
            if (huePerLine[i] < 0.0)
                printf(ANSI_COLOR_GRAY "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 1/11.0)
                printf(ANSI_COLOR_RED1 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 2/11.0)
                printf(ANSI_COLOR_RED2 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 3/11.0)
                printf(ANSI_COLOR_RED3 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 4/11.0)
                printf(ANSI_COLOR_RED4 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 5/11.0)
                printf(ANSI_COLOR_RED5 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 6/11.0)
                printf(ANSI_COLOR_YELLOW "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 7/11.0)
                printf(ANSI_COLOR_GREEN5 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 8/11.0)
                printf(ANSI_COLOR_GREEN4 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 9/11.0)
                printf(ANSI_COLOR_GREEN3 "%s" ANSI_COLOR_RESET, line);
            else if (huePerLine[i] <= 10/11.0)
                printf(ANSI_COLOR_GREEN2 "%s" ANSI_COLOR_RESET, line);
            else 
                printf(ANSI_COLOR_GREEN1 "%s" ANSI_COLOR_RESET, line);
        }
        ++i;
    }

    fclose(fp);
    if (line)
        free(line);
    
    
}

int main()
{
    int i, j, k, numberOfLines;
    char linuxCmd[100];
    memset(linuxCmd,0,100*sizeof(char));
    
    int wasLineRun[LINE_COUNT_LARGER_THAN_DOMINION]; // to hold information if the line was run for that unit test
    memset(wasLineRun,0,LINE_COUNT_LARGER_THAN_DOMINION*sizeof(int));
    int didTestPass;
    int numPassedTestsPerLine[LINE_COUNT_LARGER_THAN_DOMINION];
    memset(numPassedTestsPerLine,0,LINE_COUNT_LARGER_THAN_DOMINION*sizeof(int));
    int numFailedTestsPerLine[LINE_COUNT_LARGER_THAN_DOMINION];
    memset(numFailedTestsPerLine,0,LINE_COUNT_LARGER_THAN_DOMINION*sizeof(int));
    double huePerLine[LINE_COUNT_LARGER_THAN_DOMINION]; // don't need to preassign 0s, as we assign each value ourselves later
    int numPassedTestsTotal = 0;
    int numFailedTestsTotal = 0;
    double tempNum, tempDenom;
    
    char * progNames[8] = {"unittest1","unittest2","unittest3","unittest4","cardtest4","randomtestadventurer","randomtestcard1","randomtestcard2"};
    int numUnitTestsPerProgram[8] = {48,18,49,77,4,50,50,50};
    
    // k loops over our testing programs unittest1 to unittest4
    for (k = 1; k <= 8; ++k)
    {        
        // for each unit test possible from unittest k program...
        for (i = 1; i <= numUnitTestsPerProgram[k-1]; ++i)
        {    
            // remove old files, compile unittest k program
            sprintf(linuxCmd, "rm -f dominion.o *.gcov *.gcda *.gcno *.so %s", progNames[k-1]);
            system(linuxCmd);
            system("gcc -c dominion.c -g -Wall -fpic -coverage -lm");
            sprintf(linuxCmd, "gcc -o %s %s.c -g dominion.o rngs.o interface.o -Wall -fpic -coverage -lm", progNames[k-1], progNames[k-1]);
            system(linuxCmd);

            // run unit test i for this program
            sprintf(linuxCmd,"%s %d > /dev/null",progNames[k-1], i);
            if (system(linuxCmd) == 0) // == 0 means test passed
            {
                didTestPass = 1;
                numPassedTestsTotal++;
            }
            else
            {
                didTestPass = 0;
                numFailedTestsTotal++;
            }
            
            // generate dominion.c.gcov file
            system("gcov dominion.c > /dev/null");
            
            // find out which lines of dominino.c were run and get line count of dominion.c (via dominion.c.gcov)
            numberOfLines = discoverExecutedLines(wasLineRun);
            
            // for each line, if the test we just did passed and we ran the line, increment the number of passed tests counted for that line
            // also do similar for failed tests
            for (j = 0; j < numberOfLines; ++j)
                if (didTestPass == 1 && wasLineRun[j] == 1)
                    numPassedTestsPerLine[j]++; 
                else if (didTestPass == 0 && wasLineRun[j] == 1)
                    numFailedTestsPerLine[j]++;
        }
    }
    
    // calculate hue of each line given the data we've gathered
    // 0 = most suspicious, 1 = least suspicious, negative means the statement was never run
    for (j = 0; j < numberOfLines; ++j)
    {
        // if these are both 0, the line was never run during any test
        if (numPassedTestsPerLine[j] == 0 && numFailedTestsPerLine[j] == 0)
            huePerLine[j] = -1.0;
        // if no tests ever passed, that means any statement that was run is very suspicious (this is really unlikely)
        else if (numPassedTestsTotal == 0)
            huePerLine[j] = 0.0;
        // if no tests ever failed, that means every statement that was run is not very suspicious (not super likely this occurs either)
        else if (numFailedTestsTotal == 0)
            huePerLine[j] = 1.0;
        else
        {
            tempNum = numPassedTestsPerLine[j] / (double)numPassedTestsTotal;
            tempDenom = numFailedTestsPerLine[j] / (double)numFailedTestsTotal;
            
            huePerLine[j] = tempNum / (tempNum + tempDenom);
        }        
    }
    
    printDominionCodeBasedOnHue(huePerLine);
    
    return 0;
}
