#include <mercator_triangulator.h>


int main(){
	// Create triangulator and adc reader
	mercator_data_t* my_merc = mercator_triangulator_create(256);	

	mercator_triangulator_destroy(&my_merc);
	return EXIT_SUCCESS;
}
