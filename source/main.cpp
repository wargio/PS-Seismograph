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
#include <fstream>

#include <io/pad.h>
#include <sys/file.h>
#include <sys/systime.h>
#include <libmove/libmove.h>

using namespace std;


#include "BackGround_png.h"
#include "buttons_png.h"
#include "buttons_plus_move_png.h"
#include "controller_options_png.h"
#include "visitor_ttf.h"
#include "warning_png.h"
#include "calibrate_png.h"

msgType MSG_OK = (msgType)(MSG_DIALOG_NORMAL | MSG_DIALOG_BTN_TYPE_OK | MSG_DIALOG_DISABLE_CANCEL_ON);

int file_exist(const char* file){
	FILE *f = NULL;
	f = fopen(file,"rb");
	if(f == NULL){
		fclose(f);
		return 0;
	}else{
		fclose(f);
		return 1;
	}
}

s32 main(s32 argc, const char* argv[]){

	padData paddata;
	padInfo padinfo;
	movePadData data;

	ioPadInit(7);
	int move = initLibMove();
	
	NoRSX *GFX = new NoRSX(RESOLUTION_AUTO, RESOLUTION_1920x1080); // RESOLUTION_720x480 | RESOLUTION_720x576 | RESOLUTION_1280x720 | RESOLUTION_1920x1080
	Bitmap BMap(GFX);
	Object OBJ (GFX);
	Image  IMG (GFX);
	MsgDialog Msg(GFX);


	
	bool write = false;
	bool usb_found = false;
	bool c_show =false;

	u16 CalibX, CalibY;
	s32 fd;
	u32 frame=1;
	u64 sec, nsec;
	u64 time_on = 0;

	time_t starttime = 0;	

	char path[256];
	char data_char[1024];
	char my_time[128];
	double fps = 1;
	
	short square = 4;
	int usb_num = 0;
	int my_data = 0;
	int  t_show=4, n_pressed=0;
	float S_X_HALF = 512.f, S_Y_HALF = 455.f, S_Z_HALF = 494.f, S_G_HALF = 512.f;
	float S_X=0, S_Y=0, S_Z=0, S_G=0;
	float Max_X=0, Max_Y=0, Max_Z=0, Max_G=0;

	fstream write_data;
	
	
	for(usb_num=0;usb_num<10;usb_num++){
		sprintf(path,"/dev_usb%03d/",usb_num);
		if(sysLv2FsOpenDir(path,&fd) == 0){
			usb_found = true;
			goto close_dir;
		}
	}
	close_dir:


	NoRSX_Bitmap Precalculated_Layer;	
	NoRSX_Bitmap Precalculated_Layer2;
	pngData *png = new pngData;
	
	BMap.GenerateBitmap(&Precalculated_Layer);
	BMap.GenerateBitmap(&Precalculated_Layer2);
	
	Font F2(visitor_ttf, visitor_ttf_size, GFX);

	IMG.LoadPNG_Buf((void*)BackGround_png, BackGround_png_size, png);
	
	IMG.DrawIMGtoBitmap(0, 0, png, &Precalculated_Layer);
	IMG.DrawIMGtoBitmap(0, 0, png, &Precalculated_Layer2);
	free(png);
	png = new pngData;
	if(move == LIBMOVE_OK)
		IMG.LoadPNG_Buf((void*)buttons_plus_move_png, buttons_plus_move_png_size, png);
	else
		IMG.LoadPNG_Buf((void*)buttons_png, buttons_png_size, png);
	IMG.AlphaDrawIMGtoBitmap(200, 160, png, &Precalculated_Layer);
	free(png);
	png = new pngData;

	F2.PrintfToBitmap(200,130,&Precalculated_Layer,COLOR_ORANGE,"PS STRONG-MOTION SEISMOMETER  USB %s", (usb_found) ? "FOUND" : "NOT FOUND");

	IMG.LoadPNG_Buf((void*)warning_png, warning_png_size, png);
	CalibX = (GFX->width - png->width)/2;
	CalibY = (GFX->height - png->height)/2;
	
	IMG.DrawIMGtoBitmap(CalibX, CalibY, png, &Precalculated_Layer2);

	free(png);
	png = new pngData;
	if(move == LIBMOVE_OK)
		IMG.LoadPNG_Buf((void*)calibrate_png, calibrate_png_size, png);
	else
		IMG.LoadPNG_Buf((void*)controller_options_png, controller_options_png_size, png);
	
	ioPadSetSensorMode(0,1);
	
	GFX->AppStart();
	
	u32 pixel[400][4];
	for(u32 k=0;k<400;k++){
		pixel[k][0] = 454;
		pixel[k][1] = 574;
		pixel[k][2] = 688;
		pixel[k][3] = 814;
	}
	
	Msg.TimerDialog(MSG_OK, "PS Seismograph\n"
	                        "Written By Deroad\n\n"
	                        "http://devram0.blogspot.com/", 5000.f); // 5 secs
	
	if(move == LIBMOVE_OK){
		starttime = time(NULL);
		while(1){
			BMap.DrawBitmap(&Precalculated_Layer2);
			IMG.DrawIMG(CalibX,CalibY,png);
			getMovePadData(PLAYSTATION_MOVE_PAD_0, &data);
			
			if(data.BTN_ACTION && n_pressed>13){
				n_pressed=0;
				goto move_init;
			}			
			if(data.BTN_CROSS && n_pressed>13){
				n_pressed=0;
				goto move_init;
			}
			ioPadGetInfo(&padinfo);
			if(padinfo.status[0]){
				ioPadGetData(0, &paddata);
				if(paddata.BTN_CROSS && n_pressed>13){
					n_pressed=0;
				goto move_init;
				}
			}
			if(n_pressed<14)
				n_pressed++;
			GFX->RescaleFlip();
			time_on = difftime (time (NULL), starttime);
			fps = frame / ((time_on==0) ? 1:time_on);
			
			if(GFX->ExitSignalStatus()) goto end_all;
			
		}
	move_init:
		calibrateMove(PLAYSTATION_MOVE_PAD_0);
		getGyroPosition(PLAYSTATION_MOVE_PAD_0, &S_X_HALF, &S_Y_HALF, &S_Z_HALF);
		sleep(1);
		starttime = time(NULL);
		frame = 0;
		t_show = 3;
		square = 3;
		while(GFX->GetAppStatus()){
			if(GFX->ExitSignalStatus()) goto end_all;
			sysGetCurrentTime(&sec,&nsec);
			BMap.DrawBitmap(&Precalculated_Layer2);
			BMap.DrawBitmap(&Precalculated_Layer);
			
			getMovePadData(PLAYSTATION_MOVE_PAD_0, &data);
			ioPadGetInfo(&padinfo);
			if(padinfo.status[0]){
				ioPadGetData(0, &paddata);
				if(paddata.BTN_CROSS && n_pressed>13){
					GFX->AppExit();
				}
				
				if(paddata.BTN_TRIANGLE && n_pressed>13){
					square++;
					t_show=square%4;
					n_pressed=0;
				}
				if(paddata.BTN_SQUARE && n_pressed>13){
					if(!c_show) c_show=true;
					else c_show=false;
					n_pressed=0;
				}
				if(paddata.BTN_CIRCLE && n_pressed>13 && usb_found){
					if(write){
						write = false;
						write_data.close();
					}else{
						write = true;
						do{
							sprintf(path,"/dev_usb%03d/seismometer_%03d.data",usb_num,my_data);
							my_data++;
						}while(file_exist(path) && my_data<1000);
						if(my_data>999){
							usb_found = false;
							write = false;
						}else{
							write_data.open(path,ios::out);
							sprintf(data_char,"Data       Time     Nanosecs  SensorX  SensorY  SensorZ  SensorG\n");
							write_data << data_char;
						}
					}
					n_pressed=0;
				}
				ioPadClearBuf(0);
			}
			if(data.BTN_CROSS && n_pressed>13){
				GFX->AppExit();
			}
			if(data.BTN_TRIANGLE && n_pressed>13){
				square++;
				t_show=square%4;
				n_pressed=0;
			}
			if(data.BTN_SQUARE && n_pressed>13){
				if(!c_show)c_show=true;
				else c_show=false;
				n_pressed=0;
			}
			if(data.BTN_ACTION && n_pressed>13){
				calibrateMove(PLAYSTATION_MOVE_PAD_0);
				getGyroPosition(PLAYSTATION_MOVE_PAD_0, &S_X_HALF, &S_Y_HALF, &S_Z_HALF);
				n_pressed=0;
			}
			if(data.BTN_CIRCLE && n_pressed>13 && usb_found){
				if(write){
					write = false;
					write_data.close();
				}else{
					write = true;
					do{
						sprintf(path,"/dev_usb%03d/seismometer_%03d.data",usb_num,my_data);
						my_data++;
					}while(file_exist(path) && my_data<1000);
					if(my_data>999){
						usb_found = false;
						write = false;
					}else{
						write_data.open(path,ios::out);
						sprintf(data_char,"Data       Time     Nanosecs  SensorX  SensorY  SensorZ  SensorG\n");
						write_data << data_char;
					}
				}
				n_pressed=0;
			}
			getGyroPosition(PLAYSTATION_MOVE_PAD_0, &S_X, &S_Y, &S_Z);
			if(Max_X<abs(S_X))Max_X=abs(S_X);
			if(Max_Y<abs(S_Y))Max_Y=abs(S_Y);
			if(Max_Z<abs(S_Z))Max_Z=abs(S_Z);
		//	if(Max_G<abs(S_G))Max_G=abs(S_G);

			if(write){
				strftime (my_time, sizeof(my_time), "%Y/%m/%d %H:%M:%S", gmtime ((const time_t*)&sec));
				sprintf(data_char,"%s %9lu %f %f %f %f\n",my_time,nsec,S_X_HALF/S_X,S_Y_HALF/S_Y,S_Z_HALF/S_Z,0.f);
				write_data << data_char;
			}

			F2.Printf(200,100,COLOR_BLACK,"%02d:%02d:%02d %2.3f SECS %s",
				  time_on/3600, time_on/60, time_on%60, (float)(1.f/((fps==0) ? 1:fps)), (write)? "WRITING ON USB": " ");
			
			if((t_show==0 || t_show == 3) && !c_show)
				F2.Printf(200,450,COLOR_GREEN,"X %.4fG", S_X_HALF/S_X);	//512 is the middle
			if((t_show==1 || t_show == 3) && !c_show)
				F2.Printf(200,570,COLOR_BLUE, "Y %.4fG", S_Y_HALF/S_Y);	//455 is the middle
			if((t_show==2 || t_show == 3) && !c_show)
				F2.Printf(200,685,COLOR_RED,  "Z %.4fG", S_Z_HALF/S_Z);	//494 is the middle

		//	if((t_show==3 || t_show == 3) && !c_show)
		//		F2.Printf(200,810,COLOR_CYAN, "G %.4fG", S_G_HALF/S_G);	//512 is the middle
				
			if(c_show)
				F2.Printf(200,450,COLOR_GREEN,"MAX X %.4fG", 1.f*(float)(Max_X)/S_X_HALF);	//512 is the middle
			if(c_show)
				F2.Printf(200,570,COLOR_BLUE, "MAX Y %.4fG", 1.f*(float)(Max_Y)/S_Y_HALF);	//455 is the middle
			if(c_show)
				F2.Printf(200,685,COLOR_RED,  "MAX Z %.4fG", 1.f*(float)(Max_Z)/S_Z_HALF);	//494 is the middle
		//	if(c_show)
		//		F2.Printf(200,810,COLOR_CYAN, "MAX G %.4fG", 1.f*(float)(Max_G)/S_G_HALF);	//512 is the middle
		
			S_X+=454;
			S_Y+=574;
			S_Z+=688;
		//	S_G+=814;

			pixel[0][0]=S_X;
			pixel[0][1]=S_Y;
			pixel[0][2]=S_Z;
		//	pixel[0][3]=S_G;
			for(u32 k=0;k<400 && !c_show;k++){
				if((t_show==0 || t_show == 3)) OBJ.Pixel4(530+(k*3),pixel[k][0],COLOR_GREEN);
				if((t_show==1 || t_show == 3)) OBJ.Pixel4(530+(k*3),pixel[k][1],COLOR_BLUE);
				if((t_show==2 || t_show == 3)) OBJ.Pixel4(530+(k*3),pixel[k][2],COLOR_RED);
		//		if((t_show==3 || t_show == 3)) OBJ.Pixel4(530+(k*3),pixel[k][3],COLOR_CYAN);
			}


			GFX->RescaleFlip();
			frame ++;
		
			for(u32 j=0;j<3;j++)
				for(u32 k=400;k>0;k--)
					pixel[k][j]=pixel[k-1][j];
			if(n_pressed<14)
				n_pressed++;
		
			time_on = difftime (time (NULL), starttime);
			fps = frame / ((time_on==0) ? 1:time_on);
		
		}
	}else{
		starttime = time(NULL);
		while(1){
			BMap.DrawBitmap(&Precalculated_Layer2);

			ioPadGetInfo(&padinfo);
			if(padinfo.status[0]){
				ioPadGetData(0, &paddata);
				if(paddata.BTN_CROSS && n_pressed>13){
					n_pressed=0;
					goto draw_howto;
				}
				ioPadClearBuf(0);
			}
			if(n_pressed<14)
				n_pressed++;
			GFX->RescaleFlip();
			time_on = difftime (time (NULL), starttime);
			fps = frame / ((time_on==0) ? 1:time_on);
			if(GFX->ExitSignalStatus()) goto end_all;
		}
	draw_howto:
		sleep(1);
		starttime = time(NULL);
		frame = 0;
		while(1){
			BMap.DrawBitmap(&Precalculated_Layer2);
			IMG.DrawIMG(CalibX,CalibY,png);
			ioPadGetInfo(&padinfo);
			if(padinfo.status[0]){
				ioPadGetData(0, &paddata);
				if(paddata.BTN_CROSS && n_pressed>13){
					n_pressed=0;
					goto pad_init;
				}
				ioPadClearBuf(0);
			}
			if(n_pressed<14)
				n_pressed++;
			GFX->RescaleFlip();
			time_on = difftime (time (NULL), starttime);
			fps = frame / ((time_on==0) ? 1:time_on);
			if(GFX->ExitSignalStatus()) goto end_all;
		}
			
	pad_init:
		sleep(1);
		starttime = time(NULL);
		frame = 0;
		while(GFX->GetAppStatus()){
			if(GFX->ExitSignalStatus()) goto end_all;
			sysGetCurrentTime(&sec,&nsec);
			BMap.DrawBitmap(&Precalculated_Layer2);
			BMap.DrawBitmap(&Precalculated_Layer);

			ioPadGetInfo(&padinfo);
			if(padinfo.status[0]){
				ioPadGetData(0, &paddata);
				if(paddata.BTN_CROSS && n_pressed>13){
					GFX->AppExit();
				}
				if(paddata.BTN_TRIANGLE && n_pressed>13){
					square++;
					t_show=square%5;
					n_pressed=0;
				}
				if(paddata.BTN_SQUARE && n_pressed>13){
					if(!c_show)c_show=true;
					else c_show=false;
					n_pressed=0;
				}
				if(paddata.BTN_CIRCLE && n_pressed>13 && usb_found){
					if(write){
						write = false;
						write_data.close();
					}else{
						write = true;
						do{
							sprintf(path,"/dev_usb%03d/seismometer_%03d.data",usb_num,my_data);
							my_data++;
						}while(file_exist(path) && my_data<1000);
						if(my_data>999){
							usb_found = false;
							write = false;
						}else{
							write_data.open(path,ios::out);
							sprintf(data_char,"Data       Time     Nanosecs  SensorX  SensorY  SensorZ  SensorG\n");
							write_data << data_char;
						}
					}
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

			if(write){
				strftime (my_time, sizeof(my_time), "%Y/%m/%d %H:%M:%S", gmtime ((const time_t*)&sec));
				sprintf(data_char,"%s %9lu %f %f %f %f\n",my_time,nsec,S_X_HALF/S_X,S_Y_HALF/S_Y,S_Z_HALF/S_Z,S_G_HALF/S_G);
				write_data << data_char; 
			}
		
			F2.Printf(200,100,COLOR_BLACK,"%02d:%02d:%02d %2.3f SECS %s",
				  time_on/3600, time_on/60, time_on%60, (float)(1.f/((fps==0) ? 1:fps)), (write)? "WRITING ON USB": " ");
			if((t_show==0 || t_show == 4) && !c_show) 
				F2.Printf(200,450,COLOR_GREEN,"X %.4fG", S_X_HALF/S_X);	//512 is the middle
			if((t_show==1 || t_show == 4) && !c_show)
				F2.Printf(200,570,COLOR_BLUE, "Y %.4fG", S_Y_HALF/S_Y);	//455 is the middle
			if((t_show==2 || t_show == 4) && !c_show) 
				F2.Printf(200,685,COLOR_RED,  "Z %.4fG", S_Z_HALF/S_Z);	//494 is the middle
			if((t_show==3 || t_show == 4) && !c_show)
				F2.Printf(200,810,COLOR_CYAN, "G %.4fG", S_G_HALF/S_G);	//512 is the middle
				
			if(c_show)
				F2.Printf(200,450,COLOR_GREEN,"MAX X %.4fG", 1.f*(float)(Max_X)/S_X_HALF);	//512 is the middle
			if(c_show)
				F2.Printf(200,570,COLOR_BLUE, "MAX Y %.4fG", 1.f*(float)(Max_Y)/S_Y_HALF);	//455 is the middle
			if(c_show)
				F2.Printf(200,685,COLOR_RED,  "MAX Z %.4fG", 1.f*(float)(Max_Z)/S_Z_HALF);	//494 is the middle
			if(c_show)
				F2.Printf(200,810,COLOR_CYAN, "MAX G %.4fG", 1.f*(float)(Max_G)/S_G_HALF);	//512 is the middle
		
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
			if(n_pressed<14)n_pressed++;
		
			time_on = difftime (time (NULL), starttime);
			fps = frame / ((time_on==0) ? 1:time_on);
		
		}
	}
end_all:	
	if(write)
		write_data.close(); 

	if(move == LIBMOVE_OK)
		endLibMove();

	free(png);
	BMap.ClearBitmap(&Precalculated_Layer);
	BMap.ClearBitmap(&Precalculated_Layer2);

	GFX->NoRSX_Exit();
	ioPadEnd();
	return 0;
}


