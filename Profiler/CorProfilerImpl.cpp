// Copyright (c) .NET Foundation and contributors. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include "CorProfilerImpl.h"
#include "corhlpr.h"
#include "CComPtr.h"
#include <string>
#include <iostream>


CorProfilerImpl::CorProfilerImpl() : refCount(0), corProfilerInfo(nullptr),
destructor_called_(false),
rejit_worker_(&CorProfilerImpl::Processing, this)
{
}

CorProfilerImpl::~CorProfilerImpl()
{
	destructor_called_ = true;
	if (rejit_worker_.joinable())
	{
		rejit_worker_.join();
	}

    if (this->corProfilerInfo != nullptr)
    {
        this->corProfilerInfo->Release();
        this->corProfilerInfo = nullptr;
    }
}

HRESULT STDMETHODCALLTYPE CorProfilerImpl::Initialize(IUnknown *pICorProfilerInfoUnk)
{
    HRESULT queryInterfaceResult = pICorProfilerInfoUnk->QueryInterface(__uuidof(ICorProfilerInfo8), reinterpret_cast<void **>(&this->corProfilerInfo));

    if (FAILED(queryInterfaceResult))
    {
        return E_FAIL;
    }

    DWORD eventMask = COR_PRF_MONITOR_JIT_COMPILATION | COR_PRF_DISABLE_ALL_NGEN_IMAGES |
		COR_PRF_MONITOR_MODULE_LOADS | COR_PRF_DISABLE_INLINING |
		COR_PRF_ENABLE_STACK_SNAPSHOT | COR_PRF_ENABLE_REJIT;

    auto hr = this->corProfilerInfo->SetEventMask(eventMask);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfilerImpl::Shutdown()
{
	destructor_called_ = true;
	if (rejit_worker_.joinable())
	{
		rejit_worker_.join();
	}

    if (this->corProfilerInfo != nullptr)
    {
        this->corProfilerInfo->Release();
        this->corProfilerInfo = nullptr;
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CorProfilerImpl::JITCompilationStarted(FunctionID functionId, BOOL fIsSafeToBlock)
{
	HRESULT hr;
	mdToken token;
	ClassID classId;
	ModuleID moduleId;

	IfFailRet(this->corProfilerInfo->GetFunctionInfo(functionId, &classId, &moduleId, &token));

	CComPtr<IMetaDataImport2> metadataImport;
	IfFailRet(this->corProfilerInfo->GetModuleMetaData(moduleId, ofRead, IID_IMetaDataImport2, reinterpret_cast<IUnknown * *>(&metadataImport)));

	WCHAR nameBuffer[MAX_CLASS_NAME + 1]{ 0 };
	metadataImport->GetMethodProps(token, nullptr, nameBuffer, MAX_CLASS_NAME, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
	if (std::wstring(nameBuffer) == L"TestMethodForReJIT")
	{
		std::unique_lock<std::mutex> lock;
		for_rejit_.emplace_back(moduleId, token);
			std::cout << "for_rejit_ modified\n";
	}

	return S_OK;
}


HRESULT STDMETHODCALLTYPE CorProfilerImpl::GetReJITParameters(ModuleID moduleId, mdMethodDef methodId, ICorProfilerFunctionControl *pFunctionControl)
{
	std::cout << "GetReJITParameters\n";

    return S_OK;
}

void CorProfilerImpl::Processing()
{
	while (true) {
		if (destructor_called_.load()) {
			return;
		}

		std::vector<ModuleID> modules;
		std::vector<mdMethodDef> methods;
		{
			std::unique_lock<std::mutex> lock;
			for (auto pair : for_rejit_)
			{
				modules.push_back(pair.first);
				methods.push_back(pair.second);
			}
			for_rejit_.clear();
		}

		if (modules.size() > 0) {
			std::cout << "RequestReJit\n";
			HRESULT result = this->corProfilerInfo->RequestReJIT(modules.size(), modules.data(), methods.data());
			if (SUCCEEDED(result))
			{
				std::cout << "RequestReJit success\n";
			}
			else
			{
				std::cout << "RequestReJit fail\n";
			}
		}


		std::this_thread::sleep_for(std::chrono::seconds{ 10 });
	}
}
