#ifndef AUDIOZIP_HPP
#define AUDIOZIP_HPP

#include <ryulib/base.hpp>
#include <ryulib/AudioCapture.hpp>
#include <ryulib/AudioZipUtils.hpp>
#include <ryulib/AudioIO.hpp>
#include <ryulib/AudioEncoder.hpp>
#include <ryulib/debug_tools.hpp>

using namespace std;

class AudioZip {
public:
	/** AudioZip 생성자
	@param channels 오디오의 채널 수. 1: 모노, 2: 스테레오
	@param sampe_rate 오디오의 sampling rate. 초당 캡쳐할 샘플링(오디오의 데이터) 개수
	*/
	AudioZip(int channels, int sampe_rate)
	{
		audio_encoder_ = new AudioEncoder(channels, sampe_rate);
		audio_encoder_->setOnError([&](const void* obj, int error_code) {
			if (OnError_ != nullptr) OnError_(this, error_code);
		});

		audio_capture_ = new AudioCapture();
		audio_capture_->setOnError([&](const void* obj, int error_code) {
			if (OnError_ != nullptr) OnError_(this, error_code);
		});
		audio_capture_->setOnData([&](const void* obj, const void* buffer, int buffer_size) {
			if (OnSource_ != nullptr) OnSource_(this, buffer, buffer_size);
			audio_encoder_->add(buffer, buffer_size);
		});
	}

	~AudioZip()
	{
		delete audio_capture_;
		delete audio_encoder_;
	}

	/** 
	오디오 캡쳐 및 압축을 시작한다.
	@param device_id 오디오를 캡쳐할 디바이스 아이디 - 1은 기본 입력 장치 
	@return 성공하면 true, 오류가 발생하면 false가 리턴된다.
	*/
	bool start(int device_id = -1)
	{
		AudioCaptureOption option;
		option.mic_device_id = device_id;
		option.sample_rate = SAMPLE_RATE;
		option.frames = FRAMES_PER_BUFFER;
		return audio_capture_->start(option);
	}

	/** 
	오디오 캡쳐 및 압축을 중단한다. */
	void stop() 
	{
		audio_capture_->stop();
	}

	/**
	마이크 소리 소거 상태를 알려준다.
	@param value 마이크 소리 소거 상태 */
	bool isMicMuted()
	{ 
		return audio_capture_->isMicMuted();
	}

	/**
	시스템 오디오 소리 소거 상태를 알려준다.
	@param value 시스템 오디오 소리 소거 상태 */
	bool isSystemMuted()
	{
		return audio_capture_->isSystemMuted();
	}

	/**
	마이크(오디오 입력장치) 볼륨 상태를 알려준다.
	@return 1.0은 원음 크기를 의미하고 0.5는 50%의 볼륨이다. */
	float getMicVolume()
	{
		return audio_capture_->getMicVolume();
	}

	/**
	시스템 오디오 볼륨 상태를 알려준다.
	@return 1.0은 원음 크기를 의미하고 0.5는 50%의 볼륨이다. */
	float getSystemVolume()
	{
		return audio_capture_->getSystemVolume();
	}

	/**
	마이크 소리를 소거할지 결정한다.
	@param value 마이크 소리를 끌지 여부 */
	void setMicMuted(bool value)
	{
		audio_capture_->setMicMuted(value);
	}

	/**
	시스템 오디오 소리를 소거할지 결정한다.
	@param value 시스템 오디오 소리를 끌지 여부 */
	void setSystemMuted(bool value)
	{
		audio_capture_->setSystemMuted(value);
	}

	/**
	마이크(오디오 입력장치) 볼륨 크기를 결정한다.
	@param value 마이크에서 입력된 음량의 크기를 결정한다. 1.0은 원음 크기를 의미하고 0.5는 50%의 볼륨이다. */
	void setMicVolume(float value)
	{
		audio_capture_->setMicVolume(value);
	}

	/**
	시스템 오디오 볼륨 크기를 결정한다.
	@param value 마이크에서 입력된 음량의 크기를 결정한다. 1.0은 원음 크기를 의미하고 0.5는 50%의 볼륨이다. */
	void setSystemVolume(float value)
	{
		audio_capture_->setSystemVolume(value);
	}

	/**
	OnError 이벤트 핸들러를 지정한다.
	@param event 에러가 났을 때 실행될 이벤트 핸들러 */
	void setOnError(IntegerEvent event) { OnError_ = event; }

	/**
	OnSource 이벤트 핸들러를 지정한다.
	@param event 압축되기 전 원본 오디오가 발생했을 때 실행될 이벤트 핸들러 */
	void setOnSource(DataEvent event) { OnSource_ = event; }

	/**
	OnEncode 이벤트 핸들러를 지정한다.
	@param event 오디오가 압축되었을 때 실행될 이벤트 핸들러 */
	void setOnEncode(DataEvent event) { audio_encoder_->setOnEncode(event); }

private:
	AudioCapture* audio_capture_;
	AudioEncoder* audio_encoder_;

	int channels = 1;
	int sampe_rate = 44100;

	IntegerEvent OnError_ = nullptr;
	DataEvent OnSource_ = nullptr;
};


#endif  // AUDIOZIP_HPP