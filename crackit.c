#include "libattopng.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

#define W 640
#define H 640

#define STARTS_LEFT 0

#define GENERATE_NUM_CRACKS 500


#define THICK_THRESH_2 30 //60
#define THICK_THRESH_3 50 //90
#define CRACK_COUNTDOWN 20
#define BRANCH_COUNTDOWN 50
#define HORIZONTAL_PERTURB 30
#define VERTICAL_PERTURB 70

#define INVERT_COLOR 0

#define MINIMUM_CRACK_LEN 100

#define MAX_BRANCH 3

#define RGBA(r, g, b, a) ((r) | ((g) << 8) | ((b) << 16) | ((a) << 24))
#define RGB(r, g, b) RGBA(r, g, b, 0xff)
#define RGB5(r, g, b) RGBA(r, g, b, 0xaa)

#define ALPHA(c, a) ((c) | ((a) << 8))

int FILL_COLOR1r= 0x22;
int FILL_COLOR1g= 0x22;
  int FILL_COLOR1b= 0x22;
  int FILL_COLOR2r= 0x11;
  int FILL_COLOR2g= 0x11;
  int FILL_COLOR2b= 0x11;

int x_ratio;
int y_ratio;
int x_inc;
int y_inc;
int crack_length;
int BG_COLOR=0;
int itercount=0;

void set_ratios_from_degrees(int degree){
    double radians = degree * M_PI / 180.0;
    double x_part = cos(radians)*10;
    double y_part = sin(radians)*10;
    x_ratio = (x_part);
    y_ratio = (y_part);
    if (x_ratio >= 0)
        x_inc = 1;
    else {
        x_inc = -1;
        x_ratio=x_ratio*-1;
    }
    if (y_ratio >= 0)
        y_inc = 1;
    else {
        y_inc = -1;
        y_ratio=y_ratio*-1;
    }
    if (x_ratio > y_ratio) {
        if (y_ratio!=0) {
            x_ratio=x_ratio/y_ratio;
            y_ratio=y_ratio/y_ratio;
        }
    } else {
        if (x_ratio!=0) {
            y_ratio=y_ratio/x_ratio;
            x_ratio=x_ratio/x_ratio;
        }
    }
//    printf("Ratio Set degree=%d, radians=%f -- x_ratio %d, y_ratio %d, x_inc %d, y_inc %d\n", degree, radians, x_ratio, y_ratio, x_inc, y_inc);
}

float minx, miny;
float maxx, maxy;

void change_bg_color(libattopng_t *png){
  int x, y;
  int intensity1, intensity2, intensity3;
  BG_COLOR=rand()%10;
  
  
  for (y = 0; y < H; y++) {
      for (x = 0; x < W; x++) {
	if (libattopng_get_pixel(png, x, y)==RGBA(0,0,0,0))   {

	  if (INVERT_COLOR==0) {
	    intensity1=rand()%128+128;
	    intensity2=rand()%128+128;
	    intensity3=rand()%128+128;
	  } else {
	    intensity1=rand()%128;
	    intensity2=rand()%128;
	    intensity3=rand()%128;	    
	  }
	  
	  if (BG_COLOR == 0)
	    libattopng_set_pixel(png, x, y, RGBA(intensity1,0,0,255));
	  else if (BG_COLOR == 1)
	    libattopng_set_pixel(png, x, y, RGBA(0,intensity2,0,255));
	  else if (BG_COLOR == 2)
	    libattopng_set_pixel(png, x, y, RGBA(0,0,intensity3,255));
	  else if (BG_COLOR == 3)
	    libattopng_set_pixel(png, x, y, RGBA(intensity1,intensity2,0,255));
	  else if (BG_COLOR == 4)
	    libattopng_set_pixel(png, x, y, RGBA(0,intensity2,intensity3,255));
	  else if (BG_COLOR == 5)
	    libattopng_set_pixel(png, x, y, RGBA(intensity1,0,intensity3,255));
	  else
	    libattopng_set_pixel(png, x, y, RGBA(intensity1,intensity2,intensity3,255));
	}
      }
  }
}

void set_bounding_box(libattopng_t *png){
    int x, y;

    minx=W;
    miny=H;
    maxx=0;
    maxy=0;

    for (y = 0; y < H; y++) {
      for (x = 0; x < W; x++) {
	if (libattopng_get_pixel(png, x, y)!=RGBA(0,0,0,0))   {
	  if (x<minx)
	    minx=x;
	  if (y<miny)
	    miny=y;
	  if (x>maxx)
	    maxx=x;
	  if (y>maxy)
	    maxy=y;
	}
      }
    }

}

void border_the_crack(libattopng_t *png){
    int x, y;

    if (INVERT_COLOR)
      {
	FILL_COLOR1r= 0xDD;
	FILL_COLOR1g= 0xDD;
	FILL_COLOR1b= 0xDD;
	FILL_COLOR2r= 0xEE;
	FILL_COLOR2g= 0xEE;
	FILL_COLOR2b= 0xEE;
      }
    
    
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
            if (libattopng_get_pixel(png, x, y)==RGB(0,0,0))   {         
                if (x+1 < W) {
                    if (libattopng_get_pixel(png, x+1, y)!=RGB(0,0,0))            
                        libattopng_set_pixel(png, x+1, y, RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b));
                    if (x+2 < W) {
                        if ((libattopng_get_pixel(png, x+2, y)!=RGB(0,0,0)) && (libattopng_get_pixel(png, x+2, y)!=RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b)))         
                            libattopng_set_pixel(png, x+2, y, RGB5(FILL_COLOR1r,FILL_COLOR1g,FILL_COLOR1b));
                    }
                }
                if (y+1 < H) {
                    if (libattopng_get_pixel(png, x, y+1)!=RGB(0,0,0))            
                        libattopng_set_pixel(png, x, y+1, RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b));
                    if (y+2 < H) {
                        if ((libattopng_get_pixel(png, x, y+2)!=RGB(0,0,0)) && (libattopng_get_pixel(png, x, y+2)!=RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b)))         
                            libattopng_set_pixel(png, x, y+2, RGB5(FILL_COLOR1r,FILL_COLOR1g,FILL_COLOR1b));
                    }
                }
                if (x-1 >= 0) {
                    if (libattopng_get_pixel(png, x-1, y)!=RGB(0,0,0))            
                        libattopng_set_pixel(png, x-1, y, RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b));
                    if (x-2 >= 0) {
                        if ((libattopng_get_pixel(png, x-2, y)!=RGB(0,0,0)) && (libattopng_get_pixel(png, x-2, y)!=RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b)))         
                            libattopng_set_pixel(png, x-2, y, RGB5(FILL_COLOR1r,FILL_COLOR1g,FILL_COLOR1b));
                    }
                }
                if (y-1 >= 0) {
                    if (libattopng_get_pixel(png, x, y-1)!=RGB(0,0,0))            
                        libattopng_set_pixel(png, x, y-1, RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b));
                    if (y-2 >= 0) {
                        if ((libattopng_get_pixel(png, x, y-2)!=RGB(0,0,0)) && (libattopng_get_pixel(png, x, y-2)!=RGB5(FILL_COLOR2r,FILL_COLOR2g,FILL_COLOR2b)))         
                            libattopng_set_pixel(png, x, y-2, RGB5(FILL_COLOR1r,FILL_COLOR1g,FILL_COLOR1b));
                    }
                }
            }
        }
    }
}

void crack_it_up(int x, int y, int thickness, libattopng_t * png, int vertcrack) {
int i;

//    printf("Drawing x=%d, y=%d -- thickness %d\n", x, y, thickness);

    if (vertcrack) {
        for (i=0; i<thickness; i++)
	  if (INVERT_COLOR==0)
	    libattopng_set_pixel(png, x+i, y, RGB(0,0,0));
	  else
	    libattopng_set_pixel(png, x+i, y, RGB(255,255,255));
    } else {
        for (i=0; i<thickness; i++)
	  if (INVERT_COLOR==0)
            libattopng_set_pixel(png, x, y+i, RGB(0,0,0));
	  else
	    libattopng_set_pixel(png, x, y+i, RGB(255,255,255));
    }
}


int main(int argc, char *argv[]){
    int x,y;
    int crack_branching[MAX_BRANCH][4];
    int current_branch;
    int add_branch;
    int growth_direction;
    int initial_direction;
    int fate;
    int direction;
    int thickness;
    int branch;
    int t,i;
    int vertcrack;
    int counter;
    int branch_counter;
    libattopng_t *png=NULL;
    int x_count, y_count;
    int current_degree;
    char str2[60];
    int generate_batch_num=0;
    
      srand(time(NULL));

 for (itercount=0; itercount < 3; itercount++) {
   switch (itercount){
   case 0:
     generate_batch_num=GENERATE_NUM_CRACKS*.8;
     break;
   case 1:
     generate_batch_num=GENERATE_NUM_CRACKS*.1;
     break;
   case 2:
     generate_batch_num=GENERATE_NUM_CRACKS*.1;
     break;
   }
  for (int k=0;k<generate_batch_num;k++){

    branch_counter=BRANCH_COUNTDOWN;
    counter=CRACK_COUNTDOWN;
    vertcrack=0;
    current_branch=0;
    add_branch=1;
    crack_length=0;

    png = libattopng_new(W, H, PNG_RGBA);

    
    // Set image to clear
    for (y = 0; y < H; y++) {
        for (x = 0; x < W; x++) {
	  libattopng_set_pixel(png, x, y, RGBA(0,0,0,0));	    
        }
    }

    // Invalidate all crack branches
    for (i=0; i<10; i++)
        crack_branching[i][0]=0;

    // Initial values
    fate=500;

    if (STARTS_LEFT) {
        current_degree = 45-rand()%90;
        x=4;
    } else {
        current_degree = rand()%360;
        x=rand()%W;
    }
    
    initial_direction=current_degree;
    set_ratios_from_degrees(current_degree);
    y=rand()%H;
    x_count=0;
    y_count=0;

    // Main loop  
    while (1) {

        // Random knobs
        fate=rand()%1000;  // Controls length of crack and changes in direction
        thickness=rand()%100; // Controls thickness of crack
        branch=rand()%100; // Controls when a crack branches off to a new crack
        direction=rand()%100;

        if ((x<2) || (x>(W-2)) || (y<2) || (y>(H-2)) || (fate==0))  // Out of image bounds
            {
                // Move to next saved branching point
                current_branch++;
                if (current_branch>=MAX_BRANCH) // Check if out of branch contexts
                    {
                        break;
                    }
                else if (crack_branching[current_branch][0]==0)  // Check if branching point is valid
                    {
                        break;
                    }
                else {  // Start new branching point by loading context
                    current_degree = crack_branching[current_branch][3];   
                    initial_direction=current_degree;
                    set_ratios_from_degrees(current_degree);
                    x=crack_branching[current_branch][1];
                    y=crack_branching[current_branch][2];
                    x_count=0;
                    y_count=0;
                }
            }

        // Figure out the thickness of the crack at this point
        if (thickness<THICK_THRESH_2) 
            t=2;
        else if (thickness<THICK_THRESH_3)
                t=3;
            else
                t=4;

        // Draw the crack portion
        crack_it_up(x,y,t,png,vertcrack);
        crack_length++;

        // Check for branching
        if (fate>800 && add_branch<MAX_BRANCH && branch_counter==0) {
            // Save the branching point context
            crack_branching[add_branch][0]=1; // Valid branch
            crack_branching[add_branch][1]=x; // x coord start
            crack_branching[add_branch][2]=y; // y coord start
            t=rand()%180;
            t-=90;
            crack_branching[add_branch][3]=current_degree+t; // direction
            add_branch++;
            branch_counter=BRANCH_COUNTDOWN;
        }

        // Change in crack direction
        if (fate<100 && counter==0) {
            t=rand()%90;
            t-=45;
            current_degree+=t;
                                
            set_ratios_from_degrees(current_degree);
            x_count=0;
            y_count=0;

            // Countdown for crack changes in direction
            counter=CRACK_COUNTDOWN;
        }

        // Small perturbations in x/y coordinates
        if (direction<HORIZONTAL_PERTURB) {
            if (rand()%2)
                x++;
            else
                x--;
        } else {
            if (direction>VERTICAL_PERTURB) {
                if (rand()%2)
                        y++;
                    else
                        y--;
            }
        }


        // Counter to delay directional change 
        counter--;
        if (counter<0) counter=0;

        // Counter to delay adding a branch
        branch_counter--;
        if (branch_counter<0) branch_counter=0;

        // Change x and y coordinates according to current direction
        if (x_ratio==y_ratio){
            x+=x_inc;
            y+=y_inc;
        } else {
            if (x_ratio>y_ratio) {
                x+=x_inc;
                if (y_count==0)
                    y+=y_inc;
                y_count++;
                if (y_count>= x_ratio)
                    y_count=0;
            } else {
                    y+=y_inc;
                    if (x_count==0)
                        x+=x_inc;
                    x_count++;
                    if (x_count>= y_ratio)
                        x_count=0;
            }
    
        }
    }


    if (crack_length>MINIMUM_CRACK_LEN) {
      char str3[10];
      switch (itercount){
      case 0:
	sprintf(str3, "train");
	break;
      case 1:
	sprintf(str3, "val");
	break;
      case 2:
	sprintf(str3, "test");
	break;

      }


        border_the_crack(png);
        set_bounding_box(png);
	if (itercount!=2)
	  change_bg_color(png);
         //libattopng_set_pixel(png, 10, 10, RGBA(4,4,4,4));

	sprintf(str2, "Cracks/%s/images/Crack_MEDTHICK_SL%d_BR%d_%d.png", str3, STARTS_LEFT, MAX_BRANCH, k);
        libattopng_save(png, str2);

	{
	  FILE* file_ptr;
	  sprintf(str2, "Cracks/%s/labels/Crack_MEDTHICK_SL%d_BR%d_%d.txt", str3, STARTS_LEFT, MAX_BRANCH, k);
	  file_ptr = fopen(str2, "w");

	  sprintf(str2, "0 %f %f %f %f\n", ((minx+maxx)/2)/W, ((miny+maxy)/2)/H, (maxx-minx)/W, (maxy-miny)/H);
	  
	  fputs(str2, file_ptr);

	  fclose(file_ptr);
	}

        libattopng_destroy(png);
    } else {
        k--;
        libattopng_destroy(png);
    }
  }
 }
 printf("Done!\n");
 return 0;
}

