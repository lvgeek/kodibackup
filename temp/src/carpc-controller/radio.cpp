#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <ctype.h>

#include "rpi_i2c.h"
#include "RPI_gpio.h"
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
typedef struct rdsData_tag
{
    char programServiceName[9]; /* Program service name segment */
    char alternativeFreq[2];
    char radioText[64+1];
}rdsData_t;

/*
 * Private Functions Declarations
 */
void *Radio_Loop(void *pParam);

static void Radio_Reset();
static void Radio_Power(uint8_t val);
static void Radio_DumpRegisters();
static void Radio_Scan(uint8_t mode);
static int Radio_Seek(uint8_t dir);
static int Radio_Tune(int freq);
static int Radio_Spacing(uint16_t spacing);
static int Radio_rds(const char *arg);
static int Radio_SeVolume(uint8_t volume);
static int Radio_Set(const char *arg);

/*
 * Local Variables
 */
static bool_t       mForceStop = FALSE;
static uint16_t     mRegs[16];
static pthread_t    mRadioReadThread;
static uint8_t      mRssiLimit = 35;
static rdsData_t    mRdsData;

static uint8_t      mTmpPSName[3][9];
static uint8_t      miTmpPSName = 0;

/* 22 group types */
static const char *groupTypes[32] =
{
        "Basic Tuning and Switching Information only",
        "Basic Tuning and Switching Information only",
        "Program Item Number and Slow Labeling Codes only",
        "Program Item Number",
        "Radio Text only",
        "Radio Text only",
        "Applications Identification for ODA only",
        "Open Data Applications",
        "Clock Time and Date only",
        "Open Data Applications",
        "Transparent Data Channels (32 channels) or ODA",
        "Transparent Data Channels (32 channels) or ODA",
        "In-House Applications or ODA",
        "In-House Applications or ODA",
        "Radio Paging or ODA",
        "Open Data Applications",
        "Traffic Message Channel or ODA",
        "Open Data Applications",
        "Emergency Warning System or ODA",
        "Open Data Applications",
        "Program Type Name",
        "Open Data Applications",
        "Open Data Applications",
        "Open Data Applications",
        "Open Data Applications",
        "Open Data Applications",
        "Enhanced Radio Paging or ODA",
        "Open Data Applications",
        "Enhanced Other Networks Information only",
        "Enhanced Other Networks Information only",
        "Defined in RBDS only",
        "Fast Switching Information only"
};

/*
 * Global Variables
 */
extern uint8_t  gSystemMode; /* systemModes_t */
bool            gFrequencyChanged = true;

/*
 * Public functions definitions
 */
void Radio_Init(uint8_t volume, int frequency, uint8_t rssiLimit)
{
    //pthread_mutex_init(&lockX, NULL);
    printf("Starting Radio Module\n");

    mRssiLimit = rssiLimit;
    memset(&mRdsData, 0, sizeof(rdsData_t));

    memset(mTmpPSName, 'x', 3*9);
    mTmpPSName[0][8] = 0;
    mTmpPSName[1][8] = 0;
    mTmpPSName[2][8] = 0;

    printf("%s %s %s\n", mTmpPSName[0], mTmpPSName[1], mTmpPSName[2]);

    {
        si_init();
        /*
            rpi_i2c_open(0);
            rpi_i2c_config(I2C_SPEED_100K);
            rpi_i2c_set_slave(SI4703_ADDR);*/

        uint16_t si_regs[16];
        si_read_regs(si_regs);

        //  rpi_set_pin_mode(SI_RESET, RPI_GPIO_OUT);   /* RESET */
        //  rpi_set_pin_mode(3, RPI_GPIO_OUT);          /* SDA */
        //
        //  rpi_pin_clr(3);
        //  rpi_delay_ms(100);
        //  rpi_pin_clr(SI_RESET);
        //  rpi_delay_ms(100);
        //    rpi_pin_set(SI_RESET);
        //    rpi_delay_ms(1000);
        //    rpi_set_pin_mode(SI_RESET, RPI_GPIO_IN);

        rpi_set_pin_mode(SI_RESET, RPI_GPIO_OUT_LOW);
        rpi_delay_ms(10);
        rpi_set_pin_mode(SI_RESET, RPI_GPIO_IN);
        rpi_delay_ms(1);

        if (si_read_regs(si_regs) != 0) {
            printf("Unable to read Si4703!\n");
            return;
        }
        si_dump(si_regs, "Reset Map:\n", 16);

        // enable the oscillator
        si_regs[TEST1] |= XOSCEN;
        si_update(si_regs);
        rpi_delay_ms(500); // recommended delay
        si_read_regs(si_regs);
        si_dump(si_regs, "\nOscillator enabled:\n", 16);
        // the only way to reliable start the device is to powerdown and powerup
        // just powering up does not work for me after cold start
        uint8_t powerdown[2] = { 0, PWR_DISABLE | PWR_ENABLE };
        rpi_i2c_write(powerdown, 2);
        rpi_delay_ms(110);

        si_read_regs(si_regs);
        si_power(si_regs, PWR_ENABLE);
        si_dump(si_regs, "\nPowerup:\n", 16);

        // tune to the local station with known signal strength
        si_tune(si_regs, frequency);
        // si_tune() does not update registers
        si_regs[SYSCONF1] |= RDSIEN;
        si_regs[SYSCONF1] |= 0x04;
        si_regs[POWERCFG] |= RDSM;
        si_update(si_regs);
        rpi_delay_ms(10);
        si_read_regs(si_regs);
        si_dump(si_regs, "\nTuned\n", 16);
/*
        uint16_t si_regs[16];
        si_read_regs(si_regs);
        si_set_volume(si_regs, 2);

        si_read_regs(si_regs);
        si_tune(si_regs, 10670);
        si_read_regs(si_regs);*/
    }

    /* Create radio periodically update thread */
    //pthread_create(&mRadioReadThread, NULL, Radio_Loop, NULL);
}

void Radio_UnInit()
{
}

void Radio_FrequencyUpdate()
{
    uint16_t regs[16];
    char *pPos;
    char replyMsg[20];
    char pXbmcAction[45];

    si_read_regs(regs);
    sprintf(replyMsg, "%d", si_get_freq(regs));

    pPos = replyMsg + strlen(replyMsg) - 2;
    //*(pPos + 2) = *(pPos + 1); /* no need for the last 0 in frequency */
    *(pPos + 1) = *pPos;
    *pPos = '.';

    sprintf(pXbmcAction, "SetProperty(Radio.Frequency,%s,10000)", replyMsg);
    XBMC_ClientAction(pXbmcAction);
}

void Radio_RdsUpdate(uint32_t ticks)
{
    uint16_t    regs[16];
    int         rssi, lastRSSI = 0;
    char        text[65];
    char        pXbmcAction[150];

    while(ticks-- && gSystemMode == mModeRadio)
    {
        si_read_regs(regs);

        /* Check RDS */
        if(regs[STATUSRSSI] & RDSR)
        {
            uint16_t gt  = (regs[RDSB] >> 12) & 0x0F; // group type
            uint16_t ver = (regs[RDSB] >> 11) & 0x01; // version
            uint16_t tp  = (regs[RDSB] >> 10) & 0x01; // Traffic Program
            uint16_t pty = (regs[RDSB] >> 5) & 0x1F;
            uint16_t ta;
            uint16_t ms;
            uint16_t di;
            uint16_t idx;
            uint8_t ch;
            uint16_t abFlag;
            bool valid;

            //printf("%d group ready\n", gt);
            switch(gt)
            {
            /* Basic tuning and switching information */
            case 0:
            {
                ta  = (regs[RDSB] >> 4) & 0x01; // Traffic Announcement
                ms  = (regs[RDSB] >> 3) & 0x01; // Music/Speech
                di  = (regs[RDSB] >> 2) & 0x01; // Decoder control bit
                idx = (regs[RDSB] & 0x03); // PS name

                /* group 0A */
                if(ver == 0)
                {
                    mRdsData.alternativeFreq[0] = regs[RDSB] >> 8;
                    mRdsData.alternativeFreq[1] = regs[RDSB] & 0xFF;
                }
                /* group 0B */
                else
                {

                }

                /* Common 0A and 0B */
                ch = regs[RDSD] >> 8;
                //if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                //    || ch == '.')
                mRdsData.programServiceName[idx * 2] = ch;
                ch = regs[RDSD] & 0xFF;
                //if((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')
                //      || ch == '.')
                mRdsData.programServiceName[idx * 2 + 1] = ch;
            }
            break;
            case 1:
                break;
            case 2:
                abFlag = (regs[RDSB] >> 4) & 0x01;
                idx = regs[RDSB] & 0x0F;
                valid = false;

                /* group 2A */
                if(ver == 0)
                {
                    text[idx * 4] = regs[RDSC]>>8;
                    text[idx * 4 + 1] = regs[RDSC] & 0xFF;
                    text[idx * 4 + 2] = regs[RDSD]>>8;
                    text[idx * 4 + 3] = regs[RDSD] & 0xFF;
                    //printf("%s\n\r", text);
                    memcpy(mRdsData.radioText, text, 65);

                }
                /* group 2B */
                else
                {

                }
                break;
            case 14:
                break;
            case 15:
                break;
            }
        }

        rpi_delay_ms(10);
    }

    if(gSystemMode == mModeRadio)
    {
        /* RDS Program name */
        sprintf(pXbmcAction, "SetProperty(Radio.RadioText,%s,10000)", mRdsData.radioText);
        XBMC_ClientAction(pXbmcAction);

        /* RDS Radiotext */
        sprintf(pXbmcAction, "SetProperty(Radio.StationName,%s,10000)",
                mRdsData.programServiceName);
        XBMC_ClientAction(pXbmcAction);

        rssi = si_get_rssi(regs);
        if(lastRSSI != rssi)
        {
            lastRSSI = rssi;

            sprintf(pXbmcAction, "SetProperty(Radio.RSSI,%d,10000)", rssi);
            XBMC_ClientAction(pXbmcAction);
        }
    }
}

/*
 * Private functions definitions
 */
void *Radio_Loop(void *pParam)
{
    char        pPsText[20];
    uint16_t    regs[16];
    char        pXbmcAction[150];
    uint32_t    updateInfo = 0;
    uint8_t     idx2A = 0;
    char        text[65];
    bool        newPS, newRT;


	if(gSystemMode == mModeRadio)
	{
		if(gFrequencyChanged)
		{
			memset((uint8_t*)&mRdsData, 0, sizeof(rdsData_t));
			memset(text, 0, 65);
			gFrequencyChanged = false;
		}

		si_read_regs(regs);

		/* Check RDS */
		if((regs[STATUSRSSI] & RDSR) &&
				(((regs[STATUSRSSI] & BLERA) >> 9) < 3) &&
				(((regs[READCHAN] & BLERB) >> 14) < 3) &&
				(((regs[READCHAN] & BLERC) >> 12) < 3) &&
				((regs[READCHAN] & BLERA) >> 10) < 3
		)
		{
			uint16_t gt  = (regs[RDSB] >> 12) & 0x0F; // group type
			uint16_t ver = (regs[RDSB] >> 11) & 0x01; // version
			uint16_t tp  = (regs[RDSB] >> 10) & 0x01; // Traffic Program
			uint16_t pty = (regs[RDSB] >> 5) & 0x1F;
			uint16_t ta;
			uint16_t ms;
			uint16_t di;
			uint16_t idx;
			uint8_t ch;
			uint16_t abFlag;
			bool valid;

			newPS = newRT = false;

			//printf("%d group ready\n", gt);
			switch(gt)
			{
			/* Basic tuning and switching information */
			case 0:
			{
				ta  = (regs[RDSB] >> 4) & 0x01; // Traffic Announcement
				ms  = (regs[RDSB] >> 3) & 0x01; // Music/Speech
				di  = (regs[RDSB] >> 2) & 0x01; // Decoder control bit
				idx = (regs[RDSB] & 0x03); // PS name

				/* group 0A */
				if(ver == 0)
				{
					ch = regs[RDSB] >> 8;
					mRdsData.alternativeFreq[0] = ch;
					ch = regs[RDSB] & 0xFF;
					mRdsData.alternativeFreq[1] = ch;
				}
				/* group 0B */
				else
				{
					/* New text come */
					/*if(idx == 0)
							{
								memset(mRdsData.programServiceName, 0, 9);
							}*/


				}
				uint8_t countryCode = regs[RDSA] >> 12;
				uint8_t programType = (regs[RDSA] >> 8) & 0x0F;
				uint8_t programRefNumber = regs[RDSB] & 0xFF;
				/* Common 0A and 0B */
				ch = regs[RDSD] >> 8;
				if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
						(ch >= '0' && ch <= '9') || ch == '.' || ch == ' ')
					mRdsData.programServiceName[idx * 2] = ch;
				//mTmpPSName[miTmpPSName][idx * 2] = ch;
				ch = regs[RDSD] & 0xFF;
				if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
						(ch >= '0' && ch <= '9') || ch == '.' || ch == ' ')
					mRdsData.programServiceName[idx * 2 + 1] = ch;
				//mTmpPSName[miTmpPSName][idx * 2 + 1] = ch;

				/* printf("%d <%c%c><0x%02x,0x%02x> country code: 0x%02x, program type: %d, ref: %d, %d\n",
							idx, regs[RDSD] >> 8, regs[RDSD] & 0xFF, regs[RDSD] >> 8,
							regs[RDSD] & 0xFF, countryCode, programType, programRefNumber,
							(regs[RDSB] >> 2) & 0x01);*/
				newPS = true;

				//printf("Program: [% 8s] [%s]\n", mRdsData.programServiceName, mRdsData.radioText);
			}
			break;
			case 1:
				break;
			case 2:
				abFlag = (regs[RDSB] >> 4) & 0x01;
				idx = regs[RDSB] & 0x0F;
				valid = false;

				/* group 2A */
				if(ver == 0)
				{
					ch = regs[RDSC]>>8;
					if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
							(ch >= '0' && ch <= '9') || ch == '.')
						text[idx * 4] = ch;
					ch = regs[RDSC] & 0xFF;
					if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
							(ch >= '0' && ch <= '9') || ch == '.')
						text[idx * 4 + 1] = ch;
					ch = regs[RDSD]>>8;
					if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
							(ch >= '0' && ch <= '9') || ch == '.')
						text[idx * 4 + 2] = ch;
					ch = regs[RDSD] & 0xFF;
					if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
							(ch >= '0' && ch <= '9') || ch == '.')
						text[idx * 4 + 3] = ch;
					//printf("%s\n\r", text);
					memcpy(mRdsData.radioText, text, 65);
					newRT = true;
				}
				/* group 2B */
				else
				{

				}
				break;
			case 14:
				break;
			case 15:
				break;
			}

			{
				/*printf("%d %d %d %d\n",
						(regs[STATUSRSSI] & BLERA) >> 9,
						(regs[READCHAN] & BLERB) >> 14,
						(regs[READCHAN] & BLERC) >> 12,
						(regs[READCHAN] & BLERA) >> 10);*/


				/* RadioText is ended with 0x0D('\r') -> remove it */
				if(mRdsData.radioText[strlen(mRdsData.radioText) - 1] == '\r')
				{
					mRdsData.radioText[strlen(mRdsData.radioText) - 1] = 0;
				}

				if(newRT)
				{
					/* RDS Radiotext */
					sprintf(pXbmcAction, "SetProperty(Radio.RadioText,\"%s\",10000)", mRdsData.radioText);
					XBMC_ClientAction(pXbmcAction);
					//printf("RT: [%s]\n", pXbmcAction);
				}


				if(newPS)
				{
					/* RDS Program name */
					sprintf(pXbmcAction, "SetProperty(Radio.StationName,\"%s\",10000)",
							mRdsData.programServiceName);
					XBMC_ClientAction(pXbmcAction);
					//printf("SN: [%s]\n", pXbmcAction);
				}
			}
		}

		//System_RSSIUpdate(regs);
	}


    return NULL;
}

static void Radio_Reset()
{
    uint16_t si_regs[16];
    si_read_regs(si_regs);

//  rpi_set_pin_mode(SI_RESET, RPI_GPIO_OUT);   /* RESET */
//  rpi_set_pin_mode(3, RPI_GPIO_OUT);          /* SDA */
//
//  rpi_pin_clr(3);
//  rpi_delay_ms(100);
//  rpi_pin_clr(SI_RESET);
//  rpi_delay_ms(100);
//    rpi_pin_set(SI_RESET);
//    rpi_delay_ms(1000);
//    rpi_set_pin_mode(SI_RESET, RPI_GPIO_IN);

    rpi_set_pin_mode(SI_RESET, RPI_GPIO_OUT_LOW);
    rpi_delay_ms(10);
    rpi_set_pin_mode(SI_RESET, RPI_GPIO_IN);
    rpi_delay_ms(1);

    if (si_read_regs(si_regs) != 0) {
        printf("Unable to read Si4703!\n");
        return;
    }
    si_dump(si_regs, "Reset Map:\n", 16);

    // enable the oscillator
    si_regs[TEST1] |= XOSCEN;
    si_update(si_regs);
    rpi_delay_ms(500); // recommended delay
    si_read_regs(si_regs);
    si_dump(si_regs, "\nOscillator enabled:\n", 16);
    // the only way to reliable start the device is to powerdown and powerup
    // just powering up does not work for me after cold start
    uint8_t powerdown[2] = { 0, PWR_DISABLE | PWR_ENABLE };
    rpi_i2c_write(powerdown, 2);
    rpi_delay_ms(110);

    Radio_Power(1);
    // tune to the local station with known signal strength
    //si_tune(si_regs, DEFAULT_STATION);
    // si_tune() does not update registers
    si_update(si_regs);
    rpi_delay_ms(10);
    si_read_regs(si_regs);
    si_dump(si_regs, "\nTuned\n", 16);
}

static void Radio_Power(uint8_t val)
{
    uint16_t si_regs[16];
    si_read_regs(si_regs);

    if (val == 1) {
        si_power(si_regs, PWR_ENABLE);
        si_dump(si_regs, "\nPowerup:\n", 16);
    }
    if (val == 0) {
        si_power(si_regs, PWR_DISABLE);
        si_dump(si_regs, "\nPowerdown:\n", 16);
    }

    si_dump(si_regs, "\nPower:\n", 16);
}

static void Radio_DumpRegisters()
{
    uint16_t si_regs[16];
    si_read_regs(si_regs);
    si_dump(si_regs, "Registers map:\n", 16);
}

static void Radio_Scan(uint8_t mode)
{
    int nstations = 0;
    int freq, seek = 0;
    uint16_t si_regs[16];

    si_read_regs(si_regs);
    si_regs[POWERCFG] |= SKMODE; // stop seeking at the upper or lower band limit

    // mode as recommended in AN230, Table 23. Summary of Seek Settings

    if (mode > 5)
        mode = 5;

    si_set_channel(si_regs, 0);
    printf("scanning...\n");

    if (mode > 0) {
        si_regs[SYSCONF2] &= 0x00FF;
        si_regs[SYSCONF3] &= 0xFF00;
        uint16_t conf2 = 0;
        uint16_t conf3 = 0;

        if (mode == 1) {
            conf2 = 0x1900;
            conf3 = 0x0000;
        }
        if (mode == 2) {
            conf2 = 0x1900;
            conf3 = 0x0048;
        }
        if (mode == 3) {
            conf2 = 0x0C00;
            conf3 = 0x0048;
        }
        if (mode == 4) {
            conf2 = 0x0C00;
            conf3 = 0x007F;
        }
        if (mode == 5) {
            conf2 = 0x0000;
            conf3 = 0x004F;
        }
        si_regs[SYSCONF2] |= conf2;
        si_regs[SYSCONF3] |= conf3;

        si_update(si_regs);
    }

    while(1) {
        freq = si_seek(si_regs, SEEK_UP);
        if (freq == 0)
            break;
        seek = freq;
        nstations++;
        uint8_t rssi = si_regs[STATUSRSSI]  & RSSI;
        printf("%5d ", freq);
        for(int si = 0; si < rssi; si++)
            printf("-");
        printf(" %d", rssi);
        fflush(stdout);

        int dt = 0;
        if (rssi > mRssiLimit) {
            while(dt < 10000) {
                si_read_regs(si_regs);
                if (si_regs[STATUSRSSI] & STEREO)
                    break;
                rpi_delay_ms(10);
                dt += 10;
            }
        }
        uint16_t st = si_regs[STATUSRSSI] & STEREO;

        if (st) {
            printf(" ST");
            fflush(stdout);
            if (rssi > mRssiLimit) {
                int pi;
                char ps_name[16];
                if ((pi = si_rds_get_ps(ps_name, si_regs, 5000)) != -1)
                    printf(" %04X '%s'", pi, ps_name);
            }
        }
        printf("\n");
    }
    si_regs[POWERCFG] &= ~SKMODE; // restore wrap mode
    si_tune(si_regs, seek);

    printf("%d stations found\n", nstations);
    return;
}

extern int si_band[3][2];
extern int si_space[3];
int Radio_spectrum(const char *arg)
{
    uint16_t si_regs[16];
    uint8_t rssi_limit = mRssiLimit;

    si_read_regs(si_regs);
    int band = (si_regs[SYSCONF2] >> 6) & 0x03;
    int space = (si_regs[SYSCONF2] >> 4) & 0x03;
    int nchan = (si_band[band][1] - si_band[band][0]) / si_space[space];
/*
    if (arg)
        rssi_limit = (uint8_t)atoi(arg);
*/
    for (int i = 0; i <= nchan; i++) {
        si_regs[CHANNEL] &= ~CHAN;
        si_regs[CHANNEL] |= i;
        si_regs[CHANNEL] |= TUNE;
        si_update(si_regs);
        rpi_delay_ms(10);

        while(1) {
            si_read_regs(si_regs);
            if (si_regs[STATUSRSSI] & STC) break;
            rpi_delay_ms(10);
        }
        si_regs[CHANNEL] &= ~TUNE;
        si_update(si_regs);
        while(i) {
            si_read_regs(si_regs);
            if(!(si_regs[STATUSRSSI] & STC)) break;
            rpi_delay_ms(10);
        }

        uint8_t rssi = si_regs[STATUSRSSI]  & 0xFF;
        printf("%5d ", si_band[band][0] + i*si_space[space]);
        for(int si = 0; si < rssi; si++)
            printf("-");
        printf(" %d", rssi);
        fflush(stdout);

        int dt = 0;
        if (rssi > rssi_limit) {
            while(dt < 3000) {
                si_read_regs(si_regs);
                if (si_regs[STATUSRSSI] & STEREO)
                    break;
                rpi_delay_ms(10);
                dt += 10;
            }
        }
        uint16_t st = si_regs[STATUSRSSI] & STEREO;

        if (st) {
            printf(" ST");
            fflush(stdout);
            if (rssi > rssi_limit) {
                int pi;
                char ps_name[16];
                if ((pi = si_rds_get_ps(ps_name, si_regs, 5000)) != -1)
                    printf(" %04X '%s'", pi, ps_name);
            }
        }
        printf("\n");
    }

    return 0;
}

static int Radio_Seek(uint8_t dir)
{
    uint16_t si_regs[16];

    si_read_regs(si_regs);

    if (si_seek(si_regs, dir) == 0) {
        printf("seek failed\n");
        return -1;
    }

    return 0;
}

static int Radio_Tune(int freq)
{
    uint16_t si_regs[16];

    si_read_regs(si_regs);
    si_tune(si_regs, freq);
    si_read_regs(si_regs);
    freq = si_get_freq(si_regs);
    if (freq) {
        printf("Tuned to %d.%02dMHz\n", freq/100, freq%100);
        //si_dump(si_regs, "Register map:\n", 16);
        return 0;
    }
    return -1;
}

static int Radio_Spacing(uint16_t spacing)
{
    uint16_t si_regs[16];
    si_read_regs(si_regs);

    if (spacing == 200) spacing = 0;
    if (spacing == 100) spacing = 1;
    if (spacing == 50)  spacing = 2;
    if (spacing > 2) {
        //printf("Invalid spacing, use 200, 100, 50 kHz\n");
        return -1;
    }
    si_regs[SYSCONF2] &= ~SPACING;
    si_regs[SYSCONF2] |= spacing << 4;
    si_update(si_regs);

    spacing = (si_regs[SYSCONF2] & SPACING) >> 4;
    //printf("spacing %d\n", si_space[spacing]);
    return 0;
}

static int Radio_rds(const char *arg)
{
    uint16_t si_regs[16];
    uint16_t gtmask = 0xFFFF;

    si_read_regs(si_regs);

    if (arg) { // do we have an extra parameter?
        if (!strcmp(arg, "on")) {
            si_set_rdsprf(si_regs, 1);
            si_update(si_regs);
            si_read_regs(si_regs);
            printf("RDSPRF set to %d\n", (si_regs[SYSCONF3] & RDSPRF));
            si_dump(si_regs, "\nRegister map\n", 16);
            return 0;
        }
        if (!strcmp(arg, "off")) {
            si_set_rdsprf(si_regs, 0);
            si_update(si_regs);
            si_read_regs(si_regs);
            printf("RDSPRF set to %d\n", (si_regs[SYSCONF3] & RDSPRF));
            si_dump(si_regs, "\nRegister map\n", 16);
            return 0;
        }
        if (!strcmp(arg, "verbose")) {
            si_read_regs(si_regs);
            si_regs[POWERCFG] ^= RDSM;
            si_update(si_regs);
            si_read_regs(si_regs);
            printf("RDSM set to %d\n", (si_regs[POWERCFG] & RDSM));
            si_dump(si_regs, "\nRegister map\n", 16);
            return 0;
        }
        gtmask = (1 << atoi(arg)) & 0xFFFF;
    }

    si_rds_dump(si_regs, gtmask, 15000);
    return 0;
}

static int Radio_SeVolume(uint8_t volume)
{
    uint16_t si_regs[16];
    si_read_regs(si_regs);
    si_set_volume(si_regs, volume);
    return 0;
}

static int Radio_Set(const char *arg)
{
    uint16_t val;
    const char *pval;
    char buf[32];

    if (arg == NULL)
        return -1;

    pval = arg;
    if (pval == NULL)
        return -1;
    int i;
    for(i = 0; i < 31; i++) {
        if (*pval == '=')
            break;
        buf[i] = toupper(*pval++);
    }
    buf[i] = '\0';
    if (*pval != '=')
        return -1;

    val = (uint16_t)atoi(++pval);

    uint16_t regs[16];
    si_read_regs(regs);
    if (si_set_state(regs, buf, val) != -1) {
        si_update(regs);
        si_dump(regs, arg, 16);
        return 0;
    }
    return -1;
}
