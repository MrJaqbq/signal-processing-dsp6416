#include <limits.h>
#include "C6416dskinit.h"
#define using_bios                  //if BIOS don't use top of vector table
extern Uint32 fs;            			//for sampling frequency
extern Uint16 inputsource;
extern DSK6416_AIC23_CodecHandle hAIC23_handle;


void c6416_dsk_init()       			//dsp-peripheral initialization
{
DSK6416_init();                   	//call BSL to init DSK-EMIF,PLL)

hAIC23_handle=DSK6416_AIC23_openCodec(0, &config);//handle(pointer) to codec
DSK6416_AIC23_setFreq(hAIC23_handle, DSK6416_AIC23_FREQ_96KHZ);  //set sample rate
DSK6416_AIC23_rset(hAIC23_handle, 0x0004, inputsource);  // choose mic or line in
MCBSP_config(DSK6416_AIC23_DATAHANDLE,&AIC23CfgData);//interface 32 bits toAIC23

MCBSP_start(DSK6416_AIC23_DATAHANDLE, MCBSP_XMIT_START | MCBSP_RCV_START |
	MCBSP_SRGR_START | MCBSP_SRGR_FRAMESYNC, 220);//start data channel again
}

void comm_poll()					 		//added for communication/init using polling
{
	poll=1;              				//1 if using polling
   c6416_dsk_init();    				//init DSP and codec
}

void comm_intr()						 	//for communication/init using interrupt
{
	poll=0;                        	//0 since not polling
   IRQ_globalDisable();           	//disable interrupts
	c6416_dsk_init(); 					//init DSP and codec
	CODECEventId=MCBSP_getXmtEventId(DSK6416_AIC23_codecdatahandle);//McBSP1 Xmit

#ifndef using_bios						//do not need to point to vector table
  	IRQ_setVecs(vectors);     			//point to the IRQ vector table
#endif										//since interrupt vector handles this

  	IRQ_map(CODECEventId, 11);			//map McBSP1 Xmit to INT11
  	IRQ_reset(CODECEventId);    		//reset codec INT 11
   IRQ_globalEnable();       			//globally enable interrupts
  	IRQ_nmiEnable();          			//enable NMI interrupt
   IRQ_enable(CODECEventId);			//enable CODEC eventXmit INT11

	output_sample(0);        			//start McBSP interrupt outputting a sample
}

void output_sample(Uint32 out_data)    //for out to Left and Right channels
{
	short CHANNEL_data;

	AIC_data.uint=0;                 //clear data structure
	AIC_data.uint=out_data;          //32-bit data -->data structure

//The existing interface defaults to right channel. To default instead to the
//left channel and use output_sample(short), left and right channels are swapped
//In main source program use LEFT 0 and RIGHT 1 (opposite of what is used here)
	CHANNEL_data=AIC_data.channel[RIGHT]; 			//swap left and right channels
	AIC_data.channel[RIGHT]=AIC_data.channel[LEFT];
	AIC_data.channel[LEFT]=CHANNEL_data;
   if (poll) while(!MCBSP_xrdy(DSK6416_AIC23_DATAHANDLE));//if ready to transmit
		MCBSP_write(DSK6416_AIC23_DATAHANDLE,AIC_data.uint);//write/output data
}

void output_left_sample(short out_data)     	  	 //for output from left channel
{
	AIC_data.uint=0;                              //clear data structure
	AIC_data.channel[LEFT]=out_data;   //data from Left channel -->data structure

	if (poll) while(!MCBSP_xrdy(DSK6416_AIC23_DATAHANDLE));//if ready to transmit
		MCBSP_write(DSK6416_AIC23_DATAHANDLE,AIC_data.uint);//output left channel
}

void output_right_sample(short out_data)  		//for output from right channel
{
	AIC_data.uint=0;                             //clear data structure
	AIC_data.channel[RIGHT]=out_data; //data from Right channel -->data structure

	if (poll) while(!MCBSP_xrdy(DSK6416_AIC23_DATAHANDLE));//if ready to transmit
		MCBSP_write(DSK6416_AIC23_DATAHANDLE,AIC_data.uint);//output right channel
}

Uint32 input_sample()                      	  	//for 32-bit input
{
	short CHANNEL_data;

	if (poll) while(!MCBSP_rrdy(DSK6416_AIC23_DATAHANDLE));//if ready to receive
		 AIC_data.uint=MCBSP_read(DSK6416_AIC23_DATAHANDLE);//read data

//Swapping left and right channels (see comments in output_sample())
	CHANNEL_data=AIC_data.channel[RIGHT]; 			//swap left and right channel
	AIC_data.channel[RIGHT]=AIC_data.channel[LEFT];
	AIC_data.channel[LEFT]=CHANNEL_data;

	return(AIC_data.uint);
}

short input_left_sample()                   		//input to left channel
{
	if (poll) while(!MCBSP_rrdy(DSK6416_AIC23_DATAHANDLE));//if ready to receive
	 AIC_data.uint=MCBSP_read(DSK6416_AIC23_DATAHANDLE);//read into left channel
	return(AIC_data.channel[LEFT]);					//return left channel data
}

short input_right_sample()                  		//input to right channel
{
	if (poll) while(!MCBSP_rrdy(DSK6416_AIC23_DATAHANDLE));//if ready to receive
	 AIC_data.uint=MCBSP_read(DSK6416_AIC23_DATAHANDLE);//read into right channel
	return(AIC_data.channel[RIGHT]); 				//return right channel data
}







Uint32 fs=DSK6416_AIC23_FREQ_8KHZ;	//set sampling rate
#define DSK6416_AIC23_INPUT_MIC 0x0015
#define DSK6416_AIC23_INPUT_LINE 0x0011
Uint16 inputsource=DSK6416_AIC23_INPUT_LINE; // select input

//#include "dsk6416_dip.h"
//#include "dsk6416_led.h"

#define MAX_VALUE 400
#define MIN_VALUE -6969
#define NOISE 5
    
extern DSK6416_AIC23_CodecHandle hAIC23_handle;

short OUT_L, OUT_R;

short L_in[101]; 		/* PR�BKI WEJSIOWE L_in[0] NAJNOWSZA PR�BKA LEWA,L_in[101] NAJSTARSZA PR�BKA LEWA*/
short R_in[101];        /* PR�BKI WEJSIOWE P_in[0] NAJNOWSZA PR�BKA PRAWA,P_in[101] NAJSTARSZA PR�BKA Prawa*/

/* WSPӣCZYNNIKI WYLICZONE Z CZ�STOTLIWO�CI� PR�BKOWANIA 96kHz I ODCI�CIE DLA 1kHz */
short h1[]={
        0,      0,      0,      0,      0,      1,      2,      4,      6,
        9,     12,     17,     23,     29,     37,     46,     56,     68,
       81,     95,    111,    128,    146,    166,    186,    208,    231,
      255,    279,    304,    330,    356,    382,    408,    434,    459,
      484,    508,    531,    553,    574,    594,    612,    628,    642,
      654,    664,    672,    678,    682,    683,    682,    678,    672,
      664,    654,    642,    628,    612,    594,    574,    553,    531,
      508,    484,    459,    434,    408,    382,    356,    330,    304,
      279,    255,    231,    208,    186,    166,    146,    128,    111,
       95,     81,     68,     56,     46,     37,     29,     23,     17,
       12,      9,      6,      4,      2,      1,      0,      0,      0,
        0,      0
};

short FILTR_L (short, short*);//DEKLARACJE FUNKCJI ZWRACAJACE WARTOSC short 
short FILTR_R (short, short*);

#define CHANNEL_LENGTH 2

union deviceData
{
	Uint32 streamData;
	short channelData[2];
} currentDeviceData;

enum Channel 
{ 
	CHANNEL_RIGHT,
	CHANNEL_LEFT
};

enum Channel channels[CHANNEL_LENGTH] = {
	CHANNEL_RIGHT,
	CHANNEL_LEFT
};

short* outPtr[] = {
	&OUT_R,
	&OUT_L
};

short* inPtr[] = {
	&currentDeviceData.channelData[0],
	&currentDeviceData.channelData[1]
};

int muteButtons[] = {
	0,
	1
};

int filterButtons[] = {
	2,
	3
};

typedef short (*filterFunc_t)(short, short*);

filterFunc_t filters[] = 
{
	FILTR_R,
	FILTR_L
};

void readDeviceData(union deviceData* data)
{
	data->streamData = MCBSP_read(DSK6416_AIC23_DATAHANDLE);
}

void writeDeviceData(union deviceData* data)
{
	MCBSP_write(DSK6416_AIC23_DATAHANDLE, data->streamData);
}

void handleChannel(enum Channel channel)
{
	int mute = DSK6416_DIP_get(muteButtons[(int)(channel)]);
	int filter = DSK6416_DIP_get(filterButtons[(int)(channel)]);
	
	if(mute)
	{
		*(outPtr[channel]) = 0;
	}
	else if(filter)
	{
		*(outPtr[channel]) = (Uint32)filters[(int)channel](*(inPtr[channel]), h1);
	}
	else
	{
		*(outPtr[channel]) = *(inPtr[channel]);
	}
}

void writeChannel(enum Channel channel, union deviceData* device)
{
		device->channelData[channel] = *(outPtr[channel]);
}

// max 4294967294
// nois4294967292

short max = 0;
short min = 6969;

void handleLEDs(enum Channel channel)
{
	int i = 0;
 	float percentage_left = 0.0f;

	short data = *(inPtr[channel]);

	if(data > max)
		max = data;

	if(data < min)
		min = data;

	if(data > NOISE)
	{
		percentage_left = (float)(data - NOISE) / (float) (MAX_VALUE - NOISE);

		if(percentage_left > 0.75f)
		{
			DSK6416_LED_on(3);
		}
		else
		{
			DSK6416_LED_off(3);
		}


		if(percentage_left > 0.5f)
		{
			DSK6416_LED_on(2);
		}
		else
		{
			DSK6416_LED_off(2);
		}


		if(percentage_left > 0.25f)
		{
			DSK6416_LED_on(1);
		}
		else
		{
			DSK6416_LED_off(1);
		}


		if(percentage_left > 0.00f)
		{
			DSK6416_LED_on(0);
		}
		else
		{
			DSK6416_LED_off(0);
		}
	}
	else
	{
		for(i = 0; i < 4; i++)
		{
			DSK6416_LED_off(i);
		}
	}
}

void loop()
{
	int i = 0;
	enum Channel chan;

	readDeviceData(&currentDeviceData);

	for(; i < CHANNEL_LENGTH; i++)
	{
		chan = channels[i];

		handleChannel(chan);
        writeChannel(chan, &currentDeviceData);	
	}

	writeDeviceData(&currentDeviceData);

	handleLEDs(chan);
}

interrupt void c_int11()
{
	loop();
	return;
}

void main()
{
    // nasz kodek hAIC23_handle

    DSK6416_init();
    DSK6416_LED_init();
    DSK6416_DIP_init();

	comm_intr();
	while(1);
}


short FILTR_L (short input, short *h)
{
	int i;
	short output;
	int acc=0;
	int prod;
	L_in[0] = input;         	/* OD�WIE�A NAJNOWSZA PR�BK� */

	acc = 0;                      	/* ZERUJE AKUMULATOR */
	for (i=0; i<101; i++)         	/* PETLA WYKONA SIE 101 RAZY */
	{   
		prod = (h[i]*L_in[i]);  /* MNO�ENIE Q.15 */
		acc = acc + prod;       /* OD�WIE�ANIE 32-bit AKUMULATORA */
	}                             	
	output = (short) (acc>>15);    	/* PRZERABIANIE WYJSCIA DO WARTOSCI 16-bits. */

	for (i=100; i>0; i--)         	/* PRZESUNI�CIE PR�BEK */
		L_in[i]=L_in[i-1];

	return output;			/* ZWRACA PRZEFILTROWANA PR�BK� */
}

short FILTR_R (short input, short *h)
{
	int i;
	short output;
	int acc=0;
	int prod;
	R_in[0] = input;         	/* OD�WIE�A NAJNOWSZA PR�BK� */

	acc = 0;                      	/* ZERUJE AKUMULATOR */
	for (i=0; i<101; i++)         	/* PETLA WYKONA SIE 101 RAZY */
	{   
		prod = (h[i]*R_in[i]);  /* MNO�ENIE Q.15 */
		acc = acc + prod;       /* OD�WIE�ANIE 32-bit AKUMULATORA */
	}                             	
	output = (short) (acc>>15);    	/* PRZERABIANIE WYJSCIA DO WARTOSCI 16-bits. */

	for (i=100; i>0; i--)         	/* PRZESUNI�CIE PR�BEK */
		R_in[i]=R_in[i-1];

	return output;			/* ZWRACA PRZEFILTROWANA PR�BK� */
}
