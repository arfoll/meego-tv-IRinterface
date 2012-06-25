/*
* meego-tv-IRinterface.
* An user space daemon translating RC codes into Linux input events.
* Authored by Chen Jie <jie.a.chen@intel.com>
* Copyright (c) 2011 Intel Corp.
*
* This file is part of meego-tv-IRinterface.
*
* meego-tv-IRinterface is free software; you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public
* License as published by the Free Software Foundation; either
* version 2.1 of the License, or (at your option) any later version.
*
* meego-tv-IRinterface is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with meego-tv-IRinterface; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA */


#include <stdio.h>
#include <unistd.h>
#include <LR_PICInterface.h>

	

static  INT32 ir_handler( UINT8 cmd, UINT8 length, void* data, void* clientData )
{
    unsigned int scancode;

    printf("ir_handler: cmd %x, len %x,data ",cmd, length);
    {
        int i=0;
        for (i=0;i<length;i++)
            printf("%x ", *(((unsigned char*)data)+i));
    }
    printf("\n");

    if (cmd == LR_PIC_IR) {
         PicBufferIR* irBuffer =  PicBufferIR::unserialize((UINT8*)data);
         scancode = (irBuffer->irDevice << 8) | irBuffer->irNumber;
         printf("scancode:%x\n",scancode);
 
    }

    return PIC_SUCCESS;

}


int main(int argc,char** argv)
{

    if (PicInitIR(ir_handler, NULL) != PIC_SUCCESS)
    {
        fprintf(stderr, "failed to init pic interface\n");
        return -1;
    }
    sleep(60*60*24*365);
    return 0;
}




