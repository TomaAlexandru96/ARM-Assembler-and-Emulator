#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <assert.h>

struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;
int fbdev = 0;
char *fbp;
long int screensize = 0;

void setFB(void);
void closeFB(void);

typedef struct position {
    int x;
    int y;
} position;

typedef struct color {
    int red;
    int green;
    int blue;
    int alpha;
} rgba;

void setFB(void) {
    // open framebuffer device
    fbdev = open("/dev/fb0", O_RDWR);
    if (!fbdev) {
        perror("The device /dev/fb0 couldn't be open!");
    }

    // get variable screen information
    ioctl(fbdev, FBIOGET_VSCREENINFO, &vinfo);
    // get fixed info
    ioctl(fbdev, FBIOGET_FSCREENINFO, &finfo);
    
    vinfo.bits_per_pixel = 16;

    screensize = vinfo.xres * vinfo.yres * (vinfo.bits_per_pixel / 8);

    fbp = (char *) mmap(0, screensize, PROT_READ | PROT_WRITE, 
            MAP_SHARED, fbdev, 0);
}

void closeFB(void) {
    munmap(fbp, screensize);
    if (fbdev) {
        close(fbdev);
    }
}

void setPixel(position pos, rgba color) {
    assert(pos.x >= 0);
    assert(pos.y >= 0);
    assert(pos.x < vinfo.xres);
    assert(pos.y < vinfo.yres);
   
    long int location = (pos.x + vinfo.xoffset) * 
                        (vinfo.bits_per_pixel / 8) + 
                        (pos.y + vinfo.yoffset) * finfo.line_length;
    fbp[location] = color.blue;
    fbp[location + 1] = color.green;
    fbp[location + 2] = color.red;
    fbp[location + 3] = color.alpha;
}

int main(void) {
    setFB();

    for (int i = 0; i < vinfo.yres; i++) {
        for (int j = 0; j < vinfo.xres; j++) {
            position p = {j, i};
            rgba c = {0, 0, 0, 0};
            setPixel(p, c);
        }
    }

    closeFB();
    return 0;
}
