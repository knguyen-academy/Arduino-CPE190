//Created by Khoi Nguyen
//Date : 03-10-2017
#include <stdio.h>
#include "simpletools.h" // Include simple tools
#include "fdserial.h"
#include "adcDCpropab.h" // Include adcDCpropab

int DO = 22, CLK = 23, DI = 24, CS = 25;

int main() // Main function
{
	//Bluetooth variables
	fdserial *BTSetup;
	//SDcard
	FILE* fp[5];
	//FILE* fp2;
	//FILE* fp3;
	//FILE* fp4;
	char buff[60];	//buffer 
	
	//Bluetooth Open
	// fdserial * fdserial_open(int rxpin, int txpin, int mode, int baudrate)
	//BTSetup = fdserial_open(15, 14, 0, 115200);		//Use this for Serial
	BTSetup = fdserial_open(31, 30, 0, 115200);			//Use this for Bluetooth 
	
	//Mount SD Card
	sd_mount(DO, CLK, DI, CS); // Mount SD card

	//Other Variables
	//Time Index
	float voltage[4]; 			// Voltage array
	int time_index = 0 ;		// time_index for graph
	int pressure_data = 5;	//test
	
	while(1)
	{
	
	high(26);
	int pin12 = input(12);		//Use this button for send data
	
	pressure_data = 5;	//test
	low(27);

	//Open 4 files to write SDCard
	fp[1] = fopen("test0.txt", "a");
	fp[2] = fopen("test1.txt", "a");
	fp[3] = fopen("test2.txt", "a");
	fp[4] = fopen("test3.txt", "a");
  	
	 //Check if NULL
	if(fp[1] == NULL || fp[2] == NULL || fp[3] == NULL || fp[4] == NULL) 	
	{
		printf("Couldn't open file\n");
		return -1;
	}
	//dprint(BTSetup,"here"); 
	
 //If files ready to write
	
  for ( int j = 0 ; j < 4 ; j++)
		{
			voltage[j] = adc_volts(j); // read analog pins 0-3, 5v across 4096
			//voltage[j]=1;
			// save pressure data with time_index into microSD
       dprint(BTSetup,"A/D%d = %f V\n", j, voltage[j]);
			fprintf(fp[j+1], "%d , %f\r", time_index, voltage[j]);
       
    }
    time_index++;
   // dprint(BTSetup,"after here");        
	
  /*
	///FOR DUMMY TEST//////
	while(time_index<10)
	{ 	
		//Collect data from sensors
		fprintf(fp[1], "%d , %f\r", time_index, pressure_data);
		fprintf(fp[2], "%d , %f\r", time_index, pressure_data*2);
		fprintf(fp[3], "%d , %f\r", time_index, pressure_data*3);
		fprintf(fp[4], "%d , %f\r", time_index, pressure_data*4);
		///END DUMMY TEST//////
		
		time_index++;
		pressure_data+= 2;	//dummy test
	}	 	
	*/	
	//Close Files after write 	
	fclose(fp[1]);
	fclose(fp[2]);
	fclose(fp[3]);
	fclose(fp[4]);
	sleep(1);		//End of Write
	
	
	//Read data from SDcard
	if(pin12==1)
	{
		high(27);
		low(26);
		//Open Files
		fp[1] = fopen("test0.txt", "r");
		fp[2] = fopen("test1.txt", "r");
		fp[3] = fopen("test2.txt", "r");
		fp[4] = fopen("test3.txt", "r");
		
		if(fp[1] == NULL || fp[2] == NULL || fp[3] == NULL || fp[4] == NULL) 	
		{
			printf("Couldn't open file\n");
			return -1;
		}
		
		
		 //Read file 1
		dprint(BTSetup, "%s", "file1\n");
		while(fgets(buff, 60, fp[1])!=NULL )
		{
			//printf("%s",buff);
			dprint(BTSetup, "%s", buff);
		}
		fclose(fp[1]);
			
		//Read file 2
		dprint(BTSetup, "%s", "file2\n");
		while(fgets(buff, 60, fp[2])!=NULL )
		{
			//printf("%s",buff);
			dprint(BTSetup, "%s", buff);
		}
		fclose(fp[2]);
		
		//Read file 3
		dprint(BTSetup, "%s", "file3\n");
		while(fgets(buff, 60, fp[3])!=NULL )
		{
			//printf("%s",buff);
			dprint(BTSetup, "%s", buff);
		}
		fclose(fp[3]);
		
		//Read file 4
		dprint(BTSetup, "%s", "file4\n");
		while(fgets(buff, 60, fp[4])!=NULL )
		{
			//printf("%s",buff);
			dprint(BTSetup, "%s", buff);
		}
		fclose(fp[4]);
		dprint(BTSetup, "%s", "Done\n");
		
		
		 ////Delete Files
		int ret0,ret1,ret2,ret3;
		ret0 = remove("test0.txt");
		ret1 = remove("test1.txt");
		ret2 = remove("test2.txt");
		ret3 = remove("test3.txt");
		
     time_index=0;
		sleep(4);
		//dprint(BTSetup, "%s", "Delete Done");
		} //end if, when not press button
	}//end while(1)
}//end main







