#include "no_audio_codec.h"
#include <esp_log.h>
#include <driver/i2s_std.h>

static const char* TAG = "NoAudioCodec";

// =============================================================================
// NoAudioCodecSimplex - Separate I2S channels for mic and speaker
// =============================================================================

NoAudioCodecSimplex::NoAudioCodecSimplex(int input_sample_rate, int output_sample_rate,
                                         gpio_num_t spk_bclk, gpio_num_t spk_lrck, gpio_num_t spk_dout,
                                         gpio_num_t mic_sck, gpio_num_t mic_ws, gpio_num_t mic_din)
    : input_sample_rate_(input_sample_rate)
    , output_sample_rate_(output_sample_rate) {

    ESP_LOGI(TAG, "Initializing NoAudioCodecSimplex");
    ESP_LOGI(TAG, "  Speaker: BCLK=%d, LRCK=%d, DOUT=%d", spk_bclk, spk_lrck, spk_dout);
    ESP_LOGI(TAG, "  Mic: SCK=%d, WS=%d, DIN=%d", mic_sck, mic_ws, mic_din);

    // Configure I2S TX for speaker (MAX98357A)
    i2s_chan_config_t tx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&tx_chan_cfg, (i2s_chan_handle_t*)&i2s_tx_handle_, nullptr);

    i2s_std_config_t tx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(output_sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = spk_bclk,
            .ws = spk_lrck,
            .dout = spk_dout,
            .din = GPIO_NUM_NC,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    i2s_channel_init_std_mode((i2s_chan_handle_t)i2s_tx_handle_, &tx_std_cfg);

    // Configure I2S RX for microphone (INMP441)
    i2s_chan_config_t rx_chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_1, I2S_ROLE_MASTER);
    i2s_new_channel(&rx_chan_cfg, nullptr, (i2s_chan_handle_t*)&i2s_rx_handle_);

    i2s_std_config_t rx_std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(input_sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_32BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = mic_sck,
            .ws = mic_ws,
            .dout = GPIO_NUM_NC,
            .din = mic_din,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    i2s_channel_init_std_mode((i2s_chan_handle_t)i2s_rx_handle_, &rx_std_cfg);

    ESP_LOGI(TAG, "NoAudioCodecSimplex initialized");
}

NoAudioCodecSimplex::~NoAudioCodecSimplex() {
    if (i2s_tx_handle_) {
        i2s_channel_disable((i2s_chan_handle_t)i2s_tx_handle_);
        i2s_del_channel((i2s_chan_handle_t)i2s_tx_handle_);
    }
    if (i2s_rx_handle_) {
        i2s_channel_disable((i2s_chan_handle_t)i2s_rx_handle_);
        i2s_del_channel((i2s_chan_handle_t)i2s_rx_handle_);
    }
}

int NoAudioCodecSimplex::Read(int16_t* buffer, int samples) {
    if (!i2s_rx_handle_ || !input_enabled_) return 0;

    size_t bytes_read = 0;
    i2s_channel_read((i2s_chan_handle_t)i2s_rx_handle_, buffer,
                     samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    return bytes_read / sizeof(int16_t);
}

int NoAudioCodecSimplex::Write(const int16_t* buffer, int samples) {
    if (!i2s_tx_handle_ || !output_enabled_) return 0;

    size_t bytes_written = 0;
    i2s_channel_write((i2s_chan_handle_t)i2s_tx_handle_, buffer,
                      samples * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    return bytes_written / sizeof(int16_t);
}

void NoAudioCodecSimplex::EnableInput(bool enable) {
    if (enable == input_enabled_) return;
    input_enabled_ = enable;

    if (i2s_rx_handle_) {
        if (enable) {
            i2s_channel_enable((i2s_chan_handle_t)i2s_rx_handle_);
        } else {
            i2s_channel_disable((i2s_chan_handle_t)i2s_rx_handle_);
        }
    }
}

void NoAudioCodecSimplex::EnableOutput(bool enable) {
    if (enable == output_enabled_) return;
    output_enabled_ = enable;

    if (i2s_tx_handle_) {
        if (enable) {
            i2s_channel_enable((i2s_chan_handle_t)i2s_tx_handle_);
        } else {
            i2s_channel_disable((i2s_chan_handle_t)i2s_tx_handle_);
        }
    }
}

void NoAudioCodecSimplex::SetOutputVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    volume_ = volume;
    // Note: MAX98357A gain is hardware-controlled, software volume applied to samples
}

// =============================================================================
// NoAudioCodecDuplex - Single I2S channel for both mic and speaker
// =============================================================================

NoAudioCodecDuplex::NoAudioCodecDuplex(int input_sample_rate, int output_sample_rate,
                                       gpio_num_t bclk, gpio_num_t ws,
                                       gpio_num_t dout, gpio_num_t din)
    : input_sample_rate_(input_sample_rate)
    , output_sample_rate_(output_sample_rate) {

    ESP_LOGI(TAG, "Initializing NoAudioCodecDuplex");
    ESP_LOGI(TAG, "  BCLK=%d, WS=%d, DOUT=%d, DIN=%d", bclk, ws, dout, din);

    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    i2s_new_channel(&chan_cfg, (i2s_chan_handle_t*)&i2s_handle_, (i2s_chan_handle_t*)&i2s_handle_);

    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(output_sample_rate),
        .slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = GPIO_NUM_NC,
            .bclk = bclk,
            .ws = ws,
            .dout = dout,
            .din = din,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    i2s_channel_init_std_mode((i2s_chan_handle_t)i2s_handle_, &std_cfg);

    ESP_LOGI(TAG, "NoAudioCodecDuplex initialized");
}

NoAudioCodecDuplex::~NoAudioCodecDuplex() {
    if (i2s_handle_) {
        i2s_channel_disable((i2s_chan_handle_t)i2s_handle_);
        i2s_del_channel((i2s_chan_handle_t)i2s_handle_);
    }
}

int NoAudioCodecDuplex::Read(int16_t* buffer, int samples) {
    if (!i2s_handle_ || !input_enabled_) return 0;

    size_t bytes_read = 0;
    i2s_channel_read((i2s_chan_handle_t)i2s_handle_, buffer,
                     samples * sizeof(int16_t), &bytes_read, portMAX_DELAY);
    return bytes_read / sizeof(int16_t);
}

int NoAudioCodecDuplex::Write(const int16_t* buffer, int samples) {
    if (!i2s_handle_ || !output_enabled_) return 0;

    size_t bytes_written = 0;
    i2s_channel_write((i2s_chan_handle_t)i2s_handle_, buffer,
                      samples * sizeof(int16_t), &bytes_written, portMAX_DELAY);
    return bytes_written / sizeof(int16_t);
}

void NoAudioCodecDuplex::EnableInput(bool enable) {
    input_enabled_ = enable;
}

void NoAudioCodecDuplex::EnableOutput(bool enable) {
    output_enabled_ = enable;
}

void NoAudioCodecDuplex::SetOutputVolume(int volume) {
    if (volume < 0) volume = 0;
    if (volume > 100) volume = 100;
    volume_ = volume;
}
