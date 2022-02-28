#define _CRT_SECURE_NO_WARNINGS // Disables the warnings about scanf() and fopen() being unsecure.


#include "../api/aovspecan.h"
#include "../api/aov_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void pexit(int code)
{
	fprintf(stderr, "Program Failed. Code: %i\n",code);
	exit(0);
}


int main (int argc, char *argv[])
{
	FILE *fp;	
	//char *ip_addr = "192.168.118.243";
	char ip[128];
	char instr[128];
	char filename[128];
	int portno;
	int i;
	int iResult;
	int cmd;
	double cfreq, span, db;
	int rlvl, rbw, input, flags;
	sAOVwaveForm wave;	
	void *handle;
	AOVhandle *lhandle;// = (AOVhandle *)handle;
	int LNB_EN, LNB_18, LNB_22;
	sAOVlnb lnb;
	int marker, marker2,mode;
	double mfreq;
	unsigned int numDev;
	
	while (1)
	{
		fprintf(stderr,"Enter a command\n");
		fprintf(stderr,"\t1)  Initialize\t\tAVCOM_SA_Initialize()\n");
		fprintf(stderr,"\t2)  Deinitialize\tAVCOM_SA_Close()\n");
		fprintf(stderr,"\t3)  InitHandle\t\tAVCOM_SA_Register()\n");
		fprintf(stderr,"\t4)  KillHandle\t\tAVCOM_SA_Unregister()\n");
		fprintf(stderr,"\t5)  GetHandle\t\tAVCOM_SA_GetHandle()\n");
		fprintf(stderr,"\t6)  Open\t\tAVCOM_SA_Connect()\n");
		fprintf(stderr,"\t7)  Close\t\tAVCOM_SA_Disconnect()\n");
		fprintf(stderr,"\t8)  SetAll\t\tAVCOM_SA_SetAllSettings()\n");
		fprintf(stderr,"\t9)  GetWaveform\t\tAVCOM_SA_GetSpectrumData()\n");
		fprintf(stderr,"\t10) SetCF\t\tAVCOM_SA_SetCenterFrequency()\n");
		fprintf(stderr,"\t11) SetSpan\t\tAVCOM_SA_SetSpan()\n");
		fprintf(stderr,"\t12) SetRefLevel\t\tAVCOM_SA_SetRefLevel()\n");
		fprintf(stderr,"\t13) SetRBW\t\tAVCOM_SA_SetResBandwidth()\n");
		fprintf(stderr,"\t14) SetInput\t\tAVCOM_SA_SetInput()\n");
		fprintf(stderr,"\t15) SetLNB\t\tAVCOM_SA_SetLNB()\n");
		fprintf(stderr,"\t16) SetMarkerFreq\tAVCOM_SA_SetMarkerFreq()\n");
		fprintf(stderr,"\t17) SetMarkerOpt\tAVCOM_SA_SetMarkerOpt()\n");
		fprintf(stderr,"\t18) ClearMarker\t\tAVCOM_SA_ClearMarker()\n");
		fprintf(stderr,"\t19) GetMarker\t\tAVCOM_SA_GetMarker()\n");
		fprintf(stderr,"\t20) GetSNR\t\tAVCOM_SA_GetSNR()\n");
		fprintf(stderr,"\t21) SetAvearge\t\tAVCOM_SA_SetAverage()\n");
		fprintf(stderr,"\t22) SetSweepMode\tAVCOM_SA_SetSweepMode()\n");
		fprintf(stderr,"\t23) Trigger\t\tAVCOM_SA_Trigger()\n");
		fprintf(stderr,"\t24) GetCNR\t\tAVCOM_SA_GetCNR()\n");
		fprintf(stderr,"\t25) GetUSBnumDev\tAVCOM_SA_GetNumUSB()\n");

		fprintf(stderr,"\t99) Exit();\n");
		
		fprintf(stderr,"Your command? ");

		scanf("%i",&cmd);

		switch (cmd)
		{
		case 1: // AVCOM_SA_Initialize()
			iResult = AVCOM_SA_Initialize();
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 2: // AVCOM_SA_Close()
			iResult = AVCOM_SA_Deinitialize();
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 3: // AVCOM_SA_Register()
			fprintf(stderr,"Enter ip [192.168.118.243]: ");	
			fgets(ip,128,stdin);
			fgets(ip,128,stdin);
			if(ip[0] == 0x0A)
				memcpy(ip,"192.168.118.243",sizeof("192.168.118.243"));
			else {
				for (i=0; i < 256; i++)
				{
					if (ip[i] == 0x0A)
					{
						ip[i] = 0x00;
						break;
					}
				}
			}

			fprintf(stderr,"Enter port [26482]: ");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				portno = 26482;
			else
				portno = atoi(instr);

			fprintf(stderr,"Enter flags [0]: ");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				flags = 0;
			else
				flags = atoi(instr);
			iResult = AVCOM_SA_Register(&handle, ip, portno, 0, flags);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			lhandle = (AOVhandle *)handle;
			break;

		case 4: // AVCOM_SA_Unregister()
			iResult = AVCOM_SA_Unregister(handle);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 5: // AVCOM_SA_GetHandle()
			fprintf(stderr,"Enter ip [192.168.118.243]: ");	
			fgets(ip,128,stdin);
			fgets(ip,128,stdin);
			if(ip[0] == 0x0A)
				memcpy(ip,"192.168.118.243",sizeof("192.168.118.243"));
			else {
				for (i=0; i < 256; i++)
				{
					if (ip[i] == 0x0A)
					{
						ip[i] = 0x00;
						break;
					}
				}
			}

			fprintf(stderr,"Enter port [26482]: ");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				portno = 26482;
			else
				portno = atoi(instr);

			handle = AVCOM_SA_GetHandle(ip, portno);
			fprintf(stderr, "\n Result = 0x%08x\n\n",(int)handle);
			break;

		case 6: // AVCOM_SA_Connect()
			iResult = AVCOM_SA_Connect(handle);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 7: // AVCOM_SA_Disconnect()
			iResult = AVCOM_SA_Disconnect(handle);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 8: // AVCOM_SA_SetAllSettings()
			fprintf(stderr,"Enter Center Freq [1500]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				cfreq = 1500;
			else
				cfreq = atof(instr);

			fprintf(stderr,"Enter Span [50]: ");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				span = 50;
			else
				span = atof(instr);

			fprintf(stderr,"Enter Reference Level [-50]: ");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				rlvl = -50;
			else
				rlvl = atoi(instr);

			fprintf(stderr,"Enter RBW [0x10]: 0x");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				rbw = 0x10;
			else
				sscanf(instr, "%x", &rbw); 

			fprintf(stderr,"Enter RF Input [1]: ");
			fflush (stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				input = 1;
			else 
				input = atoi(instr);

			fprintf(stderr, "For easier input, LNB will be off");
			iResult = AVCOM_SA_SetAllSettings(handle,cfreq,span,rlvl,rbw,input,0);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 9:
			fprintf(stderr, "Enter filename [test.dat]: ");
			fgets(filename,128,stdin);
			fgets(filename,128,stdin);
			if(filename[0] == 0x0A)
				memcpy(filename,"test.dat",sizeof("test.dat"));
			iResult = AVCOM_SA_GetSpectrumData(handle, &wave);
			if (iResult)
			{
				fprintf(stderr, "\n Result = %i\n\n",iResult);
				break;
			}
			fprintf(stderr, "\n getWaveform Result = %i\n",iResult);

			if ((fp = fopen(filename,"w")) != AOV_ERR_NO_ERROR)
			{
				for (i=0; i<320; i++)
					fprintf(fp,"%.4f\t%.4f\n",wave.frequency[i],wave.amplitude[i]);
					
				fclose(fp);
			} else {
				fprintf(stderr,"Error fopen()\n");
			}
			fprintf(stderr,"\n");
			break;
		
		case 10: // AVCOM_SA_SetCenterFrequency()
			fprintf(stderr,"Enter Center Freq [1500]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				cfreq = 1500;
			else
				cfreq = atof(instr);

			iResult = AVCOM_SA_SetCenterFrequency(handle,cfreq);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;
		
		case 11: // AVCOM_SA_SetSpan()
			fprintf(stderr,"Enter Span [50]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				span = 50;
			else
				span = atof(instr);

			iResult = AVCOM_SA_SetSpan(handle,span);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;
		
		case 12: // AVCOM_SA_SetRefLevel()
			fprintf(stderr,"Enter Reference Level [-50]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				rlvl = -50;
			else
				rlvl = atoi(instr);

			iResult = AVCOM_SA_SetRefLevel(handle,rlvl);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;
		
		case 13: // AVCOM_SA_SetResBandwidth()
			fprintf(stderr,"Enter RBW [0x10]: 0x");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				rbw = 0x10;
			else
				sscanf(instr, "%x", &rbw); 

			iResult = AVCOM_SA_SetResBandwidth(handle,rbw);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;
		
		case 14: // AVCOM_SA_SetInput()
			fprintf(stderr,"Enter RF Input [1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				input = 1;
			else 
				input = atoi(instr); 

			fprintf(stderr,"Enter Input Flags [0]: ");
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				flags = 0;
			else 
				flags = atoi(instr); 

			iResult = AVCOM_SA_SetInput(handle,input,flags);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;
		
		case 15: // AVCOM_SA_SetLNB()
			fprintf(stderr,"LNB on/OFF [0]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				LNB_EN = 0;
			else 
				LNB_EN = atoi(instr); 

			if (LNB_EN)
			{
				fprintf(stderr,"13V/18V [0]: ");
				fflush (stdin);
				fgets(instr,128,stdin);
				if(instr[0] == 0x0A)
					LNB_18 = 0;
				else 
					LNB_18 = atoi(instr); 

				fprintf(stderr,"22KHz on/OFF [0]: ");
				fflush (stdin);
				fgets(instr,128,stdin);
				if(instr[0] == 0x0A) {
					LNB_22 = 0;
				} else {
					LNB_22 = atoi(instr); 
				}
			}


			iResult = AVCOM_SA_LNBstruct(&lnb,LNB_EN,LNB_18,LNB_22);
			iResult = AVCOM_SA_SetLNB(handle,&lnb);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 16: // AVCOM_SA_SetMarkerFreq()
			fprintf(stderr,"Enter Marker Number [1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = 1;
			else
				marker = atoi(instr); 

			fprintf(stderr,"Enter Freq [1500]: ");
				fflush (stdin);
				fgets(instr,128,stdin);
				if(instr[0] == 0x0A)
					mfreq = 1500;
				else 
					mfreq = atof(instr); 

			iResult = AVCOM_SA_SetMarkerFreq(handle,marker,mfreq);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 17: // AVCOM_SA_SetMarkerOpt()
			fprintf(stderr,"Enter Marker Number [1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = 1;
			else
				marker = atoi(instr); 

			fprintf(stderr,"Max=1, Min=2 [1]: ");
				fflush (stdin);
				fgets(instr,128,stdin);
				if(instr[0] == 0x0A)
					flags = 1;
				else 
					flags = atoi(instr); 

			iResult = AVCOM_SA_SetMarkerOpt(handle,marker,flags);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 18: // AVCOM_SA_ClearMarker()
			fprintf(stderr,"Enter Marker Number (-1 for all) [-1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = -1;
			else
				marker = atoi(instr); 

			iResult = AVCOM_SA_ClearMarker(handle,marker);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 19: // AVCOM_SA_GetMarker()
			fprintf(stderr,"Enter Marker Number [1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = 1;
			else
				marker = atoi(instr); 

			if ( (iResult = AVCOM_SA_GetMarker(handle,marker,&cfreq,&db))  != AOV_ERR_NO_ERROR)
			{
				fprintf(stderr, "\n Result = %i\n\n",iResult);
			} else {
				fprintf(stderr, "\n %.4fdB @ %.4fMHz\n\n",db,cfreq);
			}
			break;

		case 20: // AVCOM_SA_GetSNR()
			fprintf(stderr,"Enter Marker Number 1 [1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = 1;
			else
				marker = atoi(instr); 

			fprintf(stderr,"Enter Marker Number 2 [2]: ");
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker2 = 2;
			else
				marker2 = atoi(instr); 

			if ( (iResult = AVCOM_SA_GetSNR(handle,marker,marker2,&db)) != AOV_ERR_NO_ERROR)
			{
				fprintf(stderr, "\n Result = %i\n\n",iResult);
			} else {
				fprintf(stderr, "\n %.4fdB between Markers %i and %i\n\n",db,marker,marker2);
			}
			break;

		case 21: // AVCOM_SA_SetAverage()
			fprintf(stderr,"Enter Avearge [0]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = 0;
			else
				marker = atoi(instr); 

			iResult = AVCOM_SA_SetAverage(handle,marker);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 22: // AVCOM_SA_SetSweepMode()
			fprintf(stderr,"Enter Sweep Mode [0]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				mode = 0;
			else
				mode = atoi(instr); 

			iResult = AVCOM_SA_SetSweepMode(handle,mode);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 23: // AVCOM_SA_Trigger()
			//fgets(instr,128,stdin);
			//fgets(instr,128,stdin);
			//if(instr[0] == 0x0A)
			//	mode = 0;
			//else
			//	mode = atoi(instr); 

			iResult = AVCOM_SA_Trigger(handle,0);
			fprintf(stderr, "\n Result = %i\n\n",iResult);
			break;

		case 24: // AVCOM_SA_GetCNR()
			fprintf(stderr,"Enter Marker Number [1]: ");
			fgets(instr,128,stdin);
			fgets(instr,128,stdin);
			if(instr[0] == 0x0A)
				marker = 1;
			else
				marker = atoi(instr); 

			if ( (iResult = AVCOM_SA_GetCNR(handle,marker,&db)) != AOV_ERR_NO_ERROR)
			{
				fprintf(stderr, "\n Result = %i\n\n",iResult);
			} else {
				fprintf(stderr, "\n %.4fdB CNR for Marker %i\n\n",db,marker);
			}
			break;
/*
		case 25: //AVCOM_SA_GetNumUSB()
			if ( (iResult = AVCOM_SA_GetNumUSB ( &numDev )) != AOV_ERR_NO_ERROR)
			{
				fprintf(stderr,"\n Result = %i\n\n", iResult);
			} else {
				fprintf(stdout,"\n Number of USB Devices Connected: %d\n\n",numDev); 
			}
			break;
*/

		case 99:
			fprintf(stderr,"Exiting Program\n");
			fprintf(stderr,"forcing AOVdeinitialize()\n");
			iResult = AVCOM_SA_Deinitialize();
			return 0;

		default:
			break;

		}
	}

}
