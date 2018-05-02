#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<time.h>

char inputChar()
{
    // the value returned from inputChar needs to be one of the 8 in this array to reach state 9
    char possibleChars[] = { '[','(','{',' ','a','x','}',')',']' };

    // rand() % 9 produces an integer from 0 through 8, inclusive, and is used as an index to extract a random character from the 9 possible chars in possibleChars
    return possibleChars[rand() % 9]; 
}

char *inputString()
{
    // this c-str is used to store/build the string the function returns
    // static is needed to ensure the memory is kept around when the function returns
    // see next comment section for why its length is 7 instead of 6 (the length of "reset" including the null byte)
    static char returnString[7];

    // the string to compare with is only 6 chars long (the 5 for "reset" and the 1 null byte)
    // but, I'm including a 7th char that is always a null byte because sometimes the random 6 chars used will not contain a null byte
    // having this 7th one guarantees the returned string is always a proper c-string
    // but it does not interfere with any characters the testme looks at (testme never examines the 6th index of the returned string to reach error, but it will in the printf statement when none of the 6 chars get randomly assigned a null byte
    returnString[6] = '\0';
    
    // to reach the error printout from state 9, the testme function needs to have the string returned from this function be exactly "reset" (with a null byte after the 't')
    // so this array contains all the unique characters that are needed to build that exact length 6 char array
    char possibleChars[] = { 'r','e','s','t','\0' };

    // iteration variable over the chars in returnString
    int i;

    // for every char i in return String (indices 0-5, inclusive)
    for (i = 0; i < 6; ++i)
        // assign char i to be one of the possible chars in possibleChars, at random (random index from 0-4, inclusive), required to build the requisite c-string
        returnString[i] = possibleChars[rand() % 5];

    // now that the string is built (all 6 chars are assigned), it can be returned
    return returnString;
}

void testme()
{
    int tcCount = 0;
    char *s;
    char c;
    int state = 0;
    while (1)
    {
        tcCount++;
        c = inputChar();
        s = inputString();
        printf("Iteration %d: c = %c, s = %s, state = %d\n", tcCount, c, s, state);

        if (c == '[' && state == 0) state = 1;
        if (c == '(' && state == 1) state = 2;
        if (c == '{' && state == 2) state = 3;
        if (c == ' '&& state == 3) state = 4;
        if (c == 'a' && state == 4) state = 5;
        if (c == 'x' && state == 5) state = 6;
        if (c == '}' && state == 6) state = 7;
        if (c == ')' && state == 7) state = 8;
        if (c == ']' && state == 8) state = 9;
        if (s[0] == 'r' && s[1] == 'e'
            && s[2] == 's' && s[3] == 'e'
            && s[4] == 't' && s[5] == '\0'
            && state == 9)
        {
            printf("error ");
            exit(200);
        }
    }
}


int main(int argc, char *argv[])
{
    srand(time(NULL));
    testme();
    return 0;
}
