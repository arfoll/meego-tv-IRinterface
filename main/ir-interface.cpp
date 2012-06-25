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
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/uinput.h>
#include <LR_PICInterface.h>
#include <map>
using namespace std;

static int fd;
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

        ///read 1 line
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

static  INT32 ir_handler( UINT8 cmd, UINT8 length, void* data, void* clientData )
{
    int scancode,keycode;
    struct input_event event = {0};


    if (cmd == LR_PIC_IR) {
         PicBufferIR* irBuffer =  PicBufferIR::unserialize((UINT8*)data);
         scancode = (irBuffer->irDevice << 8) | irBuffer->irNumber;
    }
    else
        return 0;
    
    if (lookup_map(scancode,keycode) < 0) {
        fprintf(stderr, "unknown scancode %x\n",scancode);
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


int main(int argc,char** argv)
{
    struct uinput_user_dev dev = {0};

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
    dev.id.vendor = 0x8765;
    dev.id.product = 0x4321;
    strncpy(dev.name, "meego-tv-ir", UINPUT_MAX_NAME_SIZE);

    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_EVBIT, EV_MSC);
    write(fd, &dev, sizeof(dev));
    for (map<int,int>::iterator it=keymap.begin();it!=keymap.end();it++)
        ioctl(fd, UI_SET_KEYBIT, it->second);
    ioctl(fd, UI_SET_MSCBIT, MSC_SCAN);


    if (ioctl(fd, UI_DEV_CREATE)) {
        fprintf(stderr, "unable to create %s\n", dev.name);
    }

    if (PicInitIR(ir_handler, NULL) != PIC_SUCCESS) {
        fprintf(stderr, "failed to initialize IR!\n");
        close(fd);
        return -1;
    }

    printf("waiting for the PIC events ...\n");
    sleep(60*60*24*365);
    close(fd);

    return 0;
}
