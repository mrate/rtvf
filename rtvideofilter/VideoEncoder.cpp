#include "VideoEncoder.h"
#include <iostream>

extern "C" {
#include <libavutil/mathematics.h>
#include <libavformat/avformat.h>
}

using namespace std;

#define STREAM_DURATION   5.0
#define STREAM_FRAME_RATE 25 /* 25 images/s */
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))
#define STREAM_PIX_FMT PIX_FMT_YUV420P /* default pix_fmt */

CVideoEncoder::CVideoEncoder() : img_convert_ctx( NULL ), sws_flags( SWS_BICUBIC ) {
	av_register_all();
}

CVideoEncoder::~CVideoEncoder() {
}

AVFrame* CVideoEncoder::AllocPicture( PixelFormat pix_fmt, int width, int height ) {
	AVFrame* picture;
	uint8_t* picture_buf;
	int size;

	picture = avcodec_alloc_frame();
	if( !picture ) {
		return NULL;
	}
	size = avpicture_get_size( pix_fmt, width, height );
	picture_buf = ( uint8_t* )av_malloc( size );
	if( !picture_buf ) {
		av_free( picture );
		return NULL;
	}
	avpicture_fill( ( AVPicture* )picture, picture_buf, pix_fmt, width, height );
	return picture;
}

bool CVideoEncoder::OpenVideo() {
	AVCodec* codec;
	AVCodecContext* c;

	c = video_st->codec;
	/* find the video encoder */
	codec = avcodec_find_encoder( c->codec_id );
	if( !codec ) {
		cerr << "codec not found" << endl;
		return false;
	}

	/* open the codec */
	if( avcodec_open( c, codec ) < 0 ) {
		cerr << "could not open codec" << endl;
		return false;
	}

	video_outbuf = NULL;
	if( !( oc->oformat->flags & AVFMT_RAWPICTURE ) ) {
		video_outbuf_size = 200000;
		video_outbuf = ( uint8_t* )av_malloc( video_outbuf_size );
	}

	/* allocate the encoded raw picture */
	picture = AllocPicture( c->pix_fmt, c->width, c->height );
	if( !picture ) {
		cerr << "Could not allocate picture" << endl;
		return false;
	}

	tmp_picture = NULL;
	//if (c->pix_fmt != PIX_FMT_YUV420P) {
	tmp_picture = AllocPicture( PIX_FMT_YUV420P, in_w, in_h );
	if( !tmp_picture ) {
		cerr << "Could not allocate temporary picture" << endl;
		return false;
	}
	//}
	return true;
}

AVStream* CVideoEncoder::AddVideoStream( CodecID codec_id ) {
	AVCodecContext* c;
	AVStream* st;

	st = av_new_stream( oc, 0 );
	if( !st ) {
		cerr << "Could not alloc stream" << endl;
		return NULL;
	}

	c = st->codec;
	c->codec_id = codec_id;
	c->codec_type = AVMEDIA_TYPE_VIDEO;

	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = out_w;
	c->height = out_h;
	/* time base: this is the fundamental unit of time (in seconds) in terms
	    of which frame timestamps are represented. for fixed-fps content,
	    timebase should be 1/framerate and timestamp increments should be
	    identically 1. */
	c->time_base.den = 25;
	c->time_base.num = 1;
	c->gop_size = 12; /* emit one intra frame every twelve frames at most */
	c->pix_fmt = PIX_FMT_YUV420P;
	if( c->codec_id == CODEC_ID_MPEG2VIDEO ) {
		/* just for testing, we also add B frames */
		c->max_b_frames = 2;
	}
	if( c->codec_id == CODEC_ID_MPEG1VIDEO ) {
		/* Needed to avoid using macroblocks in which some coeffs overflow.
		    This does not happen with normal video, it just happens here as
		    the motion of the chroma plane does not match the luma plane. */
		c->mb_decision = 2;
	}
	// some formats want stream headers to be separate
	if( !strcmp( oc->oformat->name, "mp4" ) || !strcmp( oc->oformat->name, "mov" ) || !strcmp( oc->oformat->name, "3gp" ) ) {
		c->flags |= CODEC_FLAG_GLOBAL_HEADER;
	}
	return st;
}

bool CVideoEncoder::Open( const char* name, int width_in, int height_in, int width_out, int height_out ) {
	strcpy( fname, name );
	in_w = width_in;
	in_h = height_in;
	out_w = width_out;
	out_h = height_out;

	fmt = av_guess_format( NULL, name, NULL );
	if( !fmt ) {
		fmt = av_guess_format( "mpeg", NULL, NULL );
	}

	if( !fmt ) {
		cerr << "Could not find suitable output format" << endl;
		return false;
	}

	oc = avformat_alloc_context();
	if( !oc ) {
		cerr << "Memory error" << endl;
		return false;
	}
	oc->oformat = fmt;
	snprintf( oc->filename, sizeof( oc->filename ), "%s", name );
	video_st = NULL;
	if( fmt->video_codec != CODEC_ID_NONE ) {
		video_st = AddVideoStream( fmt->video_codec );
	}

	if( av_set_parameters( oc, NULL ) < 0 ) {
		cerr << "Invalid output format parameters" << endl;
		return false;
	}

	if( video_st ) {
		OpenVideo();
	}

	if( !( fmt->flags & AVFMT_NOFILE ) ) {
		if( url_fopen( &oc->pb, name, URL_WRONLY ) < 0 ) {
			cerr << "Could not open '" << name << "'" << endl;
			return false;
		}
	}

	/* write the stream header, if any */
	av_write_header( oc );

	return true;
}

void CVideoEncoder::DumpFormat() {
	dump_format( oc, 0, fname, 1 );
}

void CVideoEncoder::CloseVideo() {
	avcodec_close( video_st->codec );
	av_free( picture->data[0] );
	av_free( picture );
	if( tmp_picture ) {
		av_free( tmp_picture->data[0] );
		av_free( tmp_picture );
	}
	av_free( video_outbuf );
}

void CVideoEncoder::Close() {
	if( video_st ) {
		CloseVideo();
	}

	/* write the trailer, if any */
	av_write_trailer( oc );

	/* free the streams */
	for( unsigned int i = 0; i < oc->nb_streams; i++ ) {
		av_freep( &oc->streams[i]->codec );
		av_freep( &oc->streams[i] );
	}

	if( !( fmt->flags & AVFMT_NOFILE ) ) {
		/* close the output file */
		url_fclose( oc->pb );
	}

	/* free the stream */
	av_free( oc );
}

bool CVideoEncoder::EncodeFrame( BITMAP* bmp ) {
	int out_size, ret;
	AVCodecContext* c;

	c = video_st->codec;

	// prevod z RGB32 na YUV420p
	if( img_convert_ctx == NULL ) {
		img_convert_ctx = sws_getContext( bmp->w, bmp->h, PIX_FMT_RGB32,
		                                  c->width, c->height, c->pix_fmt,
		                                  sws_flags, NULL, NULL, NULL );
		if( img_convert_ctx == NULL ) {
			cerr << "Cannot initialize the conversion context" << endl;
			return false;
		}
	}
	tmp_picture->data[0] = ( uint8_t* )bmp->line[0];
	tmp_picture->linesize[0] = in_w * 4;
	sws_scale( img_convert_ctx, tmp_picture->data, tmp_picture->linesize, 0, c->height, picture->data, picture->linesize );

	// ulozeni framu
	if( oc->oformat->flags & AVFMT_RAWPICTURE ) {
		/* raw video case. The API will change slightly in the near
		futur for that */
		AVPacket pkt;
		av_init_packet( &pkt );

		pkt.flags |= AV_PKT_FLAG_KEY;
		pkt.stream_index = video_st->index;
		pkt.data = ( uint8_t* )picture;
		pkt.size = sizeof( AVPicture );

		ret = av_write_frame( oc, &pkt );
	} else {
		/* encode the image */
		out_size = avcodec_encode_video( c, video_outbuf, video_outbuf_size, picture );
		/* if zero size, it means the image was buffered */
		if( out_size > 0 ) {
			AVPacket pkt;
			av_init_packet( &pkt );

			//if (c->coded_frame->pts != AV_NOPTS_VALUE)
			pkt.pts = av_rescale_q( c->coded_frame->pts, c->time_base, video_st->time_base );
			if( c->coded_frame->key_frame ) {
				pkt.flags |= AV_PKT_FLAG_KEY;
			}
			pkt.stream_index = video_st->index;
			pkt.data = video_outbuf;
			pkt.size = out_size;

			/* write the compressed frame in the media file */
			ret = av_write_frame( oc, &pkt );
		} else {
			ret = 0;
		}
	}
	if( ret != 0 ) {
		cerr << "Error while writing video frame" << endl;
		return false;
	}
	return true;
}
