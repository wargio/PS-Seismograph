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
#include "BackGround_png.h"
#include "X_button_alpha_png.h"

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
	pngData png,button;
	u32 i;
	NoRSX *GFX = new NoRSX();
	Image IMG(GFX);
	Object OBJ(GFX);
	Font F(GFX);

	IMG.LoadPNG_Buf((void*)BackGround_png, BackGround_png_size, &png);
	IMG.LoadPNG_Buf((void*)X_button_alpha_png, X_button_alpha_png_size, &button);
	exitapp = 1;
	int frame=0;

	u32 pixel[500][4];
	for(i = 0; i<4 ;i++)
		for(u32 k=0;k<500;k++)
			pixel[k][i]=0;

	i=1;
	unsigned int S_X=0,S_Y=0,S_Z=0,S_G=0;
	ioPadSetSensorMode(0,1);
	int t_show=4,n_pressed=0;
	bool c_show =false;
	unsigned int Max_X=0, Max_Y=0, Max_Z=0, Max_G=0;
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
			if(paddata.BTN_TRIANGLE && n_pressed>(fps/2)-1){
				t_show++;
				if(t_show>4)t_show=0;
				n_pressed=0;
			}
			if(paddata.BTN_SQUARE && n_pressed>(fps/2)-1){
				if(!c_show)c_show=true;
				else c_show=false;
				n_pressed=0;
			}
			S_X = paddata.SENSOR_X;
			S_Y = paddata.SENSOR_Y;
			S_Z = paddata.SENSOR_Z;
			S_G = paddata.SENSOR_G;
			if(Max_X<S_X)Max_X=S_X;
			if(Max_Y<S_Y)Max_Y=S_Y;
			if(Max_Z<S_Z)Max_Z=S_Z;
			if(Max_G<S_G)Max_G=S_G;
		}
		IMG.DrawIMG(0,0,&png);
		char a[20];
		sprintf (a,"%.3f CENTI-SECS", (double)(100/fps));
		F.Print (200,130,(char*)("PS STRONG-MOTION SEISMOMETER"),COLOR_ORANGE);
		IMG.AlphaDrawIMG(200,160,&button);
//		F.Print (200,160,(char*)("PRESS X TO EXIT"),COLOR_RED);
		F.Print (200,100,a,COLOR_BLACK);
		if((t_show==0 || t_show == 4) && !c_show) F.Printf(200,450,COLOR_GREEN,"X %.4fG", (double)512/(double)(S_X));	//512 is the middle
		if((t_show==1 || t_show == 4) && !c_show) F.Printf(200,550,COLOR_BLUE, "Y %.4fG", (double)455/(double)(S_Y));	//455 is the middle
		if((t_show==2 || t_show == 4) && !c_show) F.Printf(200,650,COLOR_RED,  "Z %.4fG", (double)494/(double)(S_Z));	//494 is the middle
		if((t_show==3 || t_show == 4) && !c_show) F.Printf(200,750,COLOR_CYAN, "G %.4fG", (double)512/(double)(S_G));	//512 is the middle
		if(c_show) F.Printf(400,450,COLOR_GREEN,"MAX X %.4fG", 1.*(double)(Max_X)/(double)512);	//512 is the middle
		if(c_show) F.Printf(400,550,COLOR_BLUE, "MAX Y %.4fG", 1.*(double)(Max_Y)/(double)455);	//455 is the middle
		if(c_show) F.Printf(400,650,COLOR_RED,  "MAX Z %.4fG", 1.*(double)(Max_Z)/(double)494);	//494 is the middle
		if(c_show) F.Printf(400,750,COLOR_CYAN, "MAX G %.4fG", 1.*(double)(Max_G)/(double)512);	//512 is the middle
		S_X-=62;
		S_Y+=100-12;
		S_Z+=200-12;
		S_G+=300-12;

		pixel[0][0]=S_X;
		pixel[0][1]=S_Y;
		pixel[0][2]=S_Z;
		pixel[0][3]=S_G;
		for(u32 k=0;k<500;k++){
			if((t_show==0 || t_show == 4) && !c_show) OBJ.Pixel4(650+(k*3),pixel[k][0],COLOR_GREEN);
			if((t_show==1 || t_show == 4) && !c_show) OBJ.Pixel4(650+(k*3),pixel[k][1],COLOR_BLUE);
			if((t_show==2 || t_show == 4) && !c_show) OBJ.Pixel4(650+(k*3),pixel[k][2],COLOR_RED);
			if((t_show==3 || t_show == 4) && !c_show) OBJ.Pixel4(650+(k*3),pixel[k][3],COLOR_CYAN);
		}

		GFX->Flip();
		frame ++;
		sysUtilCheckCallback();
		for(u32 j=0;j<4 ;j++)
			for(u32 k=500;k>0;k--)
				pixel[k][j]=pixel[k-1][j];

		if(n_pressed<(fps/2))n_pressed++;
	}
	GFX->NoRSX_Exit();
	ioPadEnd();
	return 0;
}

