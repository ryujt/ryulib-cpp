#pragma once

#include <ryulib/SystemAudioCapture.hpp>
#include <ryulib/AudioCapture.hpp>
#include <ryulib/base.hpp>
#include <ryulib/debug_tools.hpp>
#include <ryulib/AudioIO.hpp>
#include <ryulib/ThreadQueue.hpp>

typedef struct AudioCaptureOption {
	int  mic_device_id = -1;
	bool use_system_audio = false;
	int channels = 1;
	int sample_rate = 44100;
	int sample_size = 4;
	int frames = 1024;
	int get_frame_size()
	{
		return channels * sample_size * frames;
	}
};

class AudioCapture {
public:
	AudioCapture()
	{

	}

	~AudioCapture()
	{
		if (mic_ != nullptr) delete mic_;
		if (silent_audio_ != nullptr) delete silent_audio_;
		if (audio_data_ != nullptr) delete audio_data_;
	}

	bool start(AudioCaptureOption option)
	{
		if (mic_ != nullptr) stop();

		option_ = option;

		mic_ = new AudioInput(option.channels, option.sample_rate, option.sample_size, option.frames);
		mic_->setOnError([&](const void* sender, int error_code) {
			do_error(mic_, error_code);
		});
		mic_->setOnData([&](const void* obj, const void* buffer, int buffer_size) {
			on_data(mic_, buffer, buffer_size);
		});

		if (silent_audio_ != nullptr) delete silent_audio_;
		silent_audio_ = new Memory(option.get_frame_size());

		if (audio_data_ != nullptr) delete audio_data_;
		audio_data_ = new Memory(option.get_frame_size());

		system_audio_.start(option.channels, option.sample_rate, option.sample_size, option.frames);

		return mic_->open(option.mic_device_id) == 0;
	}

	void stop()
	{
		on_data_ = nullptr;
		on_error_ = nullptr;

		system_audio_.stop();

		if (mic_ == nullptr) return;

		mic_->close();
		delete mic_;
		mic_ = nullptr;
	}

	bool isMicMuted()
	{
		return is_mic_muted_;
	}
	bool isSystemMuted()
	{
		return is_system_muted_;
	}

	float getMicVolume()
	{
		return volume_mic_;
	}
	float getSystemVolume()
	{
		return volume_system_;
	}

	void setMicMuted(bool value)
	{
		is_mic_muted_ = value;
	}
	void setSystemMuted(bool value)
	{
		is_system_muted_ = value;
	}

	void setMicVolume(float value)
	{
		volume_mic_ = value;
	}
	void setSystemVolume(float value)
	{
		volume_system_ = value;
	}

	void setOnData(DataEvent event)
	{
		on_data_ = event;
	}
	void setOnError(IntegerEvent event)
	{
		on_error_ = event;
	}

private:
	AudioInput* mic_ = nullptr;
	SystemAudioCapture system_audio_;

	Memory* silent_audio_ = nullptr;
	Memory* audio_data_ = nullptr;

	AudioCaptureOption option_;

	bool is_mic_muted_ = false;
	bool is_system_muted_ = false;
	float volume_mic_ = 1.0;
	float volume_system_ = 1.0;

	DataEvent on_data_ = nullptr;
	IntegerEvent on_error_ = nullptr;

	void do_error(AudioInput* sender, int error_code)
	{
		if (on_error_ != nullptr) on_error_(this, error_code);
	}

	void on_data(AudioInput* sender, const void* buffer, int buffer_size)
	{
		void* mic_audio = (void*) buffer;

		Memory* system_audio = system_audio_.getAudioData();
		 if (is_system_muted_ || (option_.use_system_audio == false) || (system_audio == nullptr)) {
		 	if (system_audio != nullptr) delete system_audio;
		 	system_audio = silent_audio_;
		 }

		if (is_mic_muted_) {
			mic_audio = silent_audio_->getData();
		};

		float* mic = (float*) mic_audio;
		float* system = (float*) system_audio->getData();
		float* dst = (float *) audio_data_->getData();
		for (int i = 0; i < buffer_size / sizeof(float); i++) {
			*dst = *mic * volume_mic_ + *system * volume_system_;
			mic++;
			system++;
			dst++;
		}

		if (system_audio != silent_audio_) delete system_audio;

		if (on_data_ != nullptr) on_data_(this, audio_data_->getData(), audio_data_->getSize());
	}
};
