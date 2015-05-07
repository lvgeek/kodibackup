#include <pthread.h>
#include <mqueue.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "radio.h"
#include "common.h"
#include "config.h"
#include "si4703.h"
#include "xbmcclient_if.h"
#include "event-server.h"

/*
 * Defines
 */


/*
 * Typedefs
 */

/*
 * Private Functions Declarations
 */
static void Commands_Execute(char *pStr);
static void *Commands_ProcessLoop(void *pParam);

/*
 * Local Variables
 */
static pthread_t        mCommandsProcessThread;
static mqd_t            mCommandsMessageQueueFd;
static struct mq_attr   mCommandsMessageQueueBuf;
static bool_t           mForceStop = FALSE;


/*
 * Global Variables
 */
extern uint8_t          gSystemMode;
extern bool_t           gRdsEnabled;
extern bool             gFrequencyChanged;

/*
 * Public functions definitions
 */
void Commands_Init()
{
	int res;

    //pthread_mutex_init(&lockX, NULL);
    printf("Starting Commands Module\n");

    /* Create message queue */
    mCommandsMessageQueueBuf.mq_msgsize = 512;
    mCommandsMessageQueueBuf.mq_maxmsg = 16;
    /*mCommandsMessageQueueFd = mq_open("/QCarPC", O_RDWR);
    if(mCommandsMessageQueueFd < 0)
    {
    	printf("++++++++<0\n");*/
        mCommandsMessageQueueFd = mq_open("/QCarPC", O_CREAT | O_RDWR, 600, &mCommandsMessageQueueBuf);
    //}

    if(mCommandsMessageQueueFd < 0)
        perror("Error");

    /*res = mq_send(mCommandsMessageQueueFd, pStr, strlen(pStr) + 1, 1);
    printf("res: %d\n", res);
    if(res < 0)
    {
    	printf("error fd %d, retrying\n", mCommandsMessageQueueFd);

    	mq_close(mCommandsMessageQueueFd);
    }*/

	printf("init ok %d\n", mCommandsMessageQueueFd);

    /* Create radio periodically update thread */
    pthread_create(&mCommandsProcessThread, NULL, Commands_ProcessLoop, NULL);
}

void Commands_UnInit()
{
    mq_close(mCommandsMessageQueueFd);
}

void Commands_Signal(char *pStr)
{
    int res;
    printf("sending: %s\n", pStr);

    res = mq_send(mCommandsMessageQueueFd, pStr, strlen(pStr) + 1, 1);
    if(res < 0)
	{
    	printf("error fd %d\n", mCommandsMessageQueueFd);
	}
}


/*
 * Private functions definitions
 */
static void *Commands_ProcessLoop(void *pParam)
{
    unsigned int msgPrio;
    void *pBuff = calloc(256, 1);

    while(!mForceStop)
    {
        int ret;

        ret = mq_receive(mCommandsMessageQueueFd, (char*)pBuff, mCommandsMessageQueueBuf.mq_msgsize, &msgPrio);
        if (ret >= 0) /* got a message */
        {
            Commands_Execute((char*)pBuff);
        }
        else if(ret < 0)
		{
			printf("received: %d\n", ret);
		}
    }

    return NULL;
}

/*! \brief Parse a string and send the commands found via sockets.
 *
 *  \param[IN/OUT] pStr pointer to the command that should be executed
 */
extern void REMOTE_ProcessButtons();
extern void REMOTE_ProcessEncoders();
extern void *Radio_Loop(void *pParam);
static void Commands_Execute(char *pStr)
{
    char        *pCommands[MAX_COMMANDS];
    uint16_t    si_regs[16];
    uint8_t     iComm = 0;
    char        *pComm = NULL;
    char        *pTmpStr = NULL;
    char        *pOrigStr = NULL;

    //pthread_mutex_lock(&lockX);

    if(pStr)
    {
    	printf("processing [%s]\n", pStr);
    	//printf("System mode: %d\n", gSystemMode);

    	memset(pCommands, 0, MAX_COMMANDS * sizeof(pCommands[0]));


    	/* Get the number of commands and setup pCommands array */
    	pTmpStr = strdup(pStr);
    	pOrigStr = pTmpStr; /* Used to free the correct buffer */
    	iComm = 0;
    	pCommands[iComm] = pTmpStr;
    	while(*pTmpStr)
    	{
    		if(*pTmpStr == '+')
    		{
    			iComm++;
    			pCommands[iComm] = pTmpStr + 1;
    			*pTmpStr = '\0';
    		}

    		/* Sanity check */
    		if(iComm >= MAX_COMMANDS - 1) break;

    		pTmpStr++;
    	}
    	iComm++;


    	/* Iterate through commands */
    	while(iComm)
    	{
    		pComm = pCommands[iComm - 1];

    		/* GPIO event detected */
    		if(!strcmp(pComm, "interrupt_gpio"))
    		{
    			REMOTE_ProcessButtons();
    			REMOTE_ProcessEncoders();
    		}
    		else if(!strcmp(pComm, "interrupt_rds"))
    		{
    			Radio_Loop(NULL);
    		}
    		/* XBMC Keyboard action */
    		else if((strstr(pComm, XBMCBUTTON_CMD)) && (gSystemMode == mModeXBMC))
    		{
    			XBMC_ClientButton((const char*)pComm + strlen(XBMCBUTTON_CMD), "KB");
    			//printf("KB_%s\n", pComm + strlen(XBMCBUTTON_CMD));
    		}
    		/* XBMC built-in function */
    		else if((strstr(pComm, XBMCBUILTIN_CMD)) && (gSystemMode == mModeXBMC))
    		{
    			XBMC_ClientAction(pComm + strlen(XBMCBUILTIN_CMD));
    			//printf("XBMC builtin_%s\n", pComm + strle(XBMCBUILTIN_CMD));
    		}
    		/* Toggle Radio/Media Center */
    		else if(strstr((const char*)pComm, SYSTEM_MODE_TOGGLE))
    		{
    			gSystemMode ^= 0x01;

    			si_read_regs(si_regs);

    			if(gSystemMode == mModeRadio)
    			{
    				XBMC_ClientAction("PlayerControl(Stop)");
    				XBMC_ClientAction("SetProperty(Radio.Active,true,10000)");
    				si_mute(si_regs, 0);
    			}
    			else
    			{
    				XBMC_ClientAction("SetProperty(Radio.Active,false,10000)");
    				si_mute(si_regs, 1);
    			}
    			printf("Mode: %s\n", (gSystemMode==mModeRadio)?"radio":"media center");
    		}
    		/* Volume should be updated for both Radio and XBMC */
    		else if(strstr(pComm, SYS_CMD_VOLUME))
    		{
    			System_VolumeUpdate(pComm);
    		}
    		/* RADIO action */
    		else if((strstr(pComm, RADIO_CMD)) && (gSystemMode == mModeRadio))
    		{
    			si_read_regs(si_regs);
    			if(!memcmp(pComm, RADIO_CMD_SEEK_UP, strlen(RADIO_CMD_SEEK_UP)))
    			{
    				si_seek(si_regs, SEEK_UP);
    				gFrequencyChanged = true;
    			}
    			else if(!memcmp(pComm, RADIO_CMD_SEEK_DOWN, strlen(RADIO_CMD_SEEK_DOWN)))
    			{
    				si_seek(si_regs, SEEK_DOWN);
    				gFrequencyChanged = true;
    			}
    			else if(!memcmp(pComm, RADIO_CMD_TUNE_UP, strlen(RADIO_CMD_TUNE_UP)))
    			{
    				int freq = si_get_freq(si_regs);

    				if(freq == 10800)
    				{
    					freq = 8800;
    				}
    				else
    				{
    					freq += 10;
    				}

    				si_tune(si_regs, freq);
    				gFrequencyChanged = true;
    			}
    			else if(!memcmp(pComm, RADIO_CMD_TUNE_DOWN, strlen(RADIO_CMD_TUNE_DOWN)))
    			{
    				int freq = si_get_freq(si_regs);

    				if(freq == 8800)
    				{
    					freq = 10800;
    				}
    				else
    				{
    					freq -= 10;
    				}

    				si_tune(si_regs, freq);
    				si_dump(si_regs, "test", 16);
    				gFrequencyChanged = true;
    			}
    			else if(strstr((const char*)pComm, RADIO_CMD_TUNE))
    			{
    				int frequency;
    				char *pPos;
    				char *pPos2;

    				pPos = (char*)pComm + strlen(RADIO_CMD_TUNE);

    				pPos2 = strchr(pPos, '.');
    				*pPos2 = *(pPos2 + 1);
    				*(pPos2 + 1) = *(pPos2 + 2);
    				*(pPos2 + 2) = 0;

    				frequency = atoi((const char*)(pComm + strlen(RADIO_CMD_TUNE)));
    				printf("new freq: %d\n", frequency);
    				printf("frequency to tune: %s\n", (char*)pComm + strlen(RADIO_CMD_TUNE));

    				si_tune(si_regs, frequency * 10);
    				gFrequencyChanged = true;
    			}
    			else if(strstr((const char*)pComm, RADIO_UPDATE_RDS))
    			{
    				Radio_RdsUpdate(10);
    				printf("update rds\n");
    			}
    		}
    		/* NAVIGATION commands */
    		/*else if(!strcmp(pComm, NAV_CMD_ZOOM_IN))
    		{
    			system("xdotool keyup '+'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_ZOOM_OUT))
    		{
    			system("xdotool keyup '-'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_UP))
    		{
    			system("xdotool keyup 'up'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_DOWN))
    		{
    			system("xdotool keyup 'down'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_LEFT))
    		{
    			system("xdotool keyup 'left'");
    		}
    		else if(!strcmp(pComm, NAV_CMD_RIGHT))
    		{
    			system("xdotool keyup 'right'");
    		}*/

    		iComm--;
    	}

    	free(pOrigStr);
    }
    //pthread_mutex_unlock(&lockX);
}
