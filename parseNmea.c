/* NMEA Sentence Description/Example
    Type => RMC—Recommended Minimum Navigation Information

    Type, Time, Status, Lat, N/S, Long, E/W, Speed, Course, Date,
    $GPRMC,064951.000,A,2307.1256,N,12016.4438,E,0.03,1 65.48,260406,3.05,W,A*2C
*/

#include <string.h>
#include <stdio.h>
#include <stdint.h>

static const uint8_t true = 1;
static const uint8_t false = 0;

int main(void) {
    char sentence[] = "\n$GGG,3232432\n$HHH,3232432\n$GPRMC,064951.000,A,2307.1256,N,12016.4438,E,0.03,1 65.48,260406,3.05,W,A*2C\n$GGG,3232432\n$HHH,3232432\n"; // Initial NMEA sentence example
    const char *delimiter = ",$";
    char *currentParsed;
    char sentenceToParse[128];

    uint8_t i = 0;
    uint8_t gpmrcDone = false;

    strcpy(sentenceToParse, sentence);
    printf("sentenceToParse is: %s\n", sentenceToParse);

    currentParsed = strtok(sentenceToParse, "$");
    while (gpmrcDone != true) {
        currentParsed = strtok(NULL, "$");
        currentParsed = strtok(NULL, ",");

        if (strcasecmp(currentParsed,"GPRMC") == 0) {
            for (i = 0; i < 12; i++) {
                currentParsed = strtok(NULL, delimiter);
                printf("%d: %s\n", i, currentParsed);
            }
            gpmrcDone = true;
        }
    }

    return 0;
}
