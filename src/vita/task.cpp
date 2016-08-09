/*
	Copyright (C) 2009-2015 DeSmuME team
	Copyright (C) 2015 Sergi Granell (xerpi)

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
#include <psp2/kernel/threadmgr.h>

#include <stdio.h>

#include "../utils/task.h"
#include "debug.h"

#include "config.h"

int getOnlineCores (void)
{
	if(UserConfiguration.threadedRendering)
		return 4;
	
	return 1;
}

class Task::Impl {
private:
	SceUID _thread;
	bool _isThreadRunning;

public:
	Impl();
	~Impl();

	void start(bool spinlock);
	void execute(const TWork &work, void *param);
	void* finish();
	void shutdown();

	//slock_t *mutex;
	SceUID condWork;
	TWork workFunc;
	void *workFuncParam;
	void *ret;
	bool exitThread;
};

#define EVENT_WORK_START 1
#define EVENT_WORK_END   2

static int taskProc(SceSize args, void *argp)
{
	Task::Impl *ctx = *(Task::Impl **)argp;
	do {

		while (ctx->workFunc == NULL && !ctx->exitThread) {
			sceKernelWaitEventFlag(ctx->condWork, EVENT_WORK_START,
				SCE_EVENT_WAITAND, NULL, NULL);
		}

		sceKernelClearEventFlag(ctx->condWork, ~EVENT_WORK_START);

		if (ctx->workFunc != NULL) {
			ctx->ret = ctx->workFunc(ctx->workFuncParam);
		} else {
			ctx->ret = NULL;
		}

		ctx->workFunc = NULL;
		sceKernelSetEventFlag(ctx->condWork, EVENT_WORK_END);
	} while(!ctx->exitThread);

	return 0;
}

Task::Impl::Impl()
{
	_isThreadRunning = false;
	workFunc = NULL;
	workFuncParam = NULL;
	ret = NULL;
	exitThread = false;

	condWork = sceKernelCreateEventFlag("desmume_cond_work", 0, 0, NULL);
}

Task::Impl::~Impl()
{
	shutdown();
	sceKernelDeleteEventFlag(condWork);
}

void Task::Impl::start(bool spinlock)
{
	if (this->_isThreadRunning) {
		return;
	}

	this->workFunc = NULL;
	this->workFuncParam = NULL;
	this->ret = NULL;
	this->exitThread = false;
	this->_thread = sceKernelCreateThread("desmume_task", taskProc,
		0x10000100, 0x1000, 0, 0, NULL);

	sceKernelClearEventFlag(condWork, ~(EVENT_WORK_START | EVENT_WORK_END));

	Task::Impl *_this = this;
	sceKernelStartThread(this->_thread, sizeof(_this), &_this);

	this->_isThreadRunning = true;
}

void Task::Impl::execute(const TWork &work, void *param)
{
	if (work == NULL || !this->_isThreadRunning) {
		return;
	}

	this->workFunc = work;
	this->workFuncParam = param;

	sceKernelSetEventFlag(condWork, EVENT_WORK_START);
}

void* Task::Impl::finish()
{
	void *returnValue = NULL;

	if (!this->_isThreadRunning) {
		return returnValue;
	}

	while (this->workFunc != NULL) {
		sceKernelWaitEventFlag(condWork, EVENT_WORK_END,
			SCE_EVENT_WAITAND, NULL, NULL);
	}

	sceKernelClearEventFlag(condWork, ~EVENT_WORK_END);

	returnValue = this->ret;

	return returnValue;
}

void Task::Impl::shutdown()
{
	if (!this->_isThreadRunning) {
		return;
	}

	this->workFunc = NULL;
	this->exitThread = true;

	sceKernelSetEventFlag(condWork, EVENT_WORK_START | EVENT_WORK_END);
	sceKernelWaitThreadEnd(_thread, NULL, NULL);
	sceKernelDeleteThread(_thread);

	this->_isThreadRunning = false;
}

void Task::start(bool spinlock) { impl->start(spinlock); }
void Task::shutdown() { impl->shutdown(); }
Task::Task() : impl(new Task::Impl()) {}
Task::~Task() { delete impl; }
void Task::execute(const TWork &work, void* param) { impl->execute(work,param); }
void* Task::finish() { return impl->finish(); }
