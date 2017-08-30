#include "PipeDecoder.h"

using namespace std;

CPipeDecoder::CPipeDecoder() : fd( 0 ), width( 720 ), height( 576 ) {
}

CPipeDecoder::~CPipeDecoder() {
}

bool CPipeDecoder::OpenPipe( const char* fname ) {
	if( fd ) {
		ClosePipe();
	}
	fd = open( fname, O_RDONLY );
	if( !fd ) {
		cerr << "Chyba otevirani pipe" << endl;
		return false;
	}
	return true;
}

void CPipeDecoder::ClosePipe() {
	close( fd );
	fd = 0;
}

bool CPipeDecoder::GetNextFrame( BITMAP* bmp ) {
	size_t l;
	int toread;
	for( int i = 0; i < height; i++ ) {
		toread = width;
		while( ( l = read( fd, bmp->line[i] + sizeof( long ) * ( width - toread ), sizeof( long ) * toread ) ) < width ) {
			if( l == 0 ) {
				return false;
			}
			toread -= l;
			usleep( 10 );
		}
	}

	return true;
}

bool CPipeDecoder::Init( char* line ) {
	string pipename( line );
	if( pipename.find( "pipe=" ) != 0 ) {
		cerr << "Chybi parametr 'pipe'" << endl;
		return false;
	}
	return OpenPipe( pipename.substr( 5, pipename.size() ).c_str() );
}

void CPipeDecoder::Close() {
	ClosePipe();
}

extern "C"
{
	CVideoDecoder* create_decoder() {
		return new CPipeDecoder();
	}

	void print_help() {
	}
}
