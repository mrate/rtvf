#include "VideoCapturer.h"
#include <iostream>
#include <sys/poll.h>

using namespace std;

CVideoCapturer::CVideoCapturer() : sws_flags( SWS_BICUBIC ), img_convert_ctx( NULL ), channel( 63 ) {
	av_register_all();
	//av_alloc_format_context();
	avformat_alloc_context();
	sws_opts = sws_getContext( 16, 16, PIX_FMT_RGB24, 16, 16, PIX_FMT_RGB24, sws_flags, NULL, NULL, NULL );
}

CVideoCapturer::~CVideoCapturer() {
	avcodec_close( pCodecCtx );
}

bool CVideoCapturer::StartCapture( int node ) {
	CodecID cid = CODEC_ID_DVVIDEO;
	width = 720;
	height = 576;
	pCodec = avcodec_find_decoder( cid );
	if( !pCodec ) {
		cerr << "Chyba nalezeni codecu" << endl;
		return false;
	}

	pCodecCtx = avcodec_alloc_context();
	if( !pCodecCtx ) {
		cerr << "Chyba alokace kontextu codecu" << endl;
		return false;
	}
	pCodecCtx->codec_id = cid;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->width = width;
	pCodecCtx->height = height;
	pCodecCtx->bit_rate = 0;
	pCodecCtx->time_base.den = 0;
	pCodecCtx->time_base.num = 0;
	pCodecCtx->gop_size = 12;
	pCodecCtx->pix_fmt = PIX_FMT_YUV420P;

	if( avcodec_open( pCodecCtx, pCodec ) < 0 ) {
		cerr << "Chyba otevreni codecu" << endl;
		return false;
	}

	pFrame = avcodec_alloc_frame();

	tbmp = create_bitmap_ex( 32, width, height );
	sws_flags = av_get_int( sws_opts, "sws_flags", NULL );
	img_convert_ctx = sws_getCachedContext( img_convert_ctx,
	                                        width, height, PIX_FMT_YUV420P,
	                                        width, height, PIX_FMT_RGB32, sws_flags, NULL, NULL, NULL );

	if( img_convert_ctx == NULL ) {
		cerr << "Chyba inicializace konvert. kontextu" << endl;
		return false;
	}

	//f = fopen("test.dv", "rb");
	raw_handle = raw1394_new_handle_on_port( 0 );

	if( raw_handle == NULL ) {
		cerr << "Chyba inicializace iec61883" << endl;
		return false;
	}

	raw_frame = iec61883_dv_fb_init( raw_handle, &process_frame, ( void* )this );
	if( raw_frame && iec61883_dv_fb_start( raw_frame, channel ) != 0 ) {
		cerr << "Chyba inicializace iec61883" << endl;
		return false;
	}

	return true;
}

int CVideoCapturer::process_frame( unsigned char* data, int len, int complete, void* callback_data ) {
	CVideoCapturer* inst = ( CVideoCapturer* )callback_data;
	if( complete == 0 ) {
		cout << " nekompletni frame" << endl;
	}
	return inst->ProcessFrame( data, len );
}

int CVideoCapturer::ProcessFrame( uint8_t* data, int len ) {
	int got_picture;
	//@@TODO:
	AVPacket packet;
	avcodec_decode_video2( pCodecCtx, pFrame, &got_picture, &packet );
	data = packet.data;
	//len = packet.len;
	if( got_picture ) {
		AVPicture pict;
		pict.data[0] = ( uint8_t* )tbmp->line[0];
		pict.linesize[0] = pCodecCtx->width * 4;
		sws_scale( img_convert_ctx, pFrame->data, pFrame->linesize,
		           0, pCodecCtx->height, pict.data, pict.linesize );
	}
	return 0;
}

void CVideoCapturer::StopCapture() {
	iec61883_dv_fb_close( raw_frame );
	raw1394_destroy_handle( raw_handle );
	destroy_bitmap( tbmp );
	//fclose(f);
}

bool CVideoCapturer::GetNextFrame( BITMAP* bmp ) {
	if( bmp == NULL ) {
		return false;
	}

	/*
	#define FRAME_SIZE 144000
	    int got_picture;
	    uint8_t buffer[FRAME_SIZE];
	    int length;
	    while(!feof(f))
	    {
	        length = fread(buffer, 1, FRAME_SIZE, f);
	        avcodec_decode_video(pCodecCtx, pFrame, &got_picture, buffer, length);
	            if(got_picture)
	            {
	                AVPicture pict;
	                pict.data[0] = (uint8_t*)bmp->line[0];
	                pict.linesize[0] = pCodecCtx->width*4;
	                sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize,
	                    0, pCodecCtx->height, pict.data, pict.linesize);
	                return true;
	            }
	    }
	*/
	struct pollfd pfd = {
fd:
		raw1394_get_fd( raw_handle ),
events:
		POLLIN | POLLPRI,
		revents: 0
	};
	int result = 0;

	do {
		if( poll( &pfd, 1, 100 ) > 0 && ( pfd.revents & POLLIN ) ) {
			result = raw1394_loop_iterate( raw_handle );
			blit( tbmp, bmp, 0, 0, 0, 0, width, height );
			return true;
		}
	} while( result == 0 );

	return false;
}

void CVideoCapturer::DumpFormat() {
}

bool CVideoCapturer::Init( char* line ) {
	string node( line );
	int inode = 0;
	if( node.find( "node=" ) == 0 ) {
		inode = atoi( node.substr( 5, node.size() ).c_str() );
	}
	return StartCapture( inode );
}

void CVideoCapturer::Close() {
	StopCapture();
}

extern "C"
{
	CVideoDecoder* create_decoder() {
		return new CVideoCapturer();
	}

	void print_help() {
		cout << "node - ieee-1394 node" << endl;
	}
}
