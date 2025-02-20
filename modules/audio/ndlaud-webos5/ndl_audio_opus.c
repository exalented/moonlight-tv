#include "ndl_common.h"
#include "stream/module/api.h"
#include "util/logging.h"

#include "base64.h"
#include "opus_constants.h"

#include <NDL_directmedia.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define audio_callbacks PLUGIN_SYMBOL_NAME(audio_callbacks)

#define FRAME_SIZE 240

static size_t write_opus_header(POPUS_MULTISTREAM_CONFIGURATION opusConfig, unsigned char *out);

static int init(int audioConfiguration, POPUS_MULTISTREAM_CONFIGURATION opusConfig, void *context, int arFlags) {
    media_info.audio.type = NDL_AUDIO_TYPE_OPUS;
    media_info.audio.opus.channels = opusConfig->channelCount;
    media_info.audio.opus.sampleRate = (double) opusConfig->sampleRate / 1000.0f;
#if DEBUG
    if (opusConfig->channelCount > 2) {
        unsigned char opusHead[sizeof(unsigned char) * OPUS_EXTRADATA_SIZE + 2 + OPUS_MAX_VORBIS_CHANNELS];
        size_t headSize = write_opus_header(opusConfig, opusHead);
        media_info.audio.opus.streamHeader = base64_encode(opusHead, headSize, NULL);
    }
#endif
    applog_d("NDLAud", "Reloading audio: channelCount=%d, sampleRate=%d, header=%s", opusConfig->channelCount,
             opusConfig->sampleRate,
             media_info.audio.opus.streamHeader);
    if (media_reload() != 0) {
        applog_e("NDLAud", "Failed to open audio: %s", NDL_DirectMediaGetError());
        return ERROR_AUDIO_OPEN_FAILED;
    }
    return 0;
}

static void cleanup() {
    media_unload();
    // free((void *)media_info.audio.opus.streamHeader);
    memset(&media_info.audio, 0, sizeof(media_info.audio));
}

static void play_sample(char *data, int length) {
    if (!media_loaded)
        return;
    if (NDL_DirectAudioPlay(data, length, 0) != 0) {
        applog_e("NDLAud", "Error playing sample");
    }
}

/**
 * Write opus stream header.
 * WARNING! This only works under little endian machine
 */
static size_t write_opus_header(POPUS_MULTISTREAM_CONFIGURATION opusConfig, unsigned char *out) {
    // See https://wiki.xiph.org/OggOpus#ID_Header.
    // Set magic signature.
    memcpy(&out[OPUS_EXTRADATA_LABEL_OFFSET], "OpusHead", 8);
    // Set Opus version.
    out[OPUS_EXTRADATA_VERSION_OFFSET] = 1;
    // Set channel count.
    out[OPUS_EXTRADATA_CHANNELS_OFFSET] = (uint8_t) opusConfig->channelCount;
    // Set pre-skip
    uint16_t skip = 0;
    memcpy(&out[OPUS_EXTRADATA_SKIP_SAMPLES_OFFSET], &skip, sizeof(uint16_t));
    // Set original input sample rate in Hz.
    uint32_t sampleRate = opusConfig->sampleRate;
    memcpy(&out[OPUS_EXTRADATA_SAMPLE_RATE_OFFSET], &sampleRate, sizeof(uint32_t));
    // Set output gain in dB.
    int16_t gain = 0;
    memcpy(&out[OPUS_EXTRADATA_GAIN_OFFSET], &gain, sizeof(int16_t));
    if (opusConfig->streams > 1) {
        // Channel mapping family 1 covers 1 to 8 channels in one or more streams,
        // using the Vorbis speaker assignments.
        out[OPUS_EXTRADATA_CHANNEL_MAPPING_OFFSET] = 1;
        out[OPUS_EXTRADATA_NUM_STREAMS_OFFSET] = opusConfig->streams;
        out[OPUS_EXTRADATA_NUM_COUPLED_OFFSET] = opusConfig->coupledStreams;
        memcpy(&out[OPUS_EXTRADATA_STREAM_MAP_OFFSET], opusConfig->mapping,
               sizeof(unsigned char) * opusConfig->streams);
        return OPUS_EXTRADATA_SIZE + 2 + opusConfig->streams;
    } else {
        // Channel mapping family 0 covers mono or stereo in a single stream.
        out[OPUS_EXTRADATA_CHANNEL_MAPPING_OFFSET] = 0;
        return OPUS_EXTRADATA_SIZE;
    }
}

MODULE_API AUDIO_RENDERER_CALLBACKS audio_callbacks = {
        .init = init,
        .cleanup = cleanup,
        .decodeAndPlaySample = play_sample,
        .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
