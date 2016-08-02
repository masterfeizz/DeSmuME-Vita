/*
	Copyright (C) 2009-2015 DeSmuME team

	This file is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 2 of the License, or
	(at your option) any later version.

	This file is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with the this software.  If not, see <http://www.gnu.org/licenses/>.
*/
#include <stdio.h>

#include "../utils/task.h"

int getOnlineCores (void)
{
	return 1;
}

class Task::Impl {
private:
	bool _isThreadRunning;
public:
	Impl();
	~Impl();

	void start(bool spinlock);
	void execute(const TWork &work, void *param);
	void* finish();
	void shutdown();

	//slock_t *mutex;
	TWork workFunc;
	void *workFuncParam;
	void *ret;
	bool exitThread;
};

static void taskProc(void *arg)
{
	Task::Impl *ctx = (Task::Impl *)arg;

	/*svcSleepThread(100000);

	do {

		svcWaitSynchronization(ctx->mutex, U64_MAX);

		if (ctx->workFunc != NULL && !ctx->exitThread) 
			ctx->ret = ctx->workFunc(ctx->workFuncParam);

		ctx->workFunc = NULL;

		svcReleaseMutex(ctx->mutex);

		svcSleepThread(10000);

	} while(!ctx->exitThread);*/
}

Task::Impl::Impl()
{
	_isThreadRunning = false;
	workFunc = NULL;
	workFuncParam = NULL;
	ret = NULL;
	exitThread = false;
}

Task::Impl::~Impl()
{
	shutdown();
}

int t = 0;

void Task::Impl::start(bool spinlock)
{
	if (this->_isThreadRunning) {
		return;
	}

	this->workFunc = NULL;
	this->workFuncParam = NULL;
	this->ret = NULL;
	this->exitThread = false;
	this->_isThreadRunning = true;

}

void Task::Impl::execute(const TWork &work, void *param)
{
	if (work == NULL || !this->_isThreadRunning) {
		return;
	}

	this->workFunc = work;
	this->workFuncParam = param;
}

void* Task::Impl::finish()
{
	void *returnValue = NULL;

	if (!this->_isThreadRunning) {
		return returnValue;
	}

	returnValue = this->ret;
	this->ret = NULL;

	return returnValue;
}

void Task::Impl::shutdown()
{

	if (!this->_isThreadRunning) {
		return;
	}

	this->workFunc = NULL;
	this->exitThread = true;

	this->_isThreadRunning = false;
}

void Task::start(bool spinlock) { impl->start(spinlock); }
void Task::shutdown() { impl->shutdown(); }
Task::Task() : impl(new Task::Impl()) {}
Task::~Task() { delete impl; }
void Task::execute(const TWork &work, void* param) { impl->execute(work,param); }
void* Task::finish() { return impl->finish(); }


