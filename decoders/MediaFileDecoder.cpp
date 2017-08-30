#include "MediaFileDecoder.h"
#include <string>
#include <iostream>
#include <sys/times.h>

using namespace std;

CMediaFileDecoder::CMediaFileDecoder() : video_index( -1 ), sws_flags( SWS_BICUBIC ), img_convert_ctx( NULL ), fileOpened( false ) {
	av_register_all();
	//av_alloc_format_context();
	avformat_alloc_context();
	sws_opts = sws_getContext( 16, 16, PIX_FMT_RGB24, 16, 16, PIX_FMT_RGB24, sws_flags, NULL, NULL, NULL );
}

CMediaFileDecoder::~CMediaFileDecoder() {
	CloseFile();
	avcodec_close( pCodecCtx );
}

bool CMediaFileDecoder::OpenFile( string fname ) {
	CloseFile();

	fileName = fname;

	// otevreni videa
	video_index = -1;
	if( av_open_input_file( &pFmtCtx, fname.c_str(), NULL, 0, NULL ) != 0 ) {
		cerr << "Chyba otevirani souboru" << endl;
		return false;
	}

	// nalezeni streamu
	if( av_find_stream_info( pFmtCtx ) < 0 ) {
		cerr << "Chyba zjistovani stream_info " << endl;
		return false;
	}

	// nalezeni video streamu
	for( unsigned int i = 0; i < pFmtCtx->nb_streams; i++ ) {
		if( pFmtCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
			video_index = i;
			break;
		}
	}

	if( video_index == -1 ) {
		cerr << "Nenalezen video stream" << endl;
		return false;
	}

	// nalezeni codecu
	video_stream = pFmtCtx->streams[video_index];
	pCodecCtx = video_stream->codec;

	pCodec = avcodec_find_decoder( pCodecCtx->codec_id );
	if( !pCodec ) {
		cerr << "Chyba nalezeni codecu" << endl;
		return false;
	}

	if( avcodec_open( pCodecCtx, pCodec ) < 0 ) {
		cerr << "Chyba otevreni codecu" << endl;
		return false;
	}

	pFrame = avcodec_alloc_frame();

	sws_flags = av_get_int( sws_opts, "sws_flags", NULL );
	img_convert_ctx = sws_getCachedContext( img_convert_ctx,
	                                        pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
	                                        pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB32, sws_flags, NULL, NULL, NULL );

	if( img_convert_ctx == NULL ) {
		cerr << "Cannot initialize the conversion context" << endl;
		return false;
	}

	pts = ( float )pCodecCtx->time_base.den / ( float )pCodecCtx->time_base.num;
	toWait = 1000 / pts;
	lastFrameTime = 0;

	fileOpened = true;
	return true;
}

void CMediaFileDecoder::CloseFile() {
	if( fileOpened ) {
		av_free( pFrame );
		av_close_input_file( pFmtCtx );
		sws_freeContext( img_convert_ctx );
	}
	fileOpened = false;
}

bool CMediaFileDecoder::GetNextFrame( BITMAP* bmp ) {
	if( !fileOpened ) {
		return false;
	}
	if( bmp == NULL ) {
		return false;
	}

	int got_picture;
	tms tm;
	long time;
	while( av_read_frame( pFmtCtx, &pkt ) >= 0 ) {
		if( pkt.stream_index == video_index ) {
			avcodec_decode_video2( pCodecCtx, pFrame, &got_picture, &pkt );
			if( got_picture ) {
				AVPicture pict;
				pict.data[0] = ( uint8_t* )bmp->line[0];
				pict.linesize[0] = pCodecCtx->width * 4;
				sws_scale( img_convert_ctx, pFrame->data, pFrame->linesize,
				           0, pCodecCtx->height, pict.data, pict.linesize );
				av_free_packet( &pkt );

				time = times( &tm ) * 10;
				if( time - lastFrameTime < toWait ) {
					usleep( ( toWait - ( time - lastFrameTime ) ) * 1000 );
				}
				lastFrameTime = times( &tm ) * 10;
				return true;
			}
		}
		av_free_packet( &pkt );
	}

	return false;
}

void CMediaFileDecoder::DumpFormat() {
	if( fileOpened ) {
		dump_format( pFmtCtx, 0, fileName.c_str(), 0 );
	} else {
		cout << "No opened file" << endl;
	}
}

bool CMediaFileDecoder::Init( char* line ) {
	string fname( line );
	if( fname.find( "file=" ) != 0 ) {
		cerr << "Chybi parametr 'file'" << endl;
		return false;
	}
	fname = fname.substr( 5, fname.size() );
	return OpenFile( fname );
}

void CMediaFileDecoder::Close() {
	CloseFile();
}

extern "C"
{
	CVideoDecoder* create_decoder() {
		return new CMediaFileDecoder();
	}

	void print_help() {
		cout << "Media file decoder:" << endl;
		cout << "  file - media file" << endl;
	}
}
