/// 
//  mandel.c
//  Based on example code found here:
//  https://users.cs.fiu.edu/~cpoellab/teaching/cop4610_fall22/project3.html
//
//  Converted to use jpg instead of BMP and other minor changes
//  
// Lab11
// Changes Made by: Chance Mason [masoncp@msoe.edu]
///
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "jpegrw.h"
#include <sys/wait.h>
#include <time.h>
#include <math.h>
#include <string.h>
 
// local routines
static int iteration_to_color( int i, int max);
static int iterations_at_point( double x, double y, int max );
static void compute_image( imgRawImage *img, double xmin, double xmax,
                                    double ymin, double ymax, int max);
static void show_help();
 
 
int main(int argc, char *argv[]) {
    char a;

    // Default configuration values
    const char *outfile = "mandel";
    double xcenter = 0;
    double ycenter = 0;
    double xscale = 4;
    int image_width = 1000;
    int image_height = 1000;
    int num_processes = 1;   // Default number of processes
    int max = 1000;          // Default maximum iterations
    int total_images = 50;   // Default number of images

    // Parse command-line arguments
    while ((a = getopt(argc, argv, "x:y:s:W:H:m:o:c:t:h")) != -1) {
        switch (a) {
            case 'x':
                xcenter = atof(optarg);
                break;
            case 'y':
                ycenter = atof(optarg);
                break;
            case 's':
                xscale = atof(optarg);
                break;
            case 'W':
                image_width = atoi(optarg);
                break;
            case 'H':
                image_height = atoi(optarg);
                break;
            case 'm':
                max = atoi(optarg);
                break;
            case 'o':
                outfile = optarg;
                break;
            case 'c':
                num_processes = atoi(optarg);
                break;
            case 't':
                total_images = atoi(optarg);
                break;
            case 'h':
                show_help();
                exit(1);
                break;
            default:
                show_help();
                exit(1);
                break;
        }
    }

    printf("mandel: x=%lf y=%lf xscale=%lf max=%d outfile=%s\n",
           xcenter, ycenter, xscale, max, outfile);

    if (num_processes <= 0 || total_images <= 0) {
        fprintf(stderr, "Error: num_processes and total_images must be greater than zero.\n");
        exit(1);
    }

    int images_per_process = total_images / num_processes;
    int remaining_images = total_images % num_processes;

    for (int p = 0; p < num_processes; p++) {
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            int start_image = p * images_per_process;
            int end_image = start_image + images_per_process;

            // The last process handles any remaining images
            if (p == num_processes - 1) {
                end_image += remaining_images;
            }

            for (int i = start_image; i < end_image; i++) 
			{
                double scale = xscale / (1.0 + i * 0.1);  // Adjust scale slightly per image
                if (scale <= 0.0) {  // Ensure scale never goes to zero
                    scale = 0.0001;
                }
                double yscale = scale / image_width * image_height;

                // Generating unique file names for each image
                char filename[256];
                snprintf(filename, sizeof(filename), "%s_%d.jpg", outfile, i);

                // Create a raw image of the appropriate size
                imgRawImage *img = initRawImage(image_width, image_height);
                if (!img) {
                    fprintf(stderr, "Error: failed to initialize image.\n");
                    exit(1);
                }
                setImageCOLOR(img, 0);  // Fill with black

                // Compute the Mandelbrot image
                compute_image(img,
                              xcenter - scale / 2,
                              xcenter + scale / 2,
                              ycenter - yscale / 2,
                              ycenter + yscale / 2,
                              max);

                // Save the image to the unique filename
                if (storeJpegImageFile(img, filename) != 0) {
                    fprintf(stderr, "Error: failed to save image %s.\n", filename);
                }

				printf("Created image %s | xcenter=%lf, ycenter=%lf, xscale=%lf, yscale=%lf, max iterations=%d\n",filename, xcenter, ycenter, scale, yscale, max);


                // Free the image resources
                freeRawImage(img);
            }
            exit(0);  // Child process exits
        } else if (pid < 0) {
            perror("Fork failed");  // Error checking
            exit(1);
        }
    }

    // Parent process waits for all child processes to finish
    for (int p = 0; p < num_processes; p++) {
        wait(NULL);
    }

    printf("All images generated.\n");

    return 0;
}



/*
Return the number of iterations at point x, y
in the Mandelbrot space, up to a maximum of max.
*/

int iterations_at_point( double x, double y, int max )
{
	double x0 = x;
	double y0 = y;

	int iter = 0;

	while( (x*x + y*y <= 4) && iter < max ) 
	{

		double xt = x*x - y*y + x0;
		double yt = 2*x*y + y0;

		x = xt;
		y = yt;

		iter++;
	}

	return iter;
}

/*
Compute an entire Mandelbrot image, writing each point to the given bitmap.
Scale the image to the range (xmin-xmax,ymin-ymax), limiting iterations to "max"
*/

void compute_image(imgRawImage* img, double xmin, double xmax, double ymin, double ymax, int max )
{
	int i,j;

	int width = img->width;
	int height = img->height;

	// For every pixel in the image...

	for(j=0;j<height;j++) 
	{

		for(i=0;i<width;i++) 
		{

			// Determine the point in x,y space for that pixel.
			double x = xmin + i*(xmax-xmin)/width;
			double y = ymin + j*(ymax-ymin)/height;

			// Compute the iterations at that point.
			int iters = iterations_at_point(x,y,max);

			// Set the pixel in the bitmap.
			setPixelCOLOR(img,i,j,iteration_to_color(iters,max));
		}
	}
}


/*
Convert a iteration number to a color.
Here, we just scale to gray with a maximum of imax.
Modify this function to make more interesting colors.
*/
int iteration_to_color( int iters, int max )
{
	int color = 0xFFFFFF*iters/(double)max;
	return color;
}


// Show help message
void show_help()
{
	printf("Use: mandel [options]\n");
	printf("Where options are:\n");
	printf("-m <max>    The maximum number of iterations per point. (default=1000)\n");
	printf("-x <coord>  X coordinate of image center point. (default=0)\n");
	printf("-y <coord>  Y coordinate of image center point. (default=0)\n");
	printf("-s <scale>  Scale of the image in Mandlebrot coordinates (X-axis). (default=4)\n");
	printf("-W <pixels> Width of the image in pixels. (default=1000)\n");
	printf("-H <pixels> Height of the image in pixels. (default=1000)\n");
	printf("-o <file>   Set output file. (default=mandel.bmp)\n");
	printf("-h          Show this help text.\n");
	printf("\nSome examples are:\n");
	printf("mandel -x -0.5 -y -0.5 -s 0.2\n");
	printf("mandel -x -.38 -y -.665 -s .05 -m 100\n");
	printf("mandel -x 0.286932 -y 0.014287 -s .0005 -m 1000\n\n");
}
