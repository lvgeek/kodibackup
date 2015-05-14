#define RASPBERRY_PI    (1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/types.h>
#include <pthread.h>
#include <signal.h>

#include "event-server.h"
#include "commands.h"
#include "SF_BC127.h"

#include "config.h"
#include "common.h"

/*
 * Private Functions Declaration
 */
static void intHandler(int dmmy);
static void REMOTE_ReadConfig(const char *fileName);

/*
 * Local Variables
 */
tatic bool_t       mForceStop = FALSE;

/*
 * Global Variables
 */
uint8_t             gSystemMode; /* systemModes_t */

/*
 * Public functions definitions
 */
int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        print("Please provide the path to the configuration file\n");
        print("usage: %s path/to/configuration_file\n", argv[0]);
    }

    if(!strcmp(argv[1], "--help"))
    {
        print("CarPC Remote controller\n");
        print("usage: %s path/to/configuration_file\n\n", argv[0]);
        print("Author: Andrei Istodorescu\n");
        print("andrei.istodorescu@gmail.com\n");
        print("www.engineering-diy.blogspot.com\n");
    }

    /* XBMC Event Client Module */
    XBMC_ClientInit("CarPC Controller", (const char*)NULL);

    /* BC127 Event Server Module */
    EventServer_Init();

    /* Commands Module Interface */
    Commands_Init();

    /* Radio Module Interface */
    BC127_Init();

    /* Restore default mode */
    gSystemMode = mModeBC127;
    XBMC_ClientAction("SetProperty(BC127.Active,true,10000)");

    /* Main loop */
    while(!mForceStop)
    {
        //REMOTE_ProcessButtons();
        //REMOTE_ProcessEncoders();

        /* Reduce CPU usage: sleep 0.5ms */
        //usleep(500);
    	usleep(1000000000);
    }

    return 0;
}


/*
 * Private functions definitions
 */
static void intHandler(int dmmy)
{
    print("Interrupt signal caught\n");

    Commands_UnInit();
    EventServer_DeInit();
    mForceStop = TRUE;
}

static void REMOTE_ReadConfig(const char *fileName)
{
    FILE    *fp;
    char    *pBuffer;
    uint8_t i;
    char    *pData;
    uint8_t ipAddress[50];

    memset((void*)ipAddress, 0, 50);
    memset(mButtons, 0, sizeof(button_t) * BUTTONS_NB);

    pBuffer = (char*)malloc(LINE_MAX_SIZE);
    fp = fopen(fileName, "r");

    if(fp<0)
    {
        perror("file read went wrong\n");
        print("file read went wrong\n");
    }

    while(fgets(pBuffer, LINE_MAX_SIZE, fp))
    {
        if(pBuffer[0] == '#' || pBuffer[0] == '\n' || pBuffer[0] == '\r')
            continue;

        /* Replace ':' with '\0'(end of string) character */
        for(i = 0; i < LINE_MAX_SIZE; i++)
        {
            if(!pBuffer[i]) break;
            if(pBuffer[i] == ':' || pBuffer[i] == '\n' || pBuffer[i] == '\r' || pBuffer[i] == '#')
                pBuffer[i] = 0;
        }

        /* Get ip address */
        pData = strstr(pBuffer, "[ip]");
        if(pData)
        {
            pData += strlen(pData) + 1;
            strcpy((char*)ipAddress, pData);
            pData += strlen(pData);
            continue;
        }

        /* Get click debounce value */
        pData = strstr(pBuffer, "click_skip");
        if(pData)
        {
            pData += strlen(pData) + 1;
            mClickSkip = atoi(pData);
            continue;
        }

        /* Get click hold value */
        pData = strstr(pBuffer, "click_hold");
        if(pData)
        {
            pData += strlen(pData) + 1;
            mClickHold = atoi(pData);
            continue;
        }

        /* Get encoder hold value */
        pData = strstr(pBuffer, "encoder_hold");
        if(pData)
        {
            pData += strlen(pData) + 1;
            mEncoderHold = atoi(pData);
            continue;
        }

        /* Parse button entry */
        pData = strstr(pBuffer, "button");
        if(pData)
        {
            pData += strlen(pData) + 1; /* skip 'button:' */
            mButtons[mButtonsNb].gpio = atoi(pData); /* get pin number */
            RPIGPIO_SetMode(mButtons[mButtonsNb].gpio, GPIO_IN);
            pData += strlen(pData) + 1; /* skip 'pin number:' */
            strcpy(mButtons[mButtonsNb].action, pData); /* get command(s) */

            {
                char *pStr;

                pStr = mButtons[mButtonsNb].action;
                mButtons[mButtonsNb].pCommands[mButtons[mButtonsNb].iCmd] = pStr;
                while(*pStr)
                {
                    if(*pStr == '>')
                    {
                        *pStr = 0;
                        mButtons[mButtonsNb].iCmd++;
                        mButtons[mButtonsNb].pCommands[mButtons[mButtonsNb].iCmd] = pStr + 1;
                    }

                    /* Don't exceed available array */
                    if(mButtons[mButtonsNb].iCmd >= MAX_COMMANDS) break;

                    pStr++;
                }

                /* Set the number of commands */
                mButtons[mButtonsNb].nbCmd = mButtons[mButtonsNb].iCmd + 1;
            }

            mButtonsNb++;
            continue;
        }

        /* Parse rotary encoder entry */
        pData = strstr(pBuffer, "encoder");
        if(pData)
        {
            pData += strlen(pData) + 1; /* skip 'encoder:' */
            pData += strlen(pData) + 1; /* skip 'sl:' */
            mEncoders[mEncodersNb].skipTimesLeft = atoi(pData);
            pData += strlen(pData) + 1; /* skip value: */
            pData += strlen(pData) + 1; /* skip 'sr:' */
            mEncoders[mEncodersNb].skipTimesRight = atoi(pData);
            pData += strlen(pData) + 1; /* skip value: */
            pData += strlen(pData) + 1; /* skip 'CLK:' */
            mEncoders[mEncodersNb].gpioClk = atoi(pData); /* get CLK pin(11) */
            RPIGPIO_SetMode(mEncoders[mEncodersNb].gpioClk, GPIO_IN);
            pData += strlen(pData) + 1; /* skip value: */
            pData += strlen(pData) + 1; /* skip DT: */
            mEncoders[mEncodersNb].gpioDt = atoi(pData); /* get DT pin(9) */
            RPIGPIO_SetMode(mEncoders[mEncodersNb].gpioDt, GPIO_IN);
            pData += strlen(pData) + 1; /* skip '9' */
            strcpy(mEncoders[mEncodersNb].actionLeft, pData); /* get left command(s) */
            pData += strlen(pData) + 1; /* skip left command(s) */
            strcpy(mEncoders[mEncodersNb].actionRight, pData); /* get right command(s) */

            mEncodersNb++;
            continue;
        
        }
    }

    free(pBuffer);
    fclose(fp);
}


