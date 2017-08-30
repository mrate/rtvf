#include "GrayscaleFilter.h"

void CGrayFilter::Apply( BITMAP* bmp ) {
	if( !active || perc == 0.0f ) {
		return ;
	}
	int32_t t;
	int32_t* i;
	int32_t r, g, b;
	for( int y = 0; y < bmp->h; y++ ) {
		i = ( ( int32_t* )bmp->line[y] );
		for( int x = 0; x < bmp->w; x++ ) {
			r = getr32( *i );
			g = getg32( *i );
			b = getb32( *i );
			t = ( r + g + b ) / 3;
			*i = makecol( r + perc * ( t - r ), g + perc * ( t - g ), b + perc * ( t - b ) );
			i++;
		}
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CGrayFilter();
	}

	void get_name( char* name ) {
		strcpy( name, grayFilterName );
	}
}
