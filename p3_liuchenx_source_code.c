#define _USE_MATH_DEFINES
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <omp.h>
#include <fstream>

// setting the number of threads:
// #ifdef COYOTEBOOST
// #define NUMT   4
// #else
// #define NUMT   3
// #endif

float Ranf( unsigned int *seedp,  float low, float high );
int Rani( unsigned int *seedp, int ilow, int ihigh );
float SQR( float x );

unsigned int seed = 0;

// float x = Ranf( &seed, -1.f, 1.f );

void GrainDeer();
void Grain();
void Watcher();
void coyoteBoost();

int	NowYear;		// 2020 - 2025
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int	NowNumDeer;		// number of deer in the current population
int NowCoyote;		// number of coyote in the current population

const float GRAIN_GROWS_PER_MONTH =		9.0;
const float ONE_DEER_EATS_PER_MONTH =		1.0;

const float AVG_PRECIP_PER_MONTH =		7.0;	// average
const float AMP_PRECIP_PER_MONTH =		6.0;	// plus or minus
const float RANDOM_PRECIP =			2.0;	// plus or minus noise

const float AVG_TEMP =				60.0;	// average
const float AMP_TEMP =				20.0;	// plus or minus
const float RANDOM_TEMP =			10.0;	// plus or minus noise

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

int main( int argc, char *argv[ ] )
{
	// starting date and time:
	NowMonth =    0;
	NowYear  = 2020;

	// starting state (feel free to change this if you want):
	NowNumDeer = 1;
	NowCoyote = 1;
	NowHeight =  1.;

	omp_set_num_threads(4);	// same as # of sections
	#pragma omp parallel sections
	{
		#pragma omp section
		{
			GrainDeer( );
		}

		#pragma omp section
		{
			Grain( );
		}

		#pragma omp section
		{
			coyoteBoost( );
		}

		#pragma omp section
		{
			Watcher( );
		}

		// #pragma omp section
		// {
		// 	MyAgent( );	// your own
		// }
	}       // implied barrier -- all functions must return in order
					// to allow any of them to get past here
	// return 0;
}


void GrainDeer()
{
	while( NowYear < 2026 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		int local_deer = NowNumDeer;
		// Compare deer number iwth coyote
		// Too few coyotes, spring and summer, deer boost
		if (NowCoyote * 2 < local_deer && NowMonth >=4 && NowMonth <= 9)
			local_deer = local_deer * 1.3;
		// Too many coyotes, deer number--
		if (NowCoyote > local_deer)
			local_deer--;
		// Compare deer number with carry capacity of grain
		if (local_deer > NowHeight)
			local_deer--;
		else if (local_deer < NowHeight)
			local_deer++;
		// Adjust deer number to reasonable level
		if (local_deer < 0)
			local_deer = 0;

		// DoneComputing barrier:
		#pragma omp barrier
		NowNumDeer = local_deer;

		// DoneAssigning barrier:
		#pragma omp barrier

		// DonePrinting barrier:
		#pragma omp barrier
	}
}


void Grain()
{
	while( NowYear < 2026 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		float local_height = NowHeight;
		float tempFactor = exp( -SQR( ( NowTemp - MIDTEMP ) / 10. ) );
		float precipFactor = exp(   -SQR(  ( NowPrecip - MIDPRECIP ) / 10.  )   );

	 	local_height += tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
	 	local_height -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH;

		if (local_height < 0.)
			local_height = 0;
		// DoneComputing barrier:
		#pragma omp barrier
		NowHeight = local_height;

		// DoneAssigning barrier:
		#pragma omp barrier

		// DonePrinting barrier:
		#pragma omp barrier
	}
}


void coyoteBoost()
{
	while( NowYear < 2026 )
	{
		// compute a temporary next-value for this quantity
		// based on the current state of the simulation:
		int local_coyote = NowCoyote;
		// Too many deers, coyote boost
		if (NowNumDeer >= 2*local_coyote)
			local_coyote = local_coyote + 2;
		else
			local_coyote = local_coyote - 1;
		// Adjust
		if (local_coyote < 0)
			local_coyote = 0;
		// DoneComputing barrier:
		#pragma omp barrier
		NowCoyote = local_coyote;

		// DoneAssigning barrier:
		#pragma omp barrier

		// DonePrinting barrier:
		#pragma omp barrier
	}
}


void Watcher() {
	while (NowYear < 2026) {
		// DoneComputing barrier:
		#pragma omp barrier
		// DoneAssigning barrier:
		#pragma omp barrier

		// Print info
		printf("%d-%d\t %6.2f\t %6.2f\t %d\t %d\t %6.2f\n", NowMonth+1, NowYear, (5./9.)*(NowTemp-32), NowPrecip*2.54, NowCoyote, NowNumDeer, NowHeight*2.54);

		// Set up next cycle environment variables
		// Increment time
		if (NowMonth == 11)
		{
			NowMonth = 0;
			NowYear++;
		}
		else
			NowMonth++;
		// Calculate temp and preciptation
		float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
		float temp = AVG_TEMP - AMP_TEMP * cos( ang );
		NowTemp = temp + Ranf( &seed, -RANDOM_TEMP, RANDOM_TEMP );

		float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
		NowPrecip = precip + Ranf( &seed,  -RANDOM_PRECIP, RANDOM_PRECIP );
		if( NowPrecip < 0. )
			NowPrecip = 0.;

		// DonePrinting barrier:
		#pragma omp barrier
	}
}

// Helper functions
float SQR( float x )
{
	return x*x;
}

float Ranf( unsigned int *seedp,  float low, float high )
{
	float r = (float) rand_r( seedp );              // 0 - RAND_MAX
  return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

int Ranf( unsigned int *seedp, int ilow, int ihigh )
{
  float low = (float)ilow;
  float high = (float)ihigh + 0.9999f;
  return (int)(  Ranf(seedp, low,high) );
}
