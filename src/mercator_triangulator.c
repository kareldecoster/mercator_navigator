/*---------------------------------------------------------------------------------------------------------------------------------

	Author: Karel De Coster
	Date : 2016 - 02 - 08
	Calculates a position from raw sensor data using fft and triangulation.
	
---------------------------------------------------------------------------------------------------------------------------------*/

#include <mercator_triangulator.h>

struct mercator_data{
	fftw_complex* result;
	double* signal;
	uint32_t frame_size;
	fftw_plan plan;

};

/*-------------------------------------------------------------------------------------------------------------------------------*/

mercator_data_t* mercator_triangulator_create(uint32_t frame_size) {

/*-------------------------------------------------------------------------------------------------------------------------------*/
	/* Create your variables */
	mercator_data_t* my_mercator = malloc(sizeof(mercator_data_t));
	assert(my_mercator != NULL);	
	my_mercator->signal = (double*) fftw_malloc(sizeof(double) * frame_size);
    assert(my_mercator->signal != NULL);
    my_mercator->result = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * frame_size);
    assert(my_mercator->result != NULL);
    my_mercator->plan = fftw_plan_dft_r2c_1d(frame_size, my_mercator->signal, my_mercator->result, FFTW_MEASURE);
    my_mercator->frame_size = frame_size;

	return my_mercator;
}

/*-------------------------------------------------------------------------------------------------------------------------------*/

int mercator_triangulator_destroy(mercator_data_t** my_mercator) {

/*-------------------------------------------------------------------------------------------------------------------------------*/	
	/* Do some Cleanup */	
	fftw_free((*my_mercator)->signal);
	fftw_free((*my_mercator)->result);
    fftw_destroy_plan((*my_mercator)->plan);
    (*my_mercator)->signal = NULL;
    (*my_mercator)->result = NULL;
    (*my_mercator)->plan = NULL;
    fftw_cleanup();
    free(*my_mercator);
    *my_mercator = NULL;

    return 0;
    
}

/*-------------------------------------------------------------------------------------------------------------------------------*/

double* mercator_triangulator_execute(double* data, mercator_data_t* my_mercator){

/*-------------------------------------------------------------------------------------------------------------------------------*/
    uint16_t i = (my_mercator->frame_size) * PHI;
    uint16_t n = 0;
    for( n = 0; n < (my_mercator->frame_size); n++){
    	my_mercator->signal[n] = data[n];
    }
    fftw_execute(my_mercator->plan);
    n = 0;
    double di_sqr[4];
    /* Get the interesting frequencies and calculate the distance to each led squared */
    while (n < NUM_LEDS) {
        double Pr = M_PI * sqrt(my_mercator->result[i][REAL] * my_mercator->result[i][REAL] +
                          my_mercator->result[i][IMAG] * my_mercator->result[i][IMAG]) / (my_mercator->frame_size);
    	di_sqr[n] = sqrt(pow(H,2)* AREA * SENSITIVITY * AMP_GAIN / ( M_PI * Pr) );
    	n++;
    	i = 2 * i;
    }
    
	/* Sort the indices of your amplitudes ascending and change x_vec and y_vec accordingly */
	
	uint16_t index[NUM_LEDS] = {0,1,2,3}; //a = index of strongest signal, c = the weakest
	uint16_t a;
	for(i=0; i<NUM_LEDS; ++i){
		for(n=i+1; n < NUM_LEDS; ++n){
			if((sqrt(di_sqr[index[i]]))>(sqrt(di_sqr[index[n]]))){
				a = index[i];
				index[i] = index[n];
				index[n] = a;
			}		
		}		
	}
	double x_led[4] = {X_0, X_1, X_2, X_3};
	double y_led[4] = {Y_0, Y_1, Y_2, Y_3};
	double d_vec[3] = {sqrt(di_sqr[index[0]]), sqrt(di_sqr[index[1]]), sqrt(di_sqr[index[2]])};
	double x_vec[3] = {x_led[index[0]], x_led[index[1]], x_led[index[2]]};
	double y_vec[3] = {y_led[index[0]], y_led[index[1]], y_led[index[2]]};
	
	double* result = malloc(sizeof(double)*2);
	assert(result != NULL);

	triangulate(d_vec, x_vec, y_vec, &result[0],&result[1]);
	
	return result;
}


/*-------------------------------------------------------------------------------------------------------------------------------*/

void triangulate(double* d_vec, double* x_vec, double* y_vec, double* x, double* y){

/*-------------------------------------------------------------------------------------------------------------------------------*/
	*x = calculate_x(d_vec, x_vec, y_vec);
	*y = calculate_y(d_vec, x_vec, y_vec);

}


/*-------------------------------------------------------------------------------------------------------------------------------*/

double calculate_x(double* d, double* x, double* y){

/*-------------------------------------------------------------------------------------------------------------------------------*/
	double xpos = (-0.5) * (pow(d[0],2)*y[1] - pow(d[0],2)*y[2] - pow(d[1],2)*y[0] + pow(d[1],2)*y[2] + pow(d[2],2)*y[0] - pow(d[2],2)*y[1] - pow(x[0],2)*y[0] + pow(x[0],2)*y[2] + pow(x[1],2)*y[0] - pow(x[1],2)*y[2] - pow(x[2],2)*y[0] + pow(x[2],2)*y[1] - pow(y[0],2)*y[1] + pow(y[0],2)*y[2] + pow(y[1],2)*y[0] - y[0]*pow(y[2],2) - pow(y[1],2)*y[2] + y[1]*pow(y[2],2)) / ( x[0]*y[1] - x[0]*y[2] - x[1]*y[0] + x[1]*y[2] + x[2]*y[0] - x[2]*y[1]);
	
	return xpos;
		
}


/*-------------------------------------------------------------------------------------------------------------------------------*/

double calculate_y(double* d, double* x, double* y){

/*-------------------------------------------------------------------------------------------------------------------------------*/
	double ypos = (0.5) * (pow(d[0],2)*x[1] - pow(d[0],2)*x[2] - pow(d[1],2)*x[0] + pow(d[1],2)*x[2] + pow(d[2],2)*x[0] - pow(d[2],2)*x[1] - pow(x[0],2)*x[1] + pow(x[0],2)*x[2] + pow(x[1],2)*x[0] - pow(x[2],2)*x[0] + pow(y[1],2)*x[0] - pow(y[2],2)*x[0] - pow(x[1],2)*x[2] + pow(x[2],2)*x[1] - pow(y[0],2)*x[1] + x[1]*pow(y[2],2) + pow(y[0],2)*x[2] - x[2]*pow(y[1],2)) / ( x[0]*y[1] - x[0]*y[2] - x[1]*y[0] + x[1]*y[2] + x[2]*y[0] - x[2]*y[1]);
	
	return ypos;
	
}

