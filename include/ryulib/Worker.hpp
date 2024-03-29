#ifndef RYU_WORKER_HPP
#define RYU_WORKER_HPP

#include <ryulib/base.hpp>
#include <ryulib/SimpleThread.hpp>
#include <ryulib/SuspensionQueue.hpp>

using namespace std;
using namespace ryulib;

class TaskOfWorker
{
public:
	int task;
	void* data;
	int size;
	int tag;
	string text;

	TaskOfWorker(int t, string txt, void* d, int s, int g) {
		task = t;
		text = txt;
		data = d;
		size = s;
		tag = g;
	}
};

class Worker {
public:
	Worker() {
		thread_ = new SimpleThread(on_thread_execute);
	}

	void terminate()
	{
		thread_->terminate();
	}

	void terminateNow()
	{
		thread_->terminateNow();
	}

	void terminateAndWait()
	{
		thread_->terminateAndWait();
	}

	void sleep(int millis)
	{
		thread_->sleep(millis);
	}

	void add(int task) {
		TaskOfWorker* t = new TaskOfWorker(task, "", nullptr, 0, 0);
		queue_.push(t);
	}

	void add(int task, string text) {
		TaskOfWorker* t = new TaskOfWorker(task, text, nullptr, 0, 0);
		queue_.push(t);
	}

	void add(int task, void* data) {
		TaskOfWorker* t = new TaskOfWorker(task, "", data, 0, 0);
		queue_.push(t);
	}

	void add(int task, void* data, int size) {
		TaskOfWorker* t = new TaskOfWorker(task, "", data, size, 0);
		queue_.push(t);
	}

	void add(int task, void* data, int size, int tag) {
		TaskOfWorker* t = new TaskOfWorker(task, "", data, size, tag);
		queue_.push(t);
	}

	bool is_empty() { return queue_.is_empty(); }

	void setOnTask(TaskEvent event) { on_task_ = event; }
	void setOnTerminated(NotifyEvent event) { thread_->setOnTerminated(event); }

private:
	bool started_ = false;
	SuspensionQueue<TaskOfWorker*> queue_;

	TaskEvent on_task_ = nullptr;

	SimpleThread* thread_;
	SimpleThreadEvent on_thread_execute = [&](SimpleThread * simpleThread) {
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