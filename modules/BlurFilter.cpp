#include "BlurFilter.h"

void CBlurFilter::Apply( BITMAP* bmp ) {
	if( !active || blur == 0 ) {
		return ;
	}

	int nindex = start_index - size;
	if( nindex < 0 ) {
		nindex += BLUR_FRAMES;
	}
	if( ++start_index == BLUR_FRAMES ) {
		start_index = 0;
	}
	if( size < BLUR_FRAMES ) {
		size++;
	}
	blit( bmp, dbmp[start_index], 0, 0, 0, 0, vw, vh );

	nindex = start_index;
	set_alpha_blender();
	for( int i = 0; i < blur; i++ ) {
		if( --nindex < 0 ) {
			nindex += BLUR_FRAMES;
		}
		set_trans_blender( 128 - i * 3, 128 - i * 3, 128 - i * 3, 128 - i * 3 );
		draw_trans_sprite( bmp, dbmp[nindex], 0, 0 );
	}
	solid_mode();
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CBlurFilter();
	}

	void get_name( char* name ) {
		strcpy( name, blurFilterName );
	}
}
