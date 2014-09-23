/**
 * Mupen64 - dma.c
 * Copyright (C) 2002 Hacktarux
 *
 * Mupen64 homepage: http://mupen64.emulation64.com
 * email address: hacktarux@yahoo.fr
 * 
 * If you want to contribute to the project please contact
 * me first (maybe someone is already making what you are
 * planning to do).
 *
 *
 * This program is free software; you can redistribute it and/
 * or modify it under the terms of the GNU General Public Li-
 * cence as published by the Free Software Foundation; either
 * version 2 of the Licence, or any later version.
 *
 * This program is distributed in the hope that it will be use-
 * ful, but WITHOUT ANY WARRANTY; without even the implied war-
 * ranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public Licence for more details.
 *
 * You should have received a copy of the GNU General Public
 * Licence along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,
 * USA.
 *
**/

#include "dma.h"
#include "memory.h"
#include "../main/rom.h"
#include <stdio.h>
#include "../r4300/r4300.h"
#include "../r4300/interupt.h"
#include "../r4300/macros.h"
#include <malloc.h>
#include "pif.h"
#include "flashram.h"
#include "../main/guifuncs.h"
#include "../r4300/ops.h"

static unsigned char sram[0x8000];
unsigned char KailleraSaveBuffer[0x8000];
void* Information;
int UseSave = 1;

// Basically this function takes the Good Name of a rom, looks for an SRA file, takes it, compresses it and sets it
// up so it can be sent over Kaillera.



int compress_save_read(char *TargetGoodName, void *StorageBuffer)
{
         int i;
	     char *NoExtension;
         FILE *FileIn;
	     NoExtension = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
	     strcpy(NoExtension, get_savespath());
	     strcat(NoExtension, TargetGoodName);
	     
	     char *SaveFile;
	     SaveFile = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
	     strcpy(SaveFile,NoExtension);
	     strcat(SaveFile, ".sra");
         
        //input and output files
        FileIn = fopen(SaveFile, "rb");
        if (!FileIn)
        {
                 
          free(SaveFile);
         
          SaveFile = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
	      strcpy(SaveFile,NoExtension);
	      strcat(SaveFile, ".eep");
	     
          FileIn = fopen(SaveFile, "rb");
      
	      if (!FileIn)
          {
              
                  
              free(SaveFile);
              SaveFile = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
    	      strcpy(SaveFile,NoExtension);
    	      strcat(SaveFile, ".fla");
    	      FileIn = fopen(SaveFile, "rb");
    	      if (!FileIn)
              {
                     
                  free(SaveFile);
                  SaveFile = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
        	      strcpy(SaveFile,NoExtension);
        	      strcat(SaveFile, ".mpk");
        	      FileIn = fopen(SaveFile, "rb");
        	      if (!FileIn)
                  {
                   
                    free(SaveFile);  
                    return 0;
                  }
              }

           }
        }  
        
            //get the file size of the input file
            fseek(FileIn, 0, SEEK_END);
            int FileInSize = ftell(FileIn);
            
            
            //buffers for the raw and compressed data
            char *RawDataBuff = malloc(FileInSize);
            char *CompDataBuff = NULL;
            
            
            //zlib states that the source buffer must be at least 0.1
            //times larger than the source buffer plus 12 bytes
            //to cope with the overhead of zlib data streams
            int CompBuffSize = (int)(FileInSize + (FileInSize * 0.1) + 12);
            CompDataBuff = malloc(CompBuffSize);
            
            //read in the contents of the file into the source buffer
            rewind(FileIn);
            fread(RawDataBuff, FileInSize, 1, FileIn);
    
            //now compress the data
            compress2(CompDataBuff, &CompBuffSize, RawDataBuff, FileInSize, 5);
        
            //write the compressed data to disk
        

            for(i = 0; i < CompBuffSize; i++){
    				((char*)StorageBuffer)[i] = CompDataBuff[i];
    		}
            fclose(FileIn);
            free(SaveFile);
            return CompBuffSize;
        
}

int uncompress_save_write(char *TargetGoodName, void *CompressedBuffer, int CompressedLength)
{
         int i;
	     char *NoExtension;
         char *SaveDataBuff;
         SaveDataBuff = malloc(0x20000);
         int SaveBuffSize = 0x20000;
         
         
         
         ShowInfo("usave: %i",UseSave);
	     NoExtension = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
	     strcpy(NoExtension, get_savespath());
	     strcat(NoExtension, TargetGoodName);

	     char *NetplayFile;
	     NetplayFile = malloc(strlen(get_savespath()) + strlen(TargetGoodName)+4+1);
	     strcpy(NetplayFile,NoExtension);
	     strcat(NetplayFile, ".net");
	     
        //input and output files

        FILE *FileOut = fopen(NetplayFile, "wb");
        
        //now compress the data
        if (CompressedLength > 0)
        {
            if(uncompress(SaveDataBuff, &SaveBuffSize, CompressedBuffer, CompressedLength) == 0)
            {  
            //write the compressed data to disk
                    fwrite(SaveDataBuff, SaveBuffSize, 1, FileOut);
                    fclose(FileOut);
                    free(NetplayFile);
                    return SaveBuffSize;
            // memcpy(sram,SaveDataBuff,0x8000);
            }
            else
            {
                ShowInfo("Error uncompressing savedata! Desynch detected!");
                fclose(FileOut);
                free(NetplayFile);
                return -1;
            }
        }
        fclose(FileOut);
        free(NetplayFile);
        return 0;
         

}
void dma_pi_read()
{
   int i;
   
   if (pi_register.pi_cart_addr_reg >= 0x08000000 &&
       pi_register.pi_cart_addr_reg < 0x08010000)
     {
	if (use_flashram != 1)
	  {
	     char *filename;
	     FILE *f;
	     filename = malloc(strlen(get_savespath())+
			       strlen(ROM_SETTINGS.goodname)+4+1);
	     strcpy(filename, get_savespath());
	     strcat(filename, ROM_SETTINGS.goodname);
	     if (UseSave == 1)
            strcat(filename, ".sra");
         else
            strcat(filename,".net");
         
	     f = fopen(filename, "rb");
	     if (f)
 	      {
		  fread(sram, 1, 0x8000, f);
		  fclose(f);
	       }
	     else for (i=0; i<0x8000; i++) sram[i] = 0;
	     for (i=0; i<(pi_register.pi_rd_len_reg & 0xFFFFFF)+1; i++)
	       sram[((pi_register.pi_cart_addr_reg-0x08000000)+i)^S8]=
	       ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8];
	     f = fopen(filename, "wb");
	     fwrite(sram, 1, 0x8000, f);
	     fclose(f);
	     free(filename);
	     use_flashram = -1;
	  }
	else
	  dma_write_flashram();
     }
   else
     printf("unknown dma read\n");
   
   pi_register.read_pi_status_reg |= 1;
   update_count();
   add_interupt_event(PI_INT, 0x1000/*pi_register.pi_rd_len_reg*/);
}

void dma_pi_write()
{
   unsigned long longueur;
   int i;
   
   if (pi_register.pi_cart_addr_reg < 0x10000000)
     {
	if (pi_register.pi_cart_addr_reg >= 0x08000000 &&
	    pi_register.pi_cart_addr_reg < 0x08010000)
	  {
	     if (use_flashram != 1)
	       {
		  char *filename;
		  FILE *f;
		  int i;
		  filename = malloc(strlen(get_savespath())+
				    strlen(ROM_SETTINGS.goodname)+4+1);
		  strcpy(filename, get_savespath());
		  strcat(filename, ROM_SETTINGS.goodname);
	      if (UseSave == 1)
            strcat(filename, ".sra");
          else
            strcat(filename,".net");
		  f = fopen(filename, "rb");
		  if (f)
		    {
		       fread(sram, 1, 0x8000, f);
		       fclose(f);
		    }
		  else for (i=0; i<0x8000; i++) sram[i] = 0x0;
		  free(filename);
		  for (i=0; i<(pi_register.pi_wr_len_reg & 0xFFFFFF)+1; i++)
		    ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
		    sram[(((pi_register.pi_cart_addr_reg-0x08000000)&0xFFFF)+i)^S8];
		  use_flashram = -1;
	       }
	     else
	       dma_read_flashram();
	  }
	else if (pi_register.pi_cart_addr_reg >= 0x06000000 &&
		 pi_register.pi_cart_addr_reg < 0x08000000)
	  {
	  }
	else
	  printf("unknown dma write:%x\n", (int)pi_register.pi_cart_addr_reg);
	
	pi_register.read_pi_status_reg |= 1;
	update_count();
	add_interupt_event(PI_INT, /*pi_register.pi_wr_len_reg*/0x1000);
     
	return;
     }
   
   if (pi_register.pi_cart_addr_reg >= 0x1fc00000) // for paper mario
     {
	pi_register.read_pi_status_reg |= 1;
	update_count();
	add_interupt_event(PI_INT, 0x1000);
	return;
     }
   
   longueur = (pi_register.pi_wr_len_reg & 0xFFFFFF)+1;
   i = (pi_register.pi_cart_addr_reg-0x10000000)&0x3FFFFFF;
   longueur = (i + longueur) > taille_rom ?
     (taille_rom - i) : longueur;
   longueur = (pi_register.pi_dram_addr_reg + longueur) > 0x7FFFFF ?
     (0x7FFFFF - pi_register.pi_dram_addr_reg) : longueur;
   
   if(i>taille_rom || pi_register.pi_dram_addr_reg > 0x7FFFFF)
     {
	pi_register.read_pi_status_reg |= 3;
	update_count();
	add_interupt_event(PI_INT, longueur/8);
	return;
     }
   
   if(!interpcore)
     {
	for (i=0; i<longueur; i++)
	  {
	     unsigned long rdram_address1 = pi_register.pi_dram_addr_reg+i+0x80000000;
	     unsigned long rdram_address2 = pi_register.pi_dram_addr_reg+i+0xa0000000;
	     
	     ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
	       rom[(((pi_register.pi_cart_addr_reg-0x10000000)&0x3FFFFFF)+i)^S8];
	     
	     if(!invalid_code[rdram_address1>>12])
	       if(blocks[rdram_address1>>12]->block[(rdram_address1&0xFFF)/4].ops != NOTCOMPILED)
		 invalid_code[rdram_address1>>12] = 1;
	     
	     if(!invalid_code[rdram_address2>>12])
	       if(blocks[rdram_address2>>12]->block[(rdram_address2&0xFFF)/4].ops != NOTCOMPILED)
		 invalid_code[rdram_address2>>12] = 1;
	  }
     }
   else
     {
	for (i=0; i<longueur; i++)
	  {
	     ((unsigned char*)rdram)[(pi_register.pi_dram_addr_reg+i)^S8]=
	       rom[(((pi_register.pi_cart_addr_reg-0x10000000)&0x3FFFFFF)+i)^S8];
	  }
     }
   
   /*for (i=0; i<=((longueur+0x800)>>12); i++)
     invalid_code[(((pi_register.pi_dram_addr_reg&0xFFFFFF)|0x80000000)>>12)+i] = 1;*/
   
   if ((debug_count+Count) < 0x100000)
     {
	switch(CIC_Chip)
	  {
	   case 1:
	   case 2:
	   case 3:
	   case 6:
	     rdram[0x318/4] = 0x800000;
	     break;
	   case 5:
	     rdram[0x3F0/4] = 0x800000;
	     break;
	  }

     }
   
   pi_register.read_pi_status_reg |= 3;
   update_count();
   add_interupt_event(PI_INT, longueur/8);
   return;
}

void dma_sp_write()
{
   int i;
   if ((sp_register.sp_mem_addr_reg & 0x1000) > 0)
     {
	for (i=0; i<((sp_register.sp_rd_len_reg & 0xFFF)+1); i++)
	  ((unsigned char *)(SP_IMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8]=
	  ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8];
     }
   else
     {
	for (i=0; i<((sp_register.sp_rd_len_reg & 0xFFF)+1); i++)
	  ((unsigned char *)(SP_DMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8]=
	  ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8];
     }
}

void dma_sp_read()
{
   int i;
   if ((sp_register.sp_mem_addr_reg & 0x1000) > 0)
     {
	for (i=0; i<((sp_register.sp_wr_len_reg & 0xFFF)+1); i++)
	  ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8]=
	  ((unsigned char *)(SP_IMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8];
     }
   else
     {
	for (i=0; i<((sp_register.sp_wr_len_reg & 0xFFF)+1); i++)
	  ((unsigned char *)(rdram))[((sp_register.sp_dram_addr_reg & 0xFFFFFF)+i)^S8]=
	  ((unsigned char *)(SP_DMEM))[((sp_register.sp_mem_addr_reg & 0xFFF)+i)^S8];
     }
}

void dma_si_write()
{
   int i;
   if (si_register.si_pif_addr_wr64b != 0x1FC007C0)
     {
	printf("unknown SI use\n");
	stop=1;
     }
   for (i=0; i<(64/4); i++)
     PIF_RAM[i] = sl(rdram[si_register.si_dram_addr/4+i]);
   update_pif_write();
   update_count();
   add_interupt_event(SI_INT, /*0x100*/0x900);
}

void dma_si_read()
{
   int i;
   if (si_register.si_pif_addr_rd64b != 0x1FC007C0)
     {
	printf("unknown SI use\n");
	stop=1;
     }
   update_pif_read();
   for (i=0; i<(64/4); i++)
     rdram[si_register.si_dram_addr/4+i] = sl(PIF_RAM[i]);
   update_count();
   add_interupt_event(SI_INT, /*0x100*/0x900);
}
