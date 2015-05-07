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
    /* BC127 Event Server Module */
    EventServer_Init();

    /* Commands Module Interface */
    Commands_Init();

    /* Radio Module Interface */
    Radio_Init(RADIO_START_VOL, RADIO_START_FREQ, 35);

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


