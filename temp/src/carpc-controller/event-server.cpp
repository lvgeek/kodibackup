#include <pthread.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <rpc/types.h>
#include <unistd.h>

#include "common.h"
#include "commands.h"
#include "config.h"
#include "event-server.h"
#include "xbmcclient_if.h"
#include "si4703.h"


/*
 * Defines
 */
#define INPUT_BUFFER_SIZE   (40)

/*
 * Typedefs
 */

/*
 * Private Functions Declarations
 */
//static void *EventServer_Loop(void *pParam);

/*
 * Local Variables
 */
static pthread_t        mEventServerThread;
static int              sockEventServer;
uint8_t                 xbmcVol = XBMC_START_VOL;
static char             pReplyMsg[30];
extern uint8_t          gSystemMode;
static bool_t           mForceStop = FALSE;
static uint8_t          mVolumeLookup[16];
/*
 * Public functions definitions
 */
void EventServer_Init()
{
    int err;
    struct sockaddr_in sockAddr;


    uint8_t idx;

    for(idx=0;idx<16;idx++)
    {
        mVolumeLookup[idx] = ((int)(100 / 16)) * idx + 5;
    }

    /* Create Radio UDP connection(to communicate with the SI4703 FM radio Server) */
    sockEventServer = socket(AF_INET, SOCK_DGRAM, 0);
    memset((void*)&sockAddr, 0, sizeof(struct sockaddr_in));
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(5005);
    inet_pton(AF_INET, "127.0.0.1", &(sockAddr.sin_addr));
    err = bind(sockEventServer, (struct sockaddr*)&sockAddr, sizeof sockAddr);

    if(err)
    {
        perror("setsockopt");
        printf("Bind error\n");
        exit(1);
    }

    /* Create new thread */
    pthread_create(&mEventServerThread, NULL, EventServer_Loop, NULL);
}

void EventServer_DeInit()
{
    shutdown(sockEventServer, SHUT_RDWR);
}

/*
 * Private functions definitions
 */
void *EventServer_Loop(void *pParam)
{
    uint8_t             loopInBuff[INPUT_BUFFER_SIZE];
    uint16_t            si_regs[16];
    int                 recvSize;
    struct sockaddr_in  sockAddr;
    socklen_t           addrLen = sizeof sockAddr;
    bool_t              updateFrequency = FALSE;


    si_read_regs(si_regs);
    memset(loopInBuff, 0, INPUT_BUFFER_SIZE);


    while(!mForceStop)
    {
        /* Get data from the UDP RX buffer, but don't block */
        recvSize = recvfrom(sockEventServer, loopInBuff, INPUT_BUFFER_SIZE, 0,
                (struct sockaddr*)&sockAddr, &addrLen);

        if(recvSize > 0)
        {
            Commands_Signal((char*)loopInBuff);
        }
    }

    pthread_exit(NULL);

    return NULL;
}


void System_VolumeUpdate(char *pComm)
{
    char        pData[ACTION_MAX_SIZE];
    uint16_t    si_regs[16];
    uint8_t     radioVol;
    uint8_t     idx;

    si_read_regs(si_regs);

    /* Get radio volume */
    radioVol = si_get_volume(si_regs);
    printf("---------------------%d %d\n", radioVol, xbmcVol);


    /* Increase xbmc volume */
    if(strstr(pComm, "plus") && (xbmcVol < 100 - XBMC_VOL_INC))
    {
        printf("plus---------------------%d %d\n", radioVol, xbmcVol);
        xbmcVol += XBMC_VOL_INC;
    }
    /* Decrease xbmc volume */
    else if(strstr(pComm, "minus") && (xbmcVol > 0 + XBMC_VOL_INC))
    {
        printf("minus---------------------%d %d\n", radioVol, xbmcVol);
        xbmcVol -= XBMC_VOL_INC;
    }

    /* Set volume in XBMC */
    printf("---------------------%d %d\n", radioVol, xbmcVol);
    memset(pData, 0, ACTION_MAX_SIZE);
    sprintf(pData, "SetVolume(%d,1)", xbmcVol);
    printf("xbmc command: %s\n+++++++++++++\n", pData);
    XBMC_ClientAction(pData);

    if(gSystemMode == mModeRadio)
    {
        for(idx=0;idx<16;idx++)
        {
            if(xbmcVol - mVolumeLookup[idx] < (int)(100 / 16))
            {
                printf("Set volume to %d\n", idx);
                si_set_volume(si_regs, idx);
                break;
            }
        }
    }

    /* Update radio volume in XBMC */
    memset(pData, 0, ACTION_MAX_SIZE);
    sprintf(pData, "SetProperty(Radio.Volume,%d,10000)", idx);
    XBMC_ClientAction(pData);
}

void System_RSSIUpdate(uint16_t *regs)
{
    uint8_t aXbmcAction[100];

    sprintf((char*)aXbmcAction, "SetProperty(Radio.RSSI,%d,10000)", si_get_rssi(regs));
    XBMC_ClientAction((const char*)aXbmcAction);
}
