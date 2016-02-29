#include <mercator_data_acquisition_unit.h>

int main(){
	FREQUENCY_t fs = FREQ_76_8kHz;
	uint32_t frame_size = 256;
	double vref = 2.5;
	if( mdau_create(frame_size, fs, vref) == 1 ){
		printf("pru_adc_reader_create() failed.\n");
		return EXIT_FAILURE;
	}
	double* samples = mdau_read_frame();
	int i = 0;
	printf("Sample		Value\n");
	for(i=0; i < frame_size; i++){
		printf("%d	%lf\n",i,samples[i]);
	}
			
	free(samples);
	samples = NULL;
	printf("__________________________________\n");
	samples = mdau_read_frame();
	printf("Sample		Value\n");
	for(i=0; i < frame_size; i++){
		printf("%d	%lf\n",i,samples[i]);
	}
			
	free(samples);
	samples = NULL;
	mdau_destroy();
	return EXIT_SUCCESS;
}
