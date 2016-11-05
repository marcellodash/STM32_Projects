#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#if defined(OS_LINUX) || defined(OS_MACOSX)
#include <sys/ioctl.h>
#include <termios.h>
#elif defined(OS_WINDOWS)
//#include <conio.h>
#endif

#include "hid.h"

// HID Special Command 

#define WakeUP     0x08  // WakeUP for first STM32 Communication
#define Read16     0x09  // Read16 : Read Page (64 byte) in 16 bit mode
#define Read8      0x0A  // Read8  : Read Page (64 byte) in 8 bit mode
#define Erase8     0x0B  // Erase8 : Erase Page (64 byte) in 8 bit mode
#define Erase16    0x0C  // Erase16: Erase Page (64 byte) in 16 bit mode
#define Write8     0x0D  // Read8  : Read Page (64 byte) in 8 bit mode



int main()
{
    int  r, num;
    unsigned char buf[64];
    int choixMenu=0;
    int ReadOK=1;
    unsigned long address=0;
    unsigned long i=0;
    unsigned long j=0;
    unsigned char octetActuel=0;

    // HID Command Buffer
    unsigned char HIDCommand [64];
     unsigned char *BufferROM;
     unsigned char *BufferSave;
    FILE *dump;
    FILE *save;

    printf("-= Sega Megadrive USB Dumper -= \n\n");
    printf("Detecting USB Device ... \n");

    r = rawhid_open(1, 0x0483, 0x5750,-1, -1);
    if (r <= 0)
    {

        r = rawhid_open(1, 0x0483, 0x5750,-1, -1);
        if (r <= 0)
        {
            printf("No hid device found\n");
            scanf("%d", &choixMenu);
            return -1;
        }
    }

    printf("found HID Megadrive Dumper ! \n\n");
    printf("Receiving game info ...\n\n");


    //////////////
    
    HIDCommand[0] = WakeUP; // Select WakeUP Command
    	rawhid_send(0,HIDCommand,64,25);

    	while (ReadOK !=0)
    	{

    	  num = rawhid_recv(0, buf, 64,25);
    	  		if (num < 0) {
    			printf("\nerror reading, device went offline\n");
    			rawhid_close(0);
    			return 0;
    		}
    		if (num > 0) {
    			/*printf("\nrecu %d bytes:\n", num);
    			for (i=0; i<num; i++) {
    				printf("%02X ", buf[i] & 255);
    				if (i % 16 == 15 && i < num-1) printf("\n");
    			}
    			printf("\n\n");*/
    			ReadOK=0;
    		}


    	}   
    unsigned char ReleaseDate[8];
    unsigned char GameName[32];
    unsigned long Gamesize=0;
    unsigned short SaveSize=0;
    
    for (i=0; i<8; i++) 
    {
      ReleaseDate[i]=buf[i];     
    }
    
     for (i=0; i<32-8 ; i++) 
    {
      GameName[i]=buf[i+8];     
    }
    
    Gamesize = ((buf[42]) | (buf[41] << 8) | (buf[40]<<16));
    Gamesize=(Gamesize/1024)+1; 
    SaveSize = ((buf[45]) | (buf[46] << 8));
    SaveSize=(SaveSize/1024)+1;   
      
   printf("Game Name : %s \n",GameName);
   printf("\nRelease Date : %s \n",ReleaseDate);
   printf("Game Size : %ld Ko \n",Gamesize);
   if (buf[43]!=0x01){ printf("Save Support : No\n ");}
   else {
     printf("Save Support : Yes\n ");
    printf("Save Size : %d Ko \n",SaveSize);
   if (buf[44]==0xF8){ printf("Save Type : SRAM\n ");}
   if (buf[44]==0xE8){printf("Save Type : EEPROM\n ");}
   else {printf("Save Type : NONE\n");}}
  printf("Region : %c",buf[48]);
  printf("%c",buf[49]);
  printf("%c \n\n",buf[50]);
     
    
    ///////////////



    printf("---Menu---\n\n");
    printf("1.Dump SMD ROM\n");
    printf("2.Dump SMD Save\n");
    printf("3.Write SMD Save\n");
    printf("4.Erase SMD Save\n");
    printf("5.Dump SMS ROM \n");
    printf("8.SMD Hex View \n");
    printf("9.SMS Hex View \n");
    printf("\nWhat do you want ?\n\n");
    scanf("%d", &choixMenu);
    clock_t start = clock();
    clock_t finish = clock();

    switch(choixMenu)
    {
    case 1:
        printf("Sending command Dump ROM \n");
        printf("Dumping please wait ...\n");
        HIDCommand[0] = 0x09; // Select Read in 16bit Mode
        // building adress

        address=0;
        i=0;
        j=0;
	BufferROM = (unsigned char*)malloc(1024*Gamesize);

	
	
           while (address < ((1024*Gamesize)/2) )
	   
	   {
           
            HIDCommand[1]=address & 0xFF;
            HIDCommand[2]=(address & 0xFF00)>>8;
            HIDCommand[3]=(address & 0xFF0000)>>16;
            HIDCommand[4]=(address & 0xFF000000)>>24;        
            rawhid_send(0, HIDCommand, 64, 25);
	    rawhid_recv(0,(BufferROM+ (address*2)), 64,25);
          //  memcpy((unsigned char *)BufferROM+ (address*2), (unsigned char *)buf, 64);
            address +=32 ;
        }

        dump=fopen("dump.bin","wb");
        fwrite(BufferROM,1,1024*Gamesize,dump);
    	finish = clock();
	printf("Dump completed in %ld ms",(finish - start));
        scanf("%d");
	break;

    case 2:
        printf("Sending command Dump Save \n");
        printf("Dumping please wait ...\n");
        HIDCommand[0] = 0x0A; // Select Read in 8bit Mode
 

        address=0x200001; // Start adress for Save Area
        j=0;
	BufferROM = (unsigned char*)malloc(1024*64);
	BufferSave = (unsigned char*)malloc((1024*64));
	       // Cleaning Buffer
        
         for (i=0; i<(1024*64); i++) 
	  {
	    BufferSave[i]=0xFF;
	    BufferROM[i]=0xFF;
	  }
	  rawhid_send(0, HIDCommand, 64, 12);
	    num = rawhid_recv(0,buf, 64,12);
	    address = address/2; // 8 bits read
	while (j < (1024*64)/2 )
      {
	
	
	    HIDCommand[1]=address & 0xFF;
            HIDCommand[2]=(address & 0xFF00)>>8;
            HIDCommand[3]=(address & 0xFF0000)>>16;
            HIDCommand[4]=(address & 0xFF000000)>>24;        
            rawhid_send(0, HIDCommand, 64, 12);
	    rawhid_recv(0,BufferROM+j, 64,12);
	    j +=64;
	    address +=64;
      }
	    	   	    	
	j=0;

	   for (i=0; i<(1024*64)/2; i++) 
	  {
	    j=j+1;
	    BufferSave[i+j]=BufferROM[i];
	  }
	dump=fopen("dump.srm","wb");
        fwrite(BufferSave,1,(1024*64),dump);
	printf("Dump Save OK");
	scanf("%d");
        break;
	
    case 3:
        printf("Opening save file.. \n");
	BufferSave = (unsigned char*)malloc((1024*64));
	BufferROM =  (unsigned char*)malloc(1024*64);
	save=fopen("save.srm","rb");
	if (save == NULL)
	    {
	      printf("file save.srm not found !\n");
	      printf("exit application\n");
	      scanf("%d");
	      exit(0);
            }
        else
            {
	      printf("Send save to cartridge please wait ....\n");
	        for (i=0; i<(1024*64); i++) 
		{
		  fread(&octetActuel,1,1,save); 
		  fread(&octetActuel,1,1,save);
		  BufferSave[i]=octetActuel; 
		}
		while ( j< (1024*64)/2)
		{
		    HIDCommand[0] = 0x0D; // Select Write in 8bit Mode
		    for (i=0; i<32; i++) 
		      {
		        HIDCommand[32+i]=BufferSave[i+j];			
		      }
		    rawhid_send(0,HIDCommand, 64, 12);
		  while (buf[0] != 0xAA)
		  {
		    num = rawhid_recv(0,buf, 64,12);
		  }
		  j +=32;
		}
		    printf("Save Writted sucessfully ! \n");
		    scanf("%d");
	    }
	         
	  break;
	
    case 4:
        printf("WARNING ALL SAVED DATA WILL BE LOST CONTINUE ?  \n");
	scanf("%d");
        printf("Sending command Erase Save \n");
        HIDCommand[0] = 0x0B; // Select Erase in 8bit Mode
        rawhid_send(0, HIDCommand, 64, 12);
	printf("Erasing please wait...");
	while (buf[0] != 0xAA)
	{
  	   num = rawhid_recv(0,buf, 64,12);
	}
        printf("\nErase completed sucessfully ! \n");
	break;
	
    case 5:
         printf("Sending command Dump SMS \n");
        printf("Dumping please wait ...\n");
	HIDCommand[0] = 0x0A; // Select Read in 8bit Mode
	BufferROM = (unsigned char*)malloc(1024*256);
	 HIDCommand[1]=address & 0xFF;
            HIDCommand[2]=(address & 0xFF00)>>8;
            HIDCommand[3]=(address & 0xFF0000)>>16;
            HIDCommand[4]=(address & 0xFF000000)>>24;        
            rawhid_send(0, HIDCommand, 64, 12);
	    rawhid_recv(0,BufferROM+j, 64,12);
	j=0;
	
      while (address < (1024*256))
      {

	address = address; // 8 bits read
	
	 HIDCommand[1]=address & 0xFF;
            HIDCommand[2]=(address & 0xFF00)>>8;
            HIDCommand[3]=(address & 0xFF0000)>>16;
            HIDCommand[4]=(address & 0xFF000000)>>24;        
            rawhid_send(0, HIDCommand, 64, 12);
	    rawhid_recv(0,BufferROM+j, 64,12);
	    j +=64;
	    address +=64;
      }
        dump=fopen("dump.sms","wb");
        fwrite(BufferROM,1,(1024*256),dump);
	printf("Dump SMS OK");
	scanf("%d");
	break;
	
    case 8:
      BufferROM = (unsigned char*)malloc(1024*Gamesize);
	HIDCommand[0] = 0x09; // Select Read in 16bit Mode
      while (1)
      {
        printf("\n\nEnter ROM Address ( decimal value) :\n \n");
	scanf("%ld",&address);
	address = address/2; // 16 bits read
	
	 HIDCommand[1]=address & 0xFF;
            HIDCommand[2]=(address & 0xFF00)>>8;
            HIDCommand[3]=(address & 0xFF0000)>>16;
            HIDCommand[4]=(address & 0xFF000000)>>24;        
            rawhid_send(0, HIDCommand, 64, 12);
	    num = rawhid_recv(0,buf, 64,12);
	     if (num > 0) {
    			printf("\n\n", num);
    			for (i=0; i<num; i++) {
    				printf("%02X ", buf[i] & 255);
    				if (i % 16 == 15 && i < num-1) printf("\n");
					      }
			  }
	   
      }
      
      case 9:
      BufferROM = (unsigned char*)malloc(1024*Gamesize);
	HIDCommand[0] = 0x0A; // Select Read in 8bit Mode
	  rawhid_send(0, HIDCommand, 64, 12);
	    num = rawhid_recv(0,buf, 64,12);
      while (1)
      {
        printf("\n\nEnter ROM Address ( decimal value) :\n \n");
	scanf("%ld",&address);
	address = address; // 8 bits read
	
	 HIDCommand[1]=address & 0xFF;
            HIDCommand[2]=(address & 0xFF00)>>8;
            HIDCommand[3]=(address & 0xFF0000)>>16;
            HIDCommand[4]=(address & 0xFF000000)>>24;        
            rawhid_send(0, HIDCommand, 64, 12);
	    num = rawhid_recv(0,buf, 64,12);
	    
	     if (num > 0) {
    			printf("\n\n", num);
    			for (i=0; i<num; i++) {
    				printf("%02X ", buf[i] & 255);
    				if (i % 16 == 15 && i < num-1) printf("\n");
					      }
			  }
	   
      }

    default:
        printf("Nice try bye ;)");

    }
}


#if defined(OS_LINUX) || defined(OS_MACOSX)


#endif



