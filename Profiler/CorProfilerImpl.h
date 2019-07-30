// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <mutex>
#include "cor.h"
#include "corprof.h"
#include "CorProfiler.h"

class CorProfilerImpl : public CorProfiler
{
private:
    std::atomic<int> refCount;
    ICorProfilerInfo10* corProfilerInfo;
	std::vector<std::pair<ModuleID, mdMethodDef>> for_rejit_;
	std::mutex mutex_;
	std::atomic_bool destructor_called_;
	std::thread rejit_worker_;

public:
	CorProfilerImpl();
    virtual ~CorProfilerImpl();
    HRESULT STDMETHODCALLTYPE Initialize(IUnknown* pICorProfilerInfoUnk) override;
	HRESULT STDMETHODCALLTYPE Shutdown() override;
    HRESULT STDMETHODCALLTYPE JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock) override;
	HRESULT STDMETHODCALLTYPE GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl* pFunctionControl) override;
	void Processing();
};
