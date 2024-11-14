#ifndef RYU_WORKER_HPP
#define RYU_WORKER_HPP

#include <ryulib/base.hpp>
#include <ryulib/SimpleThread.hpp>
#include <ryulib/SuspensionQueue.hpp>

using namespace std;
using namespace ryulib;

/** TaskOfWorker 클래스는 Worker 클래스가 처리할 작업의 정보를 담는 구조체입니다.
 * 작업의 종류, 텍스트 데이터, 바이너리 데이터 및 태그 정보를 포함합니다.
 */
class TaskOfWorker
{
public:
	int task;      ///< 작업의 종류를 나타내는 식별자
	void* data;    ///< 작업에 필요한 데이터의 포인터
	int size;      ///< data의 크기
	int tag;       ///< 작업을 구분하기 위한 태그 값
	string text;   ///< 작업에 필요한 문자열 데이터

	/** TaskOfWorker 생성자
	 * @param t 작업의 종류
	 * @param txt 작업에 필요한 문자열
	 * @param d 작업에 필요한 데이터의 포인터
	 * @param s 데이터의 크기
	 * @param g 작업 구분용 태그
	 */
	TaskOfWorker(int t, string txt, void* d, int s, int g) {
		task = t;
		text = txt;
		data = d;
		size = s;
		tag = g;
	}
};

/** Worker 클래스는 여러 스레드의 작업 요청을 단일 스레드에서 순차적으로 처리하기 위한 클래스입니다.
 * 내부적으로 작업 큐를 가지고 있어 스레드 안전한 방식으로 작업을 처리합니다.
 */
class Worker {
public:
	/** Worker 생성자
	 * 내부 작업 스레드를 생성하고 초기화합니다.
	 */
	Worker() {
		thread_ = new SimpleThread(on_thread_execute);
	}

	/** 작업 스레드를 안전하게 종료합니다.
	 * 현재 진행 중인 작업이 완료된 후 종료됩니다.
	 */
	void terminate()
	{
		thread_->terminate();
	}

	/** 작업 스레드를 즉시 강제 종료합니다.
	 * 주의: 현재 진행 중인 작업이 완료되지 않을 수 있습니다.
	 */
	void terminateNow()
	{
		thread_->terminateNow();
	}

	/** 작업 스레드를 종료하고 완전히 종료될 때까지 대기합니다.
	 */
	void terminateAndWait()
	{
		thread_->terminateAndWait();
	}

	/** 작업 스레드를 지정된 시간 동안 일시 중지합니다.
	 * @param millis 대기할 시간(밀리초)
	 */
	void sleep(int millis)
	{
		thread_->sleep(millis);
	}

	/** 단순 작업을 큐에 추가합니다.
	 * @param task 작업 식별자
	 */
	void add(int task) {
		TaskOfWorker* t = new TaskOfWorker(task, "", nullptr, 0, 0);
		queue_.push(t);
	}

	/** 문자열 데이터를 포함한 작업을 큐에 추가합니다.
	 * @param task 작업 식별자
	 * @param text 작업에 필요한 문자열
	 */
	void add(int task, string text) {
		TaskOfWorker* t = new TaskOfWorker(task, text, nullptr, 0, 0);
		queue_.push(t);
	}

	/** 데이터 포인터를 포함한 작업을 큐에 추가합니다.
	 * @param task 작업 식별자
	 * @param data 작업에 필요한 데이터 포인터
	 */
	void add(int task, void* data) {
		TaskOfWorker* t = new TaskOfWorker(task, "", data, 0, 0);
		queue_.push(t);
	}

	/** 크기가 지정된 데이터를 포함한 작업을 큐에 추가합니다.
	 * @param task 작업 식별자
	 * @param data 작업에 필요한 데이터 포인터
	 * @param size 데이터의 크기
	 */
	void add(int task, void* data, int size) {
		TaskOfWorker* t = new TaskOfWorker(task, "", data, size, 0);
		queue_.push(t);
	}

	/** 태그가 포함된 데이터 작업을 큐에 추가합니다.
	 * @param task 작업 식별자
	 * @param data 작업에 필요한 데이터 포인터
	 * @param size 데이터의 크기
	 * @param tag 작업 구분용 태그
	 */
	void add(int task, void* data, int size, int tag) {
		TaskOfWorker* t = new TaskOfWorker(task, "", data, size, tag);
		queue_.push(t);
	}

	/** 작업 큐가 비어있는지 확인합니다.
	 * @return 작업 큐가 비어있으면 true, 그렇지 않으면 false
	 */
	bool is_empty() { return queue_.is_empty(); }

	/** 작업 처리 이벤트 핸들러를 설정합니다.
	 * @param event 작업이 처리될 때 호출될 콜백 함수
	 */
	void setOnTask(TaskEvent event) { on_task_ = event; }

	/** 스레드 종료 이벤트 핸들러를 설정합니다.
	 * @param event 스레드가 종료될 때 호출될 콜백 함수
	 */
	void setOnTerminated(NotifyEvent event) { thread_->setOnTerminated(event); }

private:
	bool started_ = false;                              ///< 스레드 시작 여부를 나타내는 플래그
	SuspensionQueue<TaskOfWorker*> queue_;             ///< 작업을 저장하는 스레드 안전 큐
	TaskEvent on_task_ = nullptr;                      ///< 작업 처리 콜백 함수
	SimpleThread* thread_;                             ///< 작업을 처리하는 스레드
	
	/** 작업 처리 스레드의 실행 함수
	 * 큐에서 작업을 꺼내어 순차적으로 처리합니다.
	 */
	SimpleThreadEvent on_thread_execute = [&](SimpleThread* simpleThread) {
		while (simpleThread->isTerminated() == false) {
			TaskOfWorker* t = queue_.pop();
			if (on_task_ != nullptr) {
				on_task_(t->task, t->text, t->data, t->size, t->tag);
			}

			delete t;
		}
	};
};

#endif  // RYU_WORKER_HPP