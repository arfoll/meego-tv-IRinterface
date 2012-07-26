/*
* meego-tv-IRinterface.
* An user space daemon translating RC codes into Linux input events.
* Authored by Chen Jie <jie.a.chen@intel.com>
* Copyright (c) 2011, 2012 Intel Corp.
* Copyright (c) 2012 Brendan Le Foll <brendan@fridu.net>
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
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <pal.h>
#include <linux/uinput.h>
#include <LR_PICInterface.h>
#include <map>
using namespace std;

#define DEVICE_NAME "meego-tv-ir"
#define ID_VENDOR 0x8765
#define ID_PRODUCT 0x4321

static int fd;
static int count = 0;
static map<int,int> keymap;

static int init_map(const char* filename)
{
    FILE* f;

    f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "unable to open '%s'\n", filename);
        return -1;
    }

    while (!feof(f)) {
        char line[1024];
        char *p;
        int scancode, keycode;

        //read 1 line
        if (!fgets(line, sizeof(line), f))
            break;
        //skip the leading space(s)
        p = line+strspn(line, "\t ");
        //skip the comment or blank line
        if (*p == '#' || *p == '\n')
            continue;

        if (sscanf(p, "%i %i", &scancode, &keycode) != 2) {
            fprintf(stderr, "unable to parse '%s'.\n", line );
            continue;
        }

        //add to map
        keymap.insert(pair<int,int>(scancode,keycode));
    }
    return 0;
}

static int lookup_map(int scancode, int &keycode)
{
    map<int,int>::iterator it;

    it = keymap.find(scancode);
    if (it == keymap.end()) {
        return -1;
    }
    keycode = it->second;

    return 0;
}

static INT32 ir_handler(UINT8 cmd, UINT8 length, void* data, void* clientData)
{
    int scancode,keycode;
    struct input_event event = {0};


    if (cmd == LR_PIC_IR) {
         PicBufferIR* irBuffer =  PicBufferIR::unserialize((UINT8*)data);
         scancode = (irBuffer->irDevice << 8) | irBuffer->irNumber;
    }
    else
        return 0;

    // The Cocom PIC sends three hex codes for every key press
    if (!count) {
        count++;
    } else if (count > 2) {
        count = 0;
        return 0;
    } else {
        count++;
        return 0;
    }

    if (lookup_map(scancode,keycode) < 0) {
        fprintf(stderr, "unknown scancode %x\n", scancode);
        return -1;
    }

    gettimeofday(&event.time, NULL);
    event.type  = EV_MSC;
    event.code  = MSC_SCAN;
    event.value = scancode;
    write(fd, &event, sizeof(event));

    gettimeofday(&event.time, NULL);
    event.type  = EV_KEY;
    event.code  = keycode;
    event.value = 1;
    write(fd, &event, sizeof(event));

    gettimeofday(&event.time, NULL);
    event.value = 0;
    write(fd, &event, sizeof(event));

    gettimeofday(&event.time, NULL);
    event.type  = EV_SYN;
    event.code  = SYN_REPORT;
    event.value = 0;
    write(fd, &event, sizeof(event));

    return 0;
}

int main(int argc, char** argv)
{
    struct uinput_user_dev dev = {0};
    int retCode;
    int res = 0;
    char deviceName[80];
    LR_PICInterface* pInterface = NULL;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <keymap file>\n", argv[0]);
        return -1;
    }

    init_map(argv[1]);

    if ((fd = open("/dev/uinput", O_WRONLY | O_NDELAY)) < 0) {
        fprintf(stderr, "/dev/uinput doesn't exist \n" );
        return -1;
    }

    dev.id.version = 1;
    dev.id.bustype = BUS_VIRTUAL;
    dev.id.vendor = ID_VENDOR;
    dev.id.product = ID_PRODUCT;
    strncpy(dev.name, DEVICE_NAME, UINPUT_MAX_NAME_SIZE);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_MSC);
    write(fd, &dev, sizeof(dev));

    for (map<int,int>::iterator it=keymap.begin(); it!=keymap.end(); it++) {
        ioctl(fd, UI_SET_KEYBIT, it->second);
    }
    ioctl(fd, UI_SET_MSCBIT, MSC_SCAN);

    if (ioctl(fd, UI_DEV_CREATE)) {
        fprintf(stderr, "unable to create %s\n", dev.name);
    }

    // Recognize platform and configure PIC interface according to device information
    // Get the platform information to set pic interface
    pal_soc_info_t soc_info;
    if((res = pal_get_soc_info(&soc_info)) == 0) {
        printf("Got the SoC information\n");
        if(soc_info.name == SOC_NAME_CE3100 || soc_info.name == SOC_NAME_CE4100) {
            strcpy(deviceName,"/dev/ttyS1");
        }
        else if(soc_info.name == SOC_NAME_CE4200 || soc_info.name == SOC_NAME_CE5300) {
            strcpy(deviceName,"/dev/ttyS2");
        }
        else {
            fprintf(stderr,"%s  Device is not supported\n",soc_info.name);
            return -1;
        }
    } else {
      fprintf(stderr,"Can't get the SoC information\n");
      return -1;
    }

    //Initialize PIC interface for remote controller
    pInterface = new LR_PICInterface( ir_handler, NULL );
    if ( pInterface == NULL ) {
        fprintf(stderr, "failed to initialize IR!\n");
        close(fd);
        return -1;
    }
    retCode = pInterface->Init( (INT8*)(deviceName) );
    if(retCode != PIC_SUCCESS) {
        fprintf(stderr, "failed to initialize %s for GVL!\n",deviceName);
        close(fd);
        return -1;
    }

    printf("waiting for the PIC events ...\n");
    sleep(60*60*24*365);
    close(fd);

    return 0;
}
