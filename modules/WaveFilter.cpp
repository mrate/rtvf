#include "WaveFilter.h"
#include <math.h>

void CWaveFilter::Apply( BITMAP* bmp ) {
	if( !active || height == 0 || width == 0 ) {
		return;
	}

	time_t ntime = time( NULL );
	if( speed == 0 ) {
		offset = 0;
	} else {
		offset += ( 6.2831852 / ( 52.0 - speed ) );
	}
	ltime = ntime;
	for( int y = 0; y < bmp->h; y += 2 ) {
		nwidth = ( int )( sin( ( double )y / ( double )height * 6.2831852 + offset ) * width );
		blit( bmp, tmp, 0, y, 0, 0, bmp->w, 2 );
		blit( tmp, bmp, nwidth, 0, 0, y, bmp->w - nwidth, 2 );
		blit( tmp, bmp, 0, 0, bmp->w - nwidth, y, nwidth, 2 );
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CWaveFilter();
	}

	void get_name( char* name ) {
		strcpy( name, waveFilterName );
	}
}
