#define RASPBERRY_PI    (1)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <rpc/types.h>
#include <pthread.h>
#include <signal.h>

#include "RPI_gpio.h"
#include "xbmcclient_if.h"
#include "event-server.h"


#include "commands.h"
#include "radio.h"
#include "si4703.h"

#include "config.h"
#include "common.h"

/*
 * Defines
 */


/*
 * Typedefs
 */


/*
 * Private Functions Declaration
 */
static void intHandler(int dmmy);
static uint8_t grayToBinary(uint8_t num);
static void REMOTE_ReadConfig(const char *fileName);
void REMOTE_ProcessButtons();
void REMOTE_ProcessEncoders();
static char *REMOTE_But_GetNextCommand(button_t *pBut);


/*
 * Local Variables
 */
static uint8_t      mClickSkip = 7;
static uint32_t     mClickHold = 10000;
static uint32_t     mEncoderHold = 10000;
static button_t     mButtons[BUTTONS_NB];
static uint8_t      mButtonsNb = 0;
static encoder_t    mEncoders[ENCODERS_NB];
static uint8_t      mEncodersNb = 0;
static bool_t       mForceStop = FALSE;



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

    memset(mEncoders, 0, sizeof(encoder_t) * ENCODERS_NB);
    memset(mButtons, 0, sizeof(button_t) * BUTTONS_NB);
    /* Install signal handler */
    signal(SIGINT, intHandler);

    /* Initialize GPIO module */
    RPIGPIO_Init();

    /* Get configuration from the input file */
    REMOTE_ReadConfig(argv[1]);

    /* XBMC Event Client Module */
    XBMC_ClientInit("CarPC GPIO Controller", (const char*)NULL);

    /* CarPC Event Server Module */
    EventServer_Init();

    /* Commands Module Interface */
    Commands_Init();

    /* Radio Module Interface */
    Radio_Init(RADIO_START_VOL, RADIO_START_FREQ, 35);

    /* Restore default mode */
    gSystemMode = mModeRadio;
    XBMC_ClientAction("SetProperty(Radio.Active,true,10000)");

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

    XBMC_ClientDeInit();
    Commands_UnInit();
    EventServer_DeInit();
    mForceStop = TRUE;
}

static uint8_t grayToBinary(uint8_t num)
{
    uint8_t mask;
    for(mask = num >> 1; mask != 0; mask = mask >> 1)
    {
        num = num ^ mask;
    }
    return num;
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

void REMOTE_ProcessButtons()
{
    uint8_t i;

    for(i=0;i<mButtonsNb;i++)
    {
#if RASPBERRY_PI
        mButtons[i].click = !RPIGPIO_Read((unsigned)mButtons[i].gpio);
#endif /* RASPBERRY_PI */
        if(mButtons[i].click)
        {
            mButtons[i].clickTimes++;

            /* Treat just one click from 7(CLICK_DEBOUNCE) */
            if(mButtons[i].clickTimes >= mClickSkip)
            {
                char *pComm;

                /* Get the next group of commands(in case there are more groups of commands) */
                pComm = REMOTE_But_GetNextCommand((button_t*)(mButtons + i));
                /* Execute commands in a group of commands */
                //Commands_Execute(pComm);
                Commands_Signal(pComm);
                mButtons[i].clickTimes = 0;

                //usleep(mClickHold); /* Hold 1 second between two clicks */
            }

            mButtons[i].click = 0;
        }
    }
}

void REMOTE_ProcessEncoders()
{
    uint8_t i;

    for(i=0;i<mEncodersNb;i++)
    {
        /* Rotary Encoder */
#if RASPBERRY_PI
        mEncoders[i].dtNew = RPIGPIO_Read((unsigned)mEncoders[i].gpioDt);
        mEncoders[i].clkNew = RPIGPIO_Read((unsigned)mEncoders[i].gpioClk);
#endif /* RASPBERRY_PI */
        mEncoders[i].valNew = grayToBinary((mEncoders[i].clkNew << 1) | mEncoders[i].dtNew);

        /* New values arrived */
        if(mEncoders[i].valNew != mEncoders[i].valOld)
        {
            /* Update movement once in 4 changes(and not for every value change),
                 to avoid unneeded changes */
            /* LEFT */
            if(mEncoders[i].valOld == 0 && mEncoders[i].valNew == 1)
            {
                mEncoders[i].direction = RIGHT;
                mEncoders[i].changed = 1;
            }
            else if(mEncoders[i].valOld == 1 && mEncoders[i].valNew == 2)
            {
                mEncoders[i].direction = RIGHT;
                mEncoders[i].changed = 1;
            }
            else if(mEncoders[i].valOld == 2 && mEncoders[i].valNew == 3)
            {
                mEncoders[i].direction = RIGHT;
                mEncoders[i].changed = 1;
            }
            else if(mEncoders[i].valOld == 3 && mEncoders[i].valNew == 0)
            {
                mEncoders[i].direction = RIGHT;
                mEncoders[i].changed = 1;
            }
            /* RIGHT */
            else if(mEncoders[i].valOld == 0 && mEncoders[i].valNew == 3)
            {
                mEncoders[i].direction = LEFT;
                mEncoders[i].changed = 1;
            }
            else if(mEncoders[i].valOld == 3 && mEncoders[i].valNew == 2)
            {
                mEncoders[i].direction = LEFT;
                mEncoders[i].changed = 1;
            }
            else if(mEncoders[i].valOld == 2 && mEncoders[i].valNew == 1)
            {
                mEncoders[i].direction = LEFT;
                mEncoders[i].changed = 1;
            }
            else if(mEncoders[i].valOld == 1 && mEncoders[i].valNew == 0)
            {
                mEncoders[i].direction = LEFT;
                mEncoders[i].changed = 1;
            }

			//printf("direction:%d, changed:%d, old:%d, new:%d\n", mEncoders[i].direction,mEncoders[i].changed,
            //mEncoders[i].valOld, mEncoders[i].valNew);
            
            /* Check if we have a new movement */
            if(mEncoders[i].changed)
            {
                /* Treat the LEFT movement */
                if(mEncoders[i].direction == LEFT)
                {
                    /* If we haven't reached the number of steps to be ignored just increase the
                     * current step value */
                    if(mEncoders[i].leftSteps < mEncoders[i].skipTimesLeft)
                    {
                        //printf("increase left step: %d\n", mEncoders[i].leftSteps);
                        mEncoders[i].leftSteps++;
                    }
                    /* If we have reached the number of steps to be skipped then reset the current
                     * step number and execute the assigned commands */
                    else
                    {
                        mEncoders[i].leftSteps = 0; /* Reset value */
                        Commands_Signal((char*)mEncoders[i].actionLeft);
                        //usleep(mEncoderHold);
                    }
                }
                /* Treat the RIGHT movement */
                else
                {
                    /* If we haven't reached the number of steps to be ignored just increase the
                     * current step value */
                    if(mEncoders[i].rightSteps < mEncoders[i].skipTimesRight)
                    {
                        //printf("increase right step: %d\n", mEncoders[i].rightSteps);
                        mEncoders[i].rightSteps++;
                    }
                    /* If we have reached the number of steps to be skipped then reset the current
                     * step number and execute the assigned commands */
                    else
                    {
                        mEncoders[i].rightSteps = 0; /* Reset value */
                        Commands_Signal((char*)mEncoders[i].actionRight);
                        //usleep(mEncoderHold);
                    }
                }

                mEncoders[i].changed = 0;
            }

            /* Update old value */
            mEncoders[i].valOld = mEncoders[i].valNew;
        }

    }
}


/*! \brief Get the next command to be executed, from a list of commands.
 *
 *  \param[IN] pBut pointer to the button
 *  \return char * pointer to the command to be executed
 */
static char *REMOTE_But_GetNextCommand(button_t *pBut)
{
    char *pRes = NULL;

    pRes = pBut->pCommands[pBut->iCmd];

    if(pBut->iCmd >= pBut->nbCmd - 1)
    {
        pBut->iCmd = 0;
    }
    else
    {
        pBut->iCmd++;
    }

    return pRes;
}
