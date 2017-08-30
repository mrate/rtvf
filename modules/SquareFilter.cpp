#include "SquareFilter.h"

void CSquareFilter::Apply( BITMAP* bmp ) {
	if( !active || size == 1 ) {
		return ;
	}
	blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h );
	for( int y = 0; y < bmp->h; y += size ) {
		for( int x = 0; x < bmp->w; x += size ) {
			stretch_blit( tmp, bmp, x, y, 1, 1, x, y, size, size );
		}
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CSquareFilter();
	}

	void get_name( char* name ) {
		strcpy( name, squareFilterName );
	}
}
