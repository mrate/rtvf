#include "TresholdFilter.h"

void CTreshFilter::Apply( BITMAP* bmp ) {
	if( !active ) {
		return;
	}

	long p;
	for( int y = 0; y < bmp->h; y++ ) {
		for( int x = 0; x < bmp->w; x++ ) {
			p = _getpixel32( bmp, x, y );
			if( color == 0 ) {
				if( getr32( p ) < tresh || getg32( p ) < tresh || getb32( p ) < tresh ) {
					_putpixel32( bmp, x, y, makecol( 0, 0, 0 ) );
				}
			} else {
				if( getr32( p ) > tresh || getg32( p ) > tresh || getb32( p ) > tresh ) {
					_putpixel32( bmp, x, y, makecol( 255, 255, 255 ) );
				}
			}
		}
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CTreshFilter();
	}

	void get_name( char* name ) {
		strcpy( name, treshFilterName );
	}
}
