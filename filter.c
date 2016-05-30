#include "include/filter.h"

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
