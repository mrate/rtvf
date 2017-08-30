#include "WeightFilter.h"

void CWeightFilter::Apply( BITMAP* bmp ) {
	if( !active || speed == 0 ) {
		return ;
	}
	rw = rweight;
	gw = gweight;
	bw = bweight;
	sp = speed + 1;
	for( int y = 0; y < vh; y++ ) {
		i = ( ( int32_t* )bmp->line[y] );
		for( int x = 0; x < vw; x++ ) {
			*rw = ( speed * ( *rw ) + getr32( *i ) ) / sp;
			*gw = ( speed * ( *gw ) + getg32( *i ) ) / sp;
			*bw = ( speed * ( *bw ) + getb32( *i ) ) / sp;
			*i = makecol( *rw, *gw, *bw );

			rw++;
			gw++;
			bw++;
			i++;
		}
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CWeightFilter();
	}

	void get_name( char* name ) {
		strcpy( name, weightFilterName );
	}
}
