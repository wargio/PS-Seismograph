/*
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 This program was created by Grazioli Giovanni Dante <wargio@libero.it>.
*/

#include <io/pad.h>
#include "NoRSX.h"
#include <time.h>
#include "BackGround_bin.h"

static int exitapp, xmbopen;

static inline void eventHandler(u64 status, u64 param, void * userdata)
{
	switch(status)
	{
		case SYSUTIL_EXIT_GAME:
			exitapp = 0;
			break;
		case SYSUTIL_MENU_OPEN:
			xmbopen = 1;
			break;
		case SYSUTIL_MENU_CLOSE:
			xmbopen = 0;
			break;
	}
}

msgType MSG_OK = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);

s32 main(s32 argc, const char* argv[])
{
	padInfo padinfo ;
	padData paddata ;
	sysUtilRegisterCallback(SYSUTIL_EVENT_SLOT0, eventHandler, NULL);

	ioPadInit(7);
	pngData png;
	u32 i;
	NoRSX *GFX = new NoRSX();
	Image IMG(GFX);
	Object OBJ(GFX);
	Font F(GFX);

	IMG.LoadPNG_Buf((void*)BackGround_bin, BackGround_bin_size, &png);
	exitapp = 1;
	int frame=0;

	u32 pixel[500][4];
	for(i = 0; i<4 ;i++)
		for(u32 k=0;k<500;k++)
			pixel[k][i]=0;

	i=1;
	unsigned int S_X=0,S_Y=0,S_Z=0,S_G=0;
	ioPadSetSensorMode(0,1);
	while(exitapp){
		static time_t starttime = 0;
		double fps = 0;
		if (starttime == 0) starttime = time (NULL);
		else fps = frame / difftime (time (NULL), starttime);
		ioPadGetInfo(&padinfo);
		if(padinfo.status[0]){
			
			ioPadGetData(0, &paddata);
			if(paddata.BTN_CROSS){
				exitapp = 0;
			}
			S_X = paddata.SENSOR_X;
			S_Y = paddata.SENSOR_Y;
			S_Z = paddata.SENSOR_Z;
			S_G = paddata.SENSOR_G;
		}
		IMG.DrawIMG(0,0,&png);
		char a[20];
		sprintf (a,"FPS %d", int(1*fps));
		F.Print (200,130,(char*)("WRITTEN BY DEROAD"),COLOR_ORANGE);
		F.Print (200,160,(char*)("PRESS X TO EXIT"),COLOR_RED);
		F.Print (200,100,a,COLOR_BLACK);
		F.Printf(200,450,COLOR_GREEN,"SENSOR X %u", S_X);
		F.Printf(200,550,COLOR_BLUE,"SENSOR Y %u", S_Y);
		F.Printf(200,650,COLOR_RED,"SENSOR Z %u", S_Z);
		F.Printf(200,750,COLOR_CYAN,"SENSOR G %u", S_G);
		S_X-=62;
		S_Y+=100-12;
		S_Z+=200-12;
		S_G+=300-12;

		pixel[0][0]=S_X;
		pixel[0][1]=S_Y;
		pixel[0][2]=S_Z;
		pixel[0][3]=S_G;

		for(u32 k=0;k<500;k++){
			OBJ.Pixel4(650+(k*3),pixel[k][0],COLOR_GREEN);
			OBJ.Pixel4(650+(k*3),pixel[k][1],COLOR_BLUE);
			OBJ.Pixel4(650+(k*3),pixel[k][2],COLOR_RED);
			OBJ.Pixel4(650+(k*3),pixel[k][3],COLOR_CYAN);
		}

		GFX->Flip();
		frame ++;
		sysUtilCheckCallback();
		for(u32 j=0;j<4 ;j++)
			for(u32 k=500;k>0;k--)
				pixel[k][j]=pixel[k-1][j];
	}
	GFX->NoRSX_Exit();
	ioPadEnd();
	return 0;
}

