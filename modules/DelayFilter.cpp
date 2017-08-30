#include "DelayFilter.h"

void CDelayFilter::Apply( BITMAP* bmp ) {
	int nindex = start_index - delay;
	if( nindex < 0 ) {
		nindex += MAX_FRAMES;
	}
	if( ++start_index == MAX_FRAMES ) {
		start_index = 0;
	}
	if( size < MAX_FRAMES ) {
		size++;
	}
	blit( bmp, dbmp[start_index], 0, 0, 0, 0, vw, vh );

	if( !active || delay == 0 ) {
		return ;
	}

	if( size >= delay ) {
		blit( dbmp[nindex], bmp, 0, 0, 0, 0, vw, vh );
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CDelayFilter();
	}

	void get_name( char* name ) {
		strcpy( name, delayFilterName );
	}
}
