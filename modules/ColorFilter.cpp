#include "ColorFilter.h"

void CColorFilter::Apply( BITMAP* bmp ) {
	if( !active ) {
		return;
	}
	if( cr == 1.0 && cg == 1.0 && cb == 1.0 ) {
		return ;
	}

	int32_t* i;
	for( int y = 0; y < bmp->h; y++ ) {
		i = ( ( int32_t* )bmp->line[y] );
		for( int x = 0; x < bmp->w; x++ ) {
			p = *i;
			*i = makecol32( ( int )( cr * getr32( p ) ), ( int )( cg * getg32( p ) ), ( int )( cb * getb32( p ) ) );
			i++;
		}
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CColorFilter();
	}

	void get_name( char* n ) {
		strcpy( n, colorFilterName );
	}
}
