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
#include "BC127.h"


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
//uint8_t                 xbmcVol = XBMC_START_VOL;
//static char             pReplyMsg[30];
//extern uint8_t          gSystemMode;
static bool_t           mForceStop = FALSE;
/*
 * Public functions definitions
 */
void EventServer_Init()
{
    int err;
    struct sockaddr_in sockAddr;

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
    int                 recvSize;
    struct sockaddr_in  sockAddr;
    socklen_t           addrLen = sizeof sockAddr;
    bool_t              updateFrequency = FALSE;


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
