#include "FadeawayFilter.h"

void CFadeawayFilter::Apply( BITMAP* bmp ) {
	if( !active ) {
		return ;
	}
	if( size == 0 ) {
		blit( bmp, tmp, 0, 0, 0, 0, bmp->w, bmp->h );
	} else if( size < time ) {
		int t = ( int )( 256.0f * ( 1.0f - ( double )size / ( double )time ) );
		if( effect == 1 ) {
			int s = 10 + ( int )( 50.0f * ( double )size / ( double )time );
			for( int y = 0; y < bmp->h; y += s )
				for( int x = 0; x < bmp->w; x += s ) {
					stretch_blit( tmp, tmp2, x, y, 1, 1, x, y, s, s );
				}
		}
		set_alpha_blender();
		set_trans_blender( t, t, t, t );
		draw_trans_sprite( bmp, ( effect == 1 ? tmp2 : tmp ), 0, 0 );
		solid_mode();
	}
	if( size < time ) {
		size++;
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CFadeawayFilter();
	}

	void get_name( char* name ) {
		strcpy( name, fadeawayFilterName );
	}
}
