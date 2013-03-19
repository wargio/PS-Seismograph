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
#include <NoRSX.h>
#include <time.h>
#include "BackGround_png.h"
#include "X_button_alpha_png.h"
#include "visitor_ttf.h"

void ScaleLine(u32 *Target, u32 *Source, u32 SrcWidth, u32 TgtWidth){
 //Thanks to: http://www.compuphase.com/graphic/scale.htm
	int NumPixels = TgtWidth;
	int IntPart = SrcWidth / TgtWidth;
	int FractPart = SrcWidth % TgtWidth;
	int E = 0;

	while (NumPixels-- > 0) {
		*Target++ = *Source;
		Source += IntPart;
		E += FractPart;
		if (E >= (int)TgtWidth) {
			E -= TgtWidth;
			Source++;
		} /* if */
	} /* while */
}

void ResizeBuffer(u32 *in, u32 *out, u32 TgtWidth, u32 TgtHeight){
	u32 *Source = in;
	u32 *Target = out;
	if(TgtWidth == 1920 && TgtHeight == 1080){
		memcpy(Target, Source, TgtHeight*TgtWidth*sizeof(u32));
		return;
	}
	int NumPixels = TgtHeight;
	int IntPart = (1080u / TgtHeight) * 1920u;
	int FractPart = 1080u % TgtHeight;
	int E = 0;
	u32 *PrevSource = NULL;

	while (NumPixels-- > 0) {
		if (Source == PrevSource) {
			memcpy(Target, Target-TgtWidth, TgtWidth*sizeof(*Target));
		} else {
			ScaleLine(Target, Source, 1920u, TgtWidth);
			PrevSource = Source;
		}
		Target += TgtWidth;
		Source += IntPart;
		E += FractPart;
		if (E >= (int)TgtHeight) {
			E -= TgtHeight;
			Source += 1920u;
		}
	}
}

s32 main(s32 argc, const char* argv[]){
	
	NoRSX_Bitmap bmap;
	
	padInfo padinfo;
	padData paddata;

	ioPadInit(7);
	pngData png,button;
	u32 i;
	NoRSX *GFX = new NoRSX();
	u32* buffer  = makeMemBuffer(1920,1080,&i);
	u32* old_buf = GFX->buffer;
	GFX->height = 1080;
	GFX->width  = 1920;

	Image	IMG(GFX);
	Object	OBJ(GFX);
	Font 	F  (visitor_ttf, visitor_ttf_size, GFX);
	Bitmap	BMap(GFX);


	IMG.LoadPNG_Buf((void*)BackGround_png, BackGround_png_size, &png);
	IMG.LoadPNG_Buf((void*)X_button_alpha_png, X_button_alpha_png_size, &button);
	BMap.GenerateBitmap(&bmap);
	
	IMG.DrawIMGtoBitmap(0, 0, &png, &bmap);
	
	F.PrintfToBitmap(200,130,&bmap,COLOR_ORANGE,"PS STRONG-MOTION SEISMOMETER");
	IMG.AlphaDrawIMGtoBitmap(200, 160, &button, &bmap);
	
	int frame=0;

	u32 pixel[500][4];
	for(i = 0; i<4;i++)
		for(u32 k=0;k<500;k++)
			pixel[k][i]=0;

	i=1;
	unsigned int S_X=0,S_Y=0,S_Z=0,S_G=0;
	ioPadSetSensorMode(0,1);
	int t_show=4,n_pressed=0;
	bool c_show =false;
	unsigned int Max_X=0, Max_Y=0, Max_Z=0, Max_G=0;
	GFX->buffer = buffer;
	GFX->AppStart();
	short square = 4;
	while(GFX->GetAppStatus()){
		static time_t starttime = 0;
		double fps = 0;
		if (starttime == 0) starttime = time (NULL);
		else fps = frame / difftime (time (NULL), starttime);
		
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
		}
		BMap.DrawBitmap(&bmap);
		F.Printf(200,100,COLOR_BLACK,"%.3f CENTI-SECS", (float)(100.f/fps));
		if((t_show==0 || t_show == 4) && !c_show) F.Printf(200,450,COLOR_GREEN,"X %.4fG", (double)512/(double)(S_X));	//512 is the middle
		if((t_show==1 || t_show == 4) && !c_show) F.Printf(200,570,COLOR_BLUE, "Y %.4fG", (double)455/(double)(S_Y));	//455 is the middle
		if((t_show==2 || t_show == 4) && !c_show) F.Printf(200,685,COLOR_RED,  "Z %.4fG", (double)494/(double)(S_Z));	//494 is the middle
		if((t_show==3 || t_show == 4) && !c_show) F.Printf(200,810,COLOR_CYAN, "G %.4fG", (double)512/(double)(S_G));	//512 is the middle
		if(c_show) F.Printf(200,450,COLOR_GREEN,"MAX X %.4fG", 1.*(double)(Max_X)/(double)512);	//512 is the middle
		if(c_show) F.Printf(200,570,COLOR_BLUE, "MAX Y %.4fG", 1.*(double)(Max_Y)/(double)455);	//455 is the middle
		if(c_show) F.Printf(200,685,COLOR_RED,  "MAX Z %.4fG", 1.*(double)(Max_Z)/(double)494);	//494 is the middle
		if(c_show) F.Printf(200,810,COLOR_CYAN, "MAX G %.4fG", 1.*(double)(Max_G)/(double)512);	//512 is the middle
		S_X-=62;
		S_Y+=108;
		S_Z+=195;
		S_G+=300;

		pixel[0][0]=S_X;
		pixel[0][1]=S_Y;
		pixel[0][2]=S_Z;
		pixel[0][3]=S_G;
		for(u32 k=0;k<500 && !c_show;k++){
			if((t_show==0 || t_show == 4)) OBJ.Pixel4(550+(k*3),pixel[k][0],COLOR_GREEN);
			if((t_show==1 || t_show == 4)) OBJ.Pixel4(560+(k*3),pixel[k][1],COLOR_BLUE);
			if((t_show==2 || t_show == 4)) OBJ.Pixel4(550+(k*3),pixel[k][2],COLOR_RED);
			if((t_show==3 || t_show == 4)) OBJ.Pixel4(550+(k*3),pixel[k][3],COLOR_CYAN);
		}
		
		GFX->buffer = old_buf;
		ResizeBuffer(buffer, GFX->buffer ,GFX->buffers[0].width,GFX->buffers[0].height);
		GFX->Flip();
		frame ++;
		for(u32 j=0;j<4;j++)
			for(u32 k=500;k>0;k--)
				pixel[k][j]=pixel[k-1][j];

		if(n_pressed<(fps/2))n_pressed++;
		GFX->buffer = buffer;
	}
	BMap.ClearBitmap(&bmap);
	GFX->NoRSX_Exit();
	ioPadEnd();
	return 0;
}

