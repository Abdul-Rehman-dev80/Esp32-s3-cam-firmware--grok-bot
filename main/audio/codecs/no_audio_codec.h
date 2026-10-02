#ifndef NO_AUDIO_CODEC_H_
#define NO_AUDIO_CODEC_H_

#include <driver/gpio.h>
#include <cstdint>

class AudioCodec {
public:
    virtual ~AudioCodec() = default;
    virtual int Read(int16_t* buffer, int samples) = 0;
    virtual int Write(const int16_t* buffer, int samples) = 0;
    virtual void EnableInput(bool enable) = 0;
    virtual void EnableOutput(bool enable) = 0;
    virtual void SetOutputVolume(int volume) = 0;
    virtual int output_volume() const = 0;
};

class NoAudioCodecSimplex : public AudioCodec {
public:
    NoAudioCodecSimplex(int input_sample_rate, int output_sample_rate,
                        gpio_num_t spk_bclk, gpio_num_t spk_lrck, gpio_num_t spk_dout,
                        gpio_num_t mic_sck, gpio_num_t mic_ws, gpio_num_t mic_din);
    virtual ~NoAudioCodecSimplex();

    virtual int Read(int16_t* buffer, int samples) override;
    virtual int Write(const int16_t* buffer, int samples) override;
    virtual void EnableInput(bool enable) override;
    virtual void EnableOutput(bool enable) override;
    virtual void SetOutputVolume(int volume) override;
    virtual int output_volume() const override { return volume_; }

private:
    int input_sample_rate_;
    int output_sample_rate_;
    int volume_ = 80;
    bool input_enabled_ = false;
    bool output_enabled_ = false;

    void* i2s_tx_handle_ = nullptr;
    void* i2s_rx_handle_ = nullptr;
};

class NoAudioCodecDuplex : public AudioCodec {
public:
    NoAudioCodecDuplex(int input_sample_rate, int output_sample_rate,
                       gpio_num_t bclk, gpio_num_t ws, gpio_num_t dout, gpio_num_t din);
    virtual ~NoAudioCodecDuplex();

    virtual int Read(int16_t* buffer, int samples) override;
    virtual int Write(const int16_t* buffer, int samples) override;
    virtual void EnableInput(bool enable) override;
    virtual void EnableOutput(bool enable) override;
    virtual void SetOutputVolume(int volume) override;
    virtual int output_volume() const override { return volume_; }

private:
    int input_sample_rate_;
    int output_sample_rate_;
    int volume_ = 80;
    bool input_enabled_ = false;
    bool output_enabled_ = false;

    void* i2s_handle_ = nullptr;
};

#endif // NO_AUDIO_CODEC_H_
