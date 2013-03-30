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

#include <NoRSX.h>
#include <time.h>
#include <stdio.h>

#include <io/pad.h>

#include "BackGround_png.h"
#include "X_button_alpha_png.h"
#include "visitor_ttf.h"

s32 main(s32 argc, const char* argv[]){

	padData paddata;
	padInfo padinfo;

	ioPadInit(7);

	
	NoRSX *GFX = new NoRSX(RESOLUTION_AUTO, RESOLUTION_1920x1080); // RESOLUTION_720x480 | RESOLUTION_720x576 | RESOLUTION_1280x720 | RESOLUTION_1920x1080
	Bitmap BMap(GFX);
	Object OBJ (GFX);
	Image  IMG (GFX);

	NoRSX_Bitmap Precalculated_Layer;	
	NoRSX_Bitmap Precalculated_Layer2;
	pngData *png = new pngData;
	
	BMap.GenerateBitmap(&Precalculated_Layer);
	BMap.GenerateBitmap(&Precalculated_Layer2);
	
	Font F2(visitor_ttf, visitor_ttf_size, GFX);

	IMG.LoadPNG_Buf((void*)BackGround_png, BackGround_png_size, png);
	
	IMG.DrawIMGtoBitmap(0, 0, png, &Precalculated_Layer);
	
	IMG.LoadPNG_Buf((void*)X_button_alpha_png, X_button_alpha_png_size, png);
	IMG.AlphaDrawIMGtoBitmap(200, 160, png, &Precalculated_Layer);

	F2.PrintfToBitmap(200,130,&Precalculated_Layer,COLOR_ORANGE,"PS STRONG-MOTION SEISMOMETER");

	free(png);


	unsigned int S_X=0, S_Y=0, S_Z=0, S_G=0;
	ioPadSetSensorMode(0,1);
	int  t_show=4, n_pressed=0;
	bool c_show =false;
	unsigned int Max_X=0, Max_Y=0, Max_Z=0, Max_G=0;
	GFX->AppStart();
	short square = 4;
	
	u32 pixel[400][4];
	for(u32 k=0;k<400;k++){
		pixel[k][0]=S_X;
		pixel[k][1]=S_Y;
		pixel[k][2]=S_Z;
		pixel[k][3]=S_G;
	}
	
/*
	bool write = false;
	fstream data;
	
	int find_usb(){
	int usb_num;
	char path[15];
	s32 fd;
	for(usb_num=0;usb_num<10;usb_num++){
		sprintf(path,"/dev_usb%03d/",usb_num);
		if(sysLv2FsOpenDir(path,&fd) == 0){
			write = true;
			goto close_dir;
		}
	}
	close_dir:
*/
	
	u64 time_on = 0;
	u32 frame=10;
	time_t starttime = time(NULL);
	while(GFX->GetAppStatus()){
		static double fps = 0;
		BMap.DrawBitmap(&Precalculated_Layer2);
		BMap.DrawBitmap(&Precalculated_Layer);

		ioPadGetInfo(&padinfo);
		if(padinfo.status[0]){
			ioPadGetData(0, &paddata);
			if(paddata.BTN_CROSS){
				GFX->AppExit();
			}
			if(paddata.BTN_TRIANGLE && n_pressed>(fps/2)-1){
				square++;
				t_show=square%5;
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
			ioPadClearBuf(0);
		}
		
		F2.Printf(50,10,0,30,"FPS %.2f", fps);

		F2.Printf(200,100,COLOR_BLACK,"%02d:%02d:%02d %2.3f CENTI-SECS", time_on/3600, time_on/60, time_on%60, (float)(100.f/((fps==0) ? 1:fps)));
		if((t_show==0 || t_show == 4) && !c_show) F2.Printf(200,450,COLOR_GREEN,"X %.4fG", (double)512/(double)(S_X));	//512 is the middle
		if((t_show==1 || t_show == 4) && !c_show) F2.Printf(200,570,COLOR_BLUE, "Y %.4fG", (double)455/(double)(S_Y));	//455 is the middle
		if((t_show==2 || t_show == 4) && !c_show) F2.Printf(200,685,COLOR_RED,  "Z %.4fG", (double)494/(double)(S_Z));	//494 is the middle
		if((t_show==3 || t_show == 4) && !c_show) F2.Printf(200,810,COLOR_CYAN, "G %.4fG", (double)512/(double)(S_G));	//512 is the middle
		if(c_show) F2.Printf(200,450,COLOR_GREEN,"MAX X %.4fG", 1.*(double)(Max_X)/(double)512);	//512 is the middle
		if(c_show) F2.Printf(200,570,COLOR_BLUE, "MAX Y %.4fG", 1.*(double)(Max_Y)/(double)455);	//455 is the middle
		if(c_show) F2.Printf(200,685,COLOR_RED,  "MAX Z %.4fG", 1.*(double)(Max_Z)/(double)494);	//494 is the middle
		if(c_show) F2.Printf(200,810,COLOR_CYAN, "MAX G %.4fG", 1.*(double)(Max_G)/(double)512);	//512 is the middle
		
		S_X-=62;
		S_Y+=108;
		S_Z+=195;
		S_G+=300;

		pixel[0][0]=S_X;
		pixel[0][1]=S_Y;
		pixel[0][2]=S_Z;
		pixel[0][3]=S_G;
		for(u32 k=0;k<400 && !c_show;k++){
			if((t_show==0 || t_show == 4)) OBJ.Pixel4(530+(k*3),pixel[k][0],COLOR_GREEN);
			if((t_show==1 || t_show == 4)) OBJ.Pixel4(530+(k*3),pixel[k][1],COLOR_BLUE);
			if((t_show==2 || t_show == 4)) OBJ.Pixel4(530+(k*3),pixel[k][2],COLOR_RED);
			if((t_show==3 || t_show == 4)) OBJ.Pixel4(530+(k*3),pixel[k][3],COLOR_CYAN);
		}


		GFX->RescaleFlip();
		frame ++;
		
		for(u32 j=0;j<4;j++)
			for(u32 k=400;k>0;k--)
				pixel[k][j]=pixel[k-1][j];
		if(n_pressed<(fps/2))n_pressed++;
		
		time_on = difftime (time (NULL), starttime);
		fps = frame / ((time_on==0) ? 1:time_on);
		
	}

	BMap.ClearBitmap(&Precalculated_Layer);
	BMap.ClearBitmap(&Precalculated_Layer2);

	GFX->NoRSX_Exit();
	ioPadEnd();
	return 0;
}


