#include "NoiseFilter.h"

void CNoiseFilter::Apply( BITMAP* bmp ) {
	if( !active || noise == 0.0f ) {
		return;
	}

	int32_t* i, r, g, b, n;
	for( int y = 0, h = bmp->h, w = bmp->w; y < h; y++ ) {
		i = ( ( int32_t* )bmp->line[y] );
		for( int x = 0; x < w; x++ ) {
			n = rand() % 256;
			if( noise < 1.0f ) {
				r = ( ( 1.0f - noise ) * ( float )getr32( *i ) + noise * ( float )n );
				g = ( ( 1.0f - noise ) * ( float )getg32( *i ) + noise * ( float )n );
				b = ( ( 1.0f - noise ) * ( float )getb32( *i ) + noise * ( float )n );
				*i = makecol( r, g, b );
			} else {
				*i = makecol( n, n, n );
			}
			i++;
		}
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CNoiseFilter();
	}

	void get_name( char* name ) {
		strcpy( name, noiseFilterName );
	}
}
