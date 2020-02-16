#ifndef VIDEO_CREATER_HPP
#define VIDEO_CREATER_HPP


#include <ryulib/debug_tools.hpp>

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "secur32.lib")
#pragma comment(lib, "bcrypt.lib")

extern "C" {
#include <libavutil/avassert.h>
#include <libavutil/channel_layout.h>
#include <libavutil/opt.h>
#include <libavutil/mathematics.h>
#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#define STREAM_DURATION   10.0

#define STREAM_FRAME_RATE 25
#define PIXEL_SIZE        4

typedef struct OutputStream {
    AVStream *st;
    AVCodecContext *enc;

    int64_t next_pts;
    int samples_count;

    AVFrame *frame;
    AVFrame *tmp_frame;

    float t, tincr, tincr2;

    struct SwsContext *sws_ctx;
    struct SwrContext *swr_ctx;
} OutputStream;

class VideoCreater {
public:
    VideoCreater(const char* afilename, int awidth, int aheight)
        : filename(afilename), width(awidth), height(aheight)
    {
        int ret = 0;

        video_st.frame = alloc_picture(AV_PIX_FMT_YUV420P, width, height);
        if (video_st.frame == NULL) {
            DebugOutput::trace("createVideo - error alloc_picture(AV_PIX_FMT_YUV420P...)");
            return;
        }

        video_st.tmp_frame = alloc_picture(AV_PIX_FMT_BGRA, width, height);
        if (video_st.tmp_frame == NULL) {
            DebugOutput::trace("createVideo - error alloc_picture(AV_PIX_FMT_BGRA...)");
            return;
        }

        avformat_alloc_output_context2(&oc, NULL, NULL, filename);
        if (!oc) {
            DebugOutput::trace("createVideo - error avformat_alloc_output_context2");
            return;
        }

        fmt = oc->oformat;

        if (fmt->video_codec == AV_CODEC_ID_NONE) {
            DebugOutput::trace("createVideo - fmt->video_codec == AV_CODEC_ID_NONE");
            return;
        }

        if (add_video_stream() == false) return;
        if (open_video() == false) return;

        if (fmt->audio_codec == AV_CODEC_ID_NONE) {
            DebugOutput::trace("createVideo - fmt->audio_codec == AV_CODEC_ID_NONE");
            return;
        }

        if (add_audio_stream() == false) return;
        if (open_audio() == false) return;

        av_dump_format(oc, 0, filename, 1);

        if (!(fmt->flags & AVFMT_NOFILE)) {
            int ret = avio_open(&oc->pb, filename, AVIO_FLAG_WRITE);
            if (ret < 0) {
                DebugOutput::trace("createVideo - Could not open '%s'", filename);
                return;
            }
        }

        ret = avformat_write_header(oc, NULL);
        if (ret < 0) {
            DebugOutput::trace("createVideo - avformat_write_header");
            return;
        }
    }

    ~VideoCreater()
    {
        if (oc != NULL) {
            av_write_trailer(oc);
            close_stream(&video_st);
            close_stream(&audio_st);
            if (oc->pb != NULL) avio_closep(&oc->pb);
            avformat_free_context(oc);
        }
    }

    bool writeBitmap(void* bitmap)
    {
        int ret;
        AVCodecContext *c = video_st.enc;
        int got_packet = 0;
        AVPacket pkt = {0};

        if (av_frame_make_writable(video_st.frame) < 0) {
            DebugOutput::trace("writeBitmap - av_frame_make_writable < 0");
            return false;
        }

        if (!video_st.sws_ctx) {
            video_st.sws_ctx = sws_getContext(c->width, c->height,
                AV_PIX_FMT_BGRA,
                c->width, c->height,
                AV_PIX_FMT_YUV420P,
                SWS_BICUBIC, NULL, NULL, NULL);
            if (!video_st.sws_ctx) {
                DebugOutput::trace("writeBitmap - Could not initialize the conversion context");
                return false;
            }
        }

        memcpy(video_st.tmp_frame->data[0], bitmap, c->width * c->height * PIXEL_SIZE);
        sws_scale(video_st.sws_ctx, (const uint8_t * const *) video_st.tmp_frame->data,
            video_st.tmp_frame->linesize, 0, c->height, video_st.frame->data,
            video_st.frame->linesize);

        video_st.frame->pts = video_st.next_pts++;

        av_init_packet(&pkt);

        ret = avcodec_encode_video2(c, &pkt, video_st.frame, &got_packet);
        if (ret < 0) {
            DebugOutput::trace("Error encoding video frame");
            return false;
        }

        if (got_packet) {
            ret = write_frame(oc, &c->time_base, video_st.st, &pkt);
        } else {
            ret = 0;
        }

        if (ret < 0) {
            DebugOutput::trace("Error while writing video frame");
            return false;
        }

        return true;
    }

    bool isVideoTurn()
    {
        return av_compare_ts(
            video_st.next_pts, 
            video_st.enc->time_base,
            audio_st.next_pts, 
            audio_st.enc->time_base) <= 0;
    }

    bool isAudioTurn()
    {
        return ! isVideoTurn();
    }

    const char *filename;
    int width, height;

    OutputStream video_st = {0}, audio_st = {0};
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVCodec *audio_codec = nullptr;
    AVCodec *video_codec = nullptr;

private:
    AVFrame *alloc_audio_frame(enum AVSampleFormat sample_fmt, uint64_t channel_layout, int sample_rate, int nb_samples)
    {
        AVFrame *frame = av_frame_alloc();
        int ret;

        if (!frame) {
            DebugOutput::trace("alloc_audio_frame - Error allocating an audio frame");
            return NULL;
        }

        frame->format = sample_fmt;
        frame->channel_layout = channel_layout;
        frame->sample_rate = sample_rate;
        frame->nb_samples = nb_samples;

        if (nb_samples) {
            ret = av_frame_get_buffer(frame, 0);
            if (ret < 0) {
                DebugOutput::trace("alloc_audio_frame - Error allocating an audio buffer");
                return NULL;
            }
        }

        return frame;
    }

    AVFrame *alloc_picture(enum AVPixelFormat pix_fmt, int width, int height)
    {
        AVFrame *picture;
        int ret;

        picture = av_frame_alloc();
        if (!picture) {
            DebugOutput::trace("alloc_picture - Error allocating an video frame");
            return NULL;
        }

        picture->format = pix_fmt;
        picture->width  = width;
        picture->height = height;

        ret = av_frame_get_buffer(picture, 32);
        if (ret < 0) {
            DebugOutput::trace("alloc_picture - Could not allocate frame data.\n");
            return NULL;
        }

        return picture;
    }

    int write_frame(AVFormatContext *fmt_ctx, const AVRational *time_base, AVStream *st, AVPacket *pkt)
    {
        av_packet_rescale_ts(pkt, *time_base, st->time_base);
        pkt->stream_index = st->index;
        return av_interleaved_write_frame(fmt_ctx, pkt);
    }

    bool add_video_stream()
    {
        AVCodecContext *c;
        int i;

        video_codec = avcodec_find_encoder(fmt->video_codec);
        if (!(video_codec)) {
            DebugOutput::trace("Could not find encoder for '%s'", avcodec_get_name(fmt->video_codec));
            return false;
        }

        video_st.st = avformat_new_stream(oc, NULL);
        if (!video_st.st) {
            DebugOutput::trace("Could not allocate stream");
            return false;
        }

        video_st.st->id = oc->nb_streams-1;
        c = avcodec_alloc_context3(video_codec);
        if (!c) {
            DebugOutput::trace("Could not alloc an encoding context");
            return false;
        }
        video_st.enc = c;

        c->codec_id = fmt->video_codec;

        c->bit_rate = 1024 * 1024;
        c->width    = width;
        c->height   = height;
        video_st.st->time_base.num = 1;
        video_st.st->time_base.den = STREAM_FRAME_RATE;
        c->time_base       = video_st.st->time_base;

        c->gop_size      = 12;
        c->pix_fmt       = AV_PIX_FMT_YUV420P;
        if (c->codec_id == AV_CODEC_ID_MPEG2VIDEO) {
            c->max_b_frames = 2;
        }
        if (c->codec_id == AV_CODEC_ID_MPEG1VIDEO) {
            c->mb_decision = 2;
        }

        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        return true;
    }

    bool add_audio_stream()
    {
        AVCodecContext *c;
        int i;

        audio_codec = avcodec_find_encoder(fmt->audio_codec);
        if (!(audio_codec)) {
            DebugOutput::trace("Could not find encoder for '%s'", avcodec_get_name(fmt->audio_codec));
            return false;
        }

        audio_st.st = avformat_new_stream(oc, NULL);
        if (!audio_st.st) {
            DebugOutput::trace("Could not allocate stream");
            return false;
        }

        audio_st.st->id = oc->nb_streams-1;
        c = avcodec_alloc_context3(audio_codec);
        if (!c) {
            DebugOutput::trace("Could not alloc an encoding context");
            return false;
        }
        audio_st.enc = c;

        c->sample_fmt  = (audio_codec)->sample_fmts ?
            (audio_codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
        c->bit_rate    = 64000;
        c->sample_rate = 44100;
        if ((audio_codec)->supported_samplerates) {
            c->sample_rate = (audio_codec)->supported_samplerates[0];
            for (i = 0; (audio_codec)->supported_samplerates[i]; i++) {
                if ((audio_codec)->supported_samplerates[i] == 44100)
                    c->sample_rate = 44100;
            }
        }
        c->channels       = av_get_channel_layout_nb_channels(c->channel_layout);
        c->channel_layout = AV_CH_LAYOUT_STEREO;
        if ((audio_codec)->channel_layouts) {
            c->channel_layout = (audio_codec)->channel_layouts[0];
            for (i = 0; (audio_codec)->channel_layouts[i]; i++) {
                if ((audio_codec)->channel_layouts[i] == AV_CH_LAYOUT_STEREO)
                    c->channel_layout = AV_CH_LAYOUT_STEREO;
            }
        }
        c->channels        = av_get_channel_layout_nb_channels(c->channel_layout);
        audio_st.st->time_base.num = 1;
        audio_st.st->time_base.den = c->sample_rate;

        /* Some formats want stream headers to be separate. */
        if (oc->oformat->flags & AVFMT_GLOBALHEADER)
            c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        return true;
    }

    bool open_video()
    {
        int ret;
        AVCodecContext *c = video_st.enc;
        AVDictionary *opt = NULL;

        av_dict_copy(&opt, NULL, 0);
        av_opt_set(c->priv_data, "preset", "slow", 0);

        /* open the codec */
        ret = avcodec_open2(c, video_codec, &opt);
        av_dict_free(&opt);
        if (ret < 0) {
            DebugOutput::trace("Could not open video codec");
            return false;
        }

        /* allocate and init a re-usable frame */
        video_st.frame = alloc_picture(c->pix_fmt, c->width, c->height);
        if (!video_st.frame) {
            DebugOutput::trace("Could not allocate video frame");
            return false;
        }

        ret = avcodec_parameters_from_context(video_st.st->codecpar, c);
        if (ret < 0) {
            DebugOutput::trace("Could not copy the stream parameters");
            return false;
        }

        return true;
    }

    bool open_audio()
    {
        AVCodecContext *c;
        int nb_samples;
        int ret;
        AVDictionary *opt = NULL;

        c = audio_st.enc;

        /* open it */
        av_dict_copy(&opt, NULL, 0);
        ret = avcodec_open2(c, audio_codec, &opt);
        av_dict_free(&opt);
        if (ret < 0) {
            DebugOutput::trace("Could not open audio codec");
            return false;
        }

        /* init signal generator */
        audio_st.t     = 0;
        audio_st.tincr = 2 * M_PI * 110.0 / c->sample_rate;
        /* increment frequency by 110 Hz per second */
        audio_st.tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

        if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
            nb_samples = 10000;
        else
            nb_samples = c->frame_size;

        audio_st.frame     = alloc_audio_frame(c->sample_fmt, c->channel_layout,
            c->sample_rate, nb_samples);
        audio_st.tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, c->channel_layout,
            c->sample_rate, nb_samples);

        ret = avcodec_parameters_from_context(audio_st.st->codecpar, c);
        if (ret < 0) {
            DebugOutput::trace("Could not copy the stream parameters");
            return false;
        }

        audio_st.swr_ctx = swr_alloc();
        if (!audio_st.swr_ctx) {
            DebugOutput::trace("Could not allocate resampler context");
            return false;
        }

        /* set options */
        av_opt_set_int       (audio_st.swr_ctx, "in_channel_count",   c->channels,       0);
        av_opt_set_int       (audio_st.swr_ctx, "in_sample_rate",     c->sample_rate,    0);
        av_opt_set_sample_fmt(audio_st.swr_ctx, "in_sample_fmt",      AV_SAMPLE_FMT_S16, 0);
        av_opt_set_int       (audio_st.swr_ctx, "out_channel_count",  c->channels,       0);
        av_opt_set_int       (audio_st.swr_ctx, "out_sample_rate",    c->sample_rate,    0);
        av_opt_set_sample_fmt(audio_st.swr_ctx, "out_sample_fmt",     c->sample_fmt,     0);

        if ((ret = swr_init(audio_st.swr_ctx)) < 0) {
            DebugOutput::trace("Failed to initialize the resampling context");
            return false;
        }

        return true;
    }

    void close_stream(OutputStream *ost)
    {
        if (ost->enc != NULL) avcodec_free_context(&ost->enc);
        if (ost->frame != NULL) av_frame_free(&ost->frame);
        if (ost->tmp_frame != NULL) av_frame_free(&ost->tmp_frame);
        if (ost->sws_ctx != NULL) sws_freeContext(ost->sws_ctx);
        if (ost->swr_ctx != NULL) swr_free(&ost->swr_ctx);
    }
};


#endif  // VIDEO_CREATER_HPP