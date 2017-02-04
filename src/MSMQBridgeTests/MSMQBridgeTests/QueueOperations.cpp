#include "stdafx.h"
#include <memory>
#include <Rpc.h>
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

typedef HRESULT(*ptrCreateQueueDelete) (int queueId);
typedef HRESULT(*ptrCreateQueueDeleteByPath) (const wchar_t* queuePath);
typedef HRESULT(*ptrCreateQueue) (const wchar_t* queuePath, const wchar_t* queueLabel, int& queueId, int open);


template <class Func>
std::wstring GetGuidAsString(Func&&) {
	RPC_WSTR guidAsStr;
	std::wstring retval;
	std::unique_ptr<GUID> guid(new GUID);
	CoCreateGuid(guid.get());
	UuidToString(guid.get(), &guidAsStr);
	retval = std::wstring((LPTSTR)guidAsStr);

	return retval;
};

namespace MSMQBridgeTests {
	TEST_CLASS(QueueTests) {
public:

	TEST_METHOD(CreateQueueTest) {
		int id = 0;
		HRESULT retval;
		HINSTANCE hInstance;
		ptrCreateQueue queueCallback;
		auto queueName = std::wstring(L"Queue_").append(GetGuidAsString([&]() {}).substr(0, 10));
		auto queuePath = std::wstring(L".\\PRIVATE$\\").append(queueName);

		if ((hInstance = LoadLibrary(L"MSMQBridge.dll")) != NULL) {
			if ((queueCallback = (ptrCreateQueue)GetProcAddress(hInstance, "CreateQueue")) != NULL) {
				retval = queueCallback(queuePath.c_str(), L"Demo Queue", id, FALSE);
				Assert::IsTrue(SUCCEEDED(retval));
				FreeLibrary(hInstance);
			}
		}
	}

	TEST_METHOD(CreateQueueTestBadName) {
		int id = 0;
		HRESULT retval;
		HINSTANCE hInstance;
		ptrCreateQueue queueCallback;

		if ((hInstance = LoadLibrary(L"MSMQBridge.dll")) != NULL) {
			if ((queueCallback = (ptrCreateQueue)GetProcAddress(hInstance, "CreateQueue")) != NULL) {
				retval = queueCallback(L".\\PRIVATE\\MyQueue", L"Demo Queue - Bad Name", id, FALSE);
				Assert::IsFalse(SUCCEEDED(retval));
				FreeLibrary(hInstance);
			}
		}
	}

	TEST_METHOD(DeleteQueueTest) {
		int id = 0;
		HINSTANCE hInstance;
		ptrCreateQueue queueCallback;
		ptrCreateQueueDelete queueDeleteCallback;
		auto queueName = std::wstring(L"Queue_").append(GetGuidAsString([&]() {}).substr(0, 10));
		auto queuePath = std::wstring(L".\\PRIVATE$\\").append(queueName);

		if ((hInstance = LoadLibrary(L"MSMQBridge.dll")) != NULL) {
			if ((queueCallback = (ptrCreateQueue)GetProcAddress(hInstance, "CreateQueue")) != NULL &&
				(queueDeleteCallback = (ptrCreateQueueDelete)GetProcAddress(hInstance, "DeleteQueueById")) != NULL) {
				if (SUCCEEDED(queueCallback(queuePath.c_str(), L"Demo Queue - Delete", id, FALSE))) {
					Assert::IsTrue(SUCCEEDED(queueDeleteCallback(id)));
				}
				FreeLibrary(hInstance);
			}
		}
	}

	TEST_METHOD(DeleteQueueTestByPath) {
		int id = 0;
		HINSTANCE hInstance;
		ptrCreateQueue queueCallback;
		ptrCreateQueueDeleteByPath queueDeleteCallback;
		auto queueName = std::wstring(L"Queue_").append(GetGuidAsString([&]() {}).substr(0, 10));
		auto queuePath = std::wstring(L".\\PRIVATE$\\").append(queueName);

		if ((hInstance = LoadLibrary(L"MSMQBridge.dll")) != NULL) {
			if ((queueCallback = (ptrCreateQueue)GetProcAddress(hInstance, "CreateQueue")) != NULL &&
				(queueDeleteCallback = (ptrCreateQueueDeleteByPath)GetProcAddress(hInstance, "DeleteQueueByPath")) != NULL) {
				if (SUCCEEDED(queueCallback(queuePath.c_str(), L"Demo Queue - Delete By Path", id, FALSE))) {
					Assert::IsTrue(SUCCEEDED(queueDeleteCallback(queuePath.c_str())));
				}
				FreeLibrary(hInstance);
			}
		}
	}
	};
}