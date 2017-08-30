#include "EdgeFilter.h"

void CEdgeFilter::Apply( BITMAP* bmp ) {
	if( !active || alpha == 0.0f ) {
		return;
	}

	int sum;
	int c;
	int32_t* i;
	if( method == 0 ) {
		long		sumX, sumY;
		for( int y = 0; y < vh; y++ ) {
			i = ( ( int32_t* )nbmp->line[y] );
			for( int x = 0; x < vw; x++ ) {
				sumX = 0;
				sumY = 0;

				if( y == 0 || y == vh - 1 ) {
					sum = 0;
				} else if( x == 0 || x == vw - 1 ) {
					sum = 0;
				} else {
					for( int i = -1; i <= 1; i++ ) {
						for( int j = -1; j <= 1; j++ ) {
							c = getpixel( bmp, x + i, y + j );
							c = ( getr32( c ) + getg32( c ) + getb32( c ) ) / 3;

							sumX = sumX + ( int )( c * GX[i + 1][j + 1] );
							sumY = sumY + ( int )( c * GY[i + 1][j + 1] );
						}
					}

					sum = abs( sumX ) + abs( sumY );
				}

				if( sum > 255 ) {
					sum = 255;
				}
				if( sum < 0 ) {
					sum = 0;
				}

				if( treshold > 0 ) {
					if( sum > treshold ) {
						sum = 255;
					} else {
						sum = 0;
					}
				}

				if( color == 0 ) {
					*i = makecol( 255 - sum, 255 - sum, 255 - sum );
				} else {
					*i = makecol( sum, sum, sum );
				}
				i++;
			}
		}
	} else {
		for( int y = 0; y < vh; y++ ) {
			i = ( ( int32_t* )nbmp->line[y] );
			for( int x = 0; x < vw; x++ ) {
				sum = 0;

				if( y == 0 || y == 1 || y == vh - 2 || y == vh - 1 ) {
					sum = 0;
				} else if( x == 0 || x == 1 || x == vw - 2 || x == vh - 1 ) {
					sum = 0;
				} else {
					for( int i = -2; i <= 2; i++ ) {
						for( int j = -2; j <= 2; j++ ) {
							c = getpixel( bmp, x + i, y + j );
							c = ( getr32( c ) + getg32( c ) + getb32( c ) ) / 3;
							sum += ( int )( c * MASK[i + 2][j + 2] );
						}
					}
				}
				if( sum > 255 ) {
					sum = 255;
				}
				if( sum < 0 ) {
					sum = 0;
				}

				if( treshold > 0 ) {
					if( sum > treshold ) {
						sum = 255;
					} else {
						sum = 0;
					}
				}

				if( color == 0 ) {
					*i = makecol( 255 - sum, 255 - sum, 255 - sum );
				} else {
					*i = makecol( sum, sum, sum );
				}
				i++;
			}
		}
	}
	if( alpha == 1.0f ) {
		blit( nbmp, bmp, 0, 0, 0, 0, bmp->w, bmp->h );
	} else {
		set_alpha_blender();
		set_trans_blender( alpha * 256, alpha * 256, alpha * 256, alpha * 256 );
		draw_trans_sprite( bmp, nbmp, 0, 0 );
		solid_mode();
	}
}

extern "C"
{
	CVideoFilter* create_instance() {
		return new CEdgeFilter();
	}

	void get_name( char* name ) {
		strcpy( name, edgeFilterName );
	}
}
