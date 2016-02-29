/*---------------------------------------------------------------------------------------------------------------------------------

	Author: Karel De Coster
	Date : 2016 - 02 - 08
	Calculates a position from raw sensor data using fft and triangulation.
	
---------------------------------------------------------------------------------------------------------------------------------*/
#ifndef _MERCATOR_TRIANGULATOR_H_
#define _MERCATOR_TRIANGULATOR_H_

#define REAL 0
#define IMAG 1

#include <assert.h>
#include <config.h>
#include <fftw3.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct mercator_data mercator_data_t;

void triangulate(double* d_vec, double* x_vec, double* y_vec, double* x, double* y);
/*  Calculates [x y] = ((transpose(A)*A)^-1)*transpose(A)*B
	d_vec = {D0, D1, D2};
	x_vec = {X0, X1, X2};
	y_vec = {Y0, Y1, Y2};
	x,y are the returned values
	where A:
	[X1-X0	Y1-Y0]					where X and Y are the position of a certain led and D is the distance to that led
	[X2-X0	Y2-Y0]
	and B :
	[ (D0² - D1² + X1² + Y1² - X0² - Y0²)/2 ]
	[ (D0² - D2² + X2² + Y2² - X0² - Y0²)/2]
	
 */	


double calculate_x(double* d, double* x, double* y);
/*
	Calculates the X value for the triangulation formula.
*/


double calculate_y(double* d, double* x, double* y);
/*
	Calculates the Y value for the triangulation formula.
*/

mercator_data_t* mercator_triangulator_create(uint32_t frame_size);
/*
	Returns a pointer to a mercator_data_t object.
*/

int mercator_triangulator_destroy(mercator_data_t** my_mercator);
/*
	Destroys the mercator_data_t object and cleans up to prevent memory leaks.
*/

double* mercator_triangulator_execute(double* data, mercator_data_t* my_mercator);
/*
	Calculates the position and returns it as a double array { x_position, y_position }.
*/

#endif
