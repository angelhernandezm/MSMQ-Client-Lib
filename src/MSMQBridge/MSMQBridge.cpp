// MSMQBridge.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "MSMQBridge.h"

typedef pair <long, std::wstring> HResult_Pair;
typedef pair <int, QueueInformation> Queue_Pair;

std::hash_map<int, QueueInformation> CMSMQBridge::Queues;

template <class iterator, class function, class predicate>
function for_each_if (iterator first, iterator last, predicate pred, function f) {
	for (; first != last; ++first) {
		if (pred (*first))
			f (*first);
	}

	return f;
};

// This is the constructor of a class that has been exported.
// see MSMQBridge.h for the class definition
CMSMQBridge::CMSMQBridge()
{
	return;
}

CMSMQBridge::CMSMQBridge(bool isFullyVerbose) {
	verbosityEnabled = isFullyVerbose;
}

CMSMQBridge::~CMSMQBridge() {
	if (Queues.size() > 0) {
		std::for_each(Queues.begin(), Queues.end(), [&] (Queue_Pair item) {

			//Queues.at(

			if (item.second.IsOpen()) 
				MQCloseQueue(item.second.queueHandle);
		});
	}
}

QueueOpResult CMSMQBridge::DeleteQueue(int nQueueId) {
	HRESULT ret;
	QueueOpResult retval;
	retval.isSuccess = false;
	wchar_t szFormatName[MAX_PATH];
	DWORD cbFormatLength = MAX_PATH;
	hash_map<int, QueueInformation>::const_iterator found = Queues.find(nQueueId);

	if (Queues.size() > 0 || found != Queues.end()) {
		auto pathName = found->second.Queuepath.c_str();
		auto result = MQPathNameToFormatName(pathName, &szFormatName[0], &cbFormatLength);
		if (SUCCEEDED((ret = MQDeleteQueue(szFormatName)))) {
			retval.hr = S_OK;
			retval.isSuccess = true;
			Queues.erase(found);
		} else {
			CreateFailedQueueOpResult(retval, ret);
		}
	} else {
		retval.isSuccess = false;
		retval.errMessage = L"Queue is empty or Queue Id wasn't found";
	}

	return retval;
}

bool CMSMQBridge::Item_Exist(bool checkBoth, QueueInformation& item, std::wstring expr1, std::wstring expr2 = L"") {
	auto retval = false;

	if (Queues.size() > 0) {
		for (auto nItem = Queues.begin(); nItem != Queues.end(); ++nItem) {
			auto x =  (QueueInformation) nItem._Ptr->_Myval.second;
			if (checkBoth) {
				if (x.Queuepath.compare(expr1) == 0 || x.Label.compare(expr2) == 0) {
					item = x;
					retval = true;
					break;
				}
			} else {
				if (x.Queuepath.compare(expr1) == 0) {
					item = x;
					retval = true;
					break;
				}
			}
		}
	}
	return retval;
}

QueueOpResult CMSMQBridge::DeleteQueue(std::wstring szQueuePath) {
	HRESULT ret;
	QueueOpResult retval;
	QueueInformation item;
	retval.isSuccess = false;
	wchar_t szFormatName[MAX_PATH];
	DWORD cbFormatLength = MAX_PATH;
	bool exist = Item_Exist(false, item, szQueuePath);

	if (exist) {
		auto result = MQPathNameToFormatName(item.Queuepath.c_str(), &szFormatName[0], &cbFormatLength);
		if (SUCCEEDED((ret = MQDeleteQueue(szFormatName)))) {
			retval.hr = S_OK;
			retval.isSuccess = true;
			Queues.erase(Queues.find(item.QueueId));
		} else {
			CreateFailedQueueOpResult(retval, ret);
		}
	} else {
		retval.isSuccess = false;
		retval.errMessage = L"Queue is empty or Queue Id wasn't found";
	}
	return retval;
}

QueueOpResult CMSMQBridge::PrepareMessage(std::wstring label, std::wstring messageBody, MQMSGPROPS& message) {
	auto nPropId = 0;
	QueueOpResult retval;
	retval.isSuccess = false;
	MSGPROPID aPropId[MAX_MESSAGE_PROPERTIES];
	//HRESULT aMsgStatus[MAX_MESSAGE_PROPERTIES];
	MQPROPVARIANT aMsgPropVar[MAX_MESSAGE_PROPERTIES];

	aPropId[nPropId] = PROPID_M_LABEL;
	aMsgPropVar[nPropId].vt = VT_LPWSTR;
	aMsgPropVar[nPropId].pwszVal =  (LPWSTR)  (!label.empty() ? label.c_str() : DEFAULT_LABEL);
	
	return retval;
}

QueueOpResult CMSMQBridge::SendMessageToQueue(std::wstring pathOrId, std::wstring label, std::wstring message) {
	QueueOpResult retval;
	QueueInformation item;
	retval.isSuccess = false;
	hash_map<int, QueueInformation>::const_iterator found;
	
	if (Queues.size() > 0) {
		if (auto id = _wtoi(pathOrId.c_str()) == 0)  {
			found = Queues.find(id);
			item = found._Ptr->_Myval.second;
		}
		else Item_Exist(false, item, pathOrId);

		if (item.IsOpen()) {


		}
	}
	return retval;
}


QueueOpResult CMSMQBridge::CreateQueue(std::wstring szQueuePath, std::wstring szQueueLabel, bool isTransactional, int& nQueueId, bool open) {
	HRESULT ret; 
	bool exist = false;
	QueueOpResult retval;
	QueueInformation item;
	retval.isSuccess = false;
	QueueInformation newQueue;
	wchar_t szFormatName[MAX_PATH];
	DWORD cbFormatLength = MAX_PATH;
	auto flags = MQ_RECEIVE_ACCESS | MQ_ADMIN_ACCESS | MQ_SEND_ACCESS;
	int vtValues[MAX_QUEUE_PROPERTIES] = {VT_LPWSTR, VT_UI1, VT_LPWSTR};
	int queueProps[MAX_QUEUE_PROPERTIES] = {PROPID_Q_PATHNAME, PROPID_Q_TRANSACTION, PROPID_Q_LABEL};

	if (!szQueuePath.empty() && !szQueueLabel.empty()) {
		newQueue.Label = szQueueLabel;
		newQueue.Queuepath = szQueuePath;
		newQueue.IsTransactional = isTransactional;
		newQueue.queueProperties.cProp = MAX_QUEUE_PROPERTIES;

		for (auto nIndex = 0; nIndex < MAX_QUEUE_PROPERTIES; nIndex++) {
			newQueue.QueuePropId[nIndex] = queueProps[nIndex];
			newQueue.QueuePropVar[nIndex].vt = vtValues[nIndex]; 

			if (nIndex == 0 || nIndex == 2)
				newQueue.QueuePropVar[nIndex].pwszVal = (LPWSTR)((nIndex == 0 ? szQueuePath.c_str() : szQueueLabel.c_str()));
			else newQueue.QueuePropVar[nIndex].bVal = (UCHAR) isTransactional;
		}

		exist = Item_Exist(true, item, szQueuePath, szQueueLabel);
		newQueue.queueProperties.aPropID = newQueue.QueuePropId;
		newQueue.queueProperties.aStatus = newQueue.QueueStatus;
		newQueue.queueProperties.aPropVar = newQueue.QueuePropVar;


		if (!exist) {
			if (SUCCEEDED(ret = MQCreateQueue(NULL, &newQueue.queueProperties, &szFormatName[0], &cbFormatLength))) {
				retval.hr = S_OK;
				retval.isSuccess = true;
				nQueueId = newQueue.QueueId = Queues.size() + 1;
				newQueue.FormatName = std::wstring(szFormatName);

				// Is it required to open it?
				if (open) {
					if ((FAILED(ret = MQOpenQueue(szFormatName, flags, MQ_DENY_NONE, &newQueue.queueHandle)))) {
						retval.hr = ret;
						retval.isSuccess = false;
						CreateFailedQueueOpResult(retval, ret);
					}
				}

				if (retval.isSuccess)
					Queues.insert(Queue_Pair(nQueueId, newQueue));
			} else {
				CreateFailedQueueOpResult(retval, ret);
			}
		} else {
			retval.isSuccess = false;
			retval.errMessage = L"Path and/or Label exists already";
		}
	}

	return retval;
}



void CMSMQBridge::CreateFailedQueueOpResult(QueueOpResult& valueToBeReturned, const HRESULT& hr) {
	valueToBeReturned.hr = hr;
	valueToBeReturned.isSuccess = false;
	valueToBeReturned.errMessage = _com_error(hr).ErrorMessage();

	if (verbosityEnabled) {
		LogException(std::wstring(L"MSMQBridge Component has Full Verbosity enabled\n")
			.append(L"Error Message: ").append(valueToBeReturned.errMessage)
			.append(L" - Description: ").append(GetHResultDescription(hr)));
	}
}


void CMSMQBridge::LogException(std::wstring exception) {
	HANDLE hEventLog;
	LPWSTR entryDetails[EVENT_LOG_ENTRY_COUNT] = {(WCHAR*) EVENT_LOG_NAME, (WCHAR*) exception.c_str()};

	if ((hEventLog = OpenEventLog(NULL, EVENT_LOG_NAME)) != NULL) 
		ReportEvent(hEventLog, EVENTLOG_INFORMATION_TYPE, 0x1, 0x2, NULL, 2, NULL, (LPCWSTR*)entryDetails, NULL);

	CloseEventLog(hEventLog);
}



std::wstring CMSMQBridge::GetHResultDescription(const HRESULT& hr) {
	std::wstring retval;
	std::hash_map<long, std::wstring> Descriptions;
	Descriptions.insert(HResult_Pair(MQ_ERROR, L"MQ_ERROR"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ACCESS_DENIED, L"MQ_ERROR_ACCESS_DENIED"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_BAD_SECURITY_CONTEXT, L"MQ_ERROR_BAD_SECURITY_CONTEXT"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_BAD_XML_FORMAT, L"MQ_ERROR_BAD_XML_FORMAT"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_BUFFER_OVERFLOW, L"MQ_ERROR_BUFFER_OVERFLOW "));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_CREATE_CERT_STORE, L"MQ_ERROR_CANNOT_CREATE_CERT_STORE"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_CREATE_HASH_EX, L"MQ_ERROR_CANNOT_CREATE_HASH_EX"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_CREATE_PSC_OBJECTS, L"MQ_ERROR_CANNOT_CREATE_PSC_OBJECTS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS, L"MQ_ERROR_CANNOT_DELETE_PSC_OBJECTS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_LOAD_MQAD, L"MQ_ERROR_CANNOT_LOAD_MQAD"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_SIGN_DATA_EX, L"MQ_ERROR_CANNOT_SIGN_DATA_EX"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_IMPERSONATE_CLIENT, L"MQ_ERROR_CANNOT_IMPERSONATE_CLIENT"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_OPEN_CERT_STORE, L"MQ_ERROR_CANNOT_OPEN_CERT_STORE"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR, L"MQ_ERROR_CANNOT_SET_CRYPTO_SEC_DESCR"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS, L"MQ_ERROR_CANNOT_UPDATE_PSC_OBJECTS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CANT_RESOLVE_SITES, L"MQ_ERROR_CANT_RESOLVE_SITES"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CERTIFICATE_NOT_PROVIDED, L"MQ_ERROR_CERTIFICATE_NOT_PROVIDED"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION, L"MQ_ERROR_COMPUTER_DOES_NOT_SUPPORT_ENCRYPTION"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CORRUPTED_INTERNAL_CERTIFICATE, L"MQ_ERROR_CORRUPTED_INTERNAL_CERTIFICATE"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE, L"MQ_ERROR_CORRUPTED_PERSONAL_CERT_STORE"));
	Descriptions.insert(HResult_Pair(MQ_CORRUPTED_QUEUE_WAS_DELETED, L"MQ_CORRUPTED_QUEUE_WAS_DELETED"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_CORRUPTED_SECURITY_DATA, L"MQ_ERROR_CORRUPTED_SECURITY_DATA"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO, L"MQ_ERROR_COULD_NOT_GET_ACCOUNT_INFO"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_COULD_NOT_GET_USER_SID, L"MQ_ERROR_COULD_NOT_GET_USER_SID"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DELETE_CN_IN_USE, L"MQ_ERROR_DELETE_CN_IN_USE"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW, L"MQ_ERROR_DEPEND_WKS_LICENSE_OVERFLOW"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DS_BIND_ROOT_FOREST, L"MQ_ERROR_DS_BIND_ROOT_FOREST"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DS_ERROR , L"MQ_ERROR_DS_ERROR"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DS_IS_FULL, L"MQ_ERROR_DS_IS_FULL"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DS_LOCAL_USER, L"MQ_ERROR_DS_LOCAL_USER"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_DTC_CONNECT, L"MQ_ERROR_DTC_CONNECT"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX , L"MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL, L"MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_GC_NEEDED, L"MQ_ERROR_GC_NEEDED"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_CONTEXT, L"MQ_ERROR_ILLEGAL_CONTEXT"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_CURSOR_ACTION, L"MQ_ERROR_ILLEGAL_CURSOR_ACTION"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_ENTERPRISE_OPERATION, L"MQ_ERROR_ILLEGAL_ENTERPRISE_OPERATION"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_FORMATNAME, L"MQ_ERROR_ILLEGAL_FORMATNAME"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_MQCOLUMNS, L"MQ_ERROR_ILLEGAL_MQCOLUMNS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_MQPRIVATEPROPS, L"MQ_ERROR_ILLEGAL_MQPRIVATEPROPS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_MQQMPROPS, L"MQ_ERROR_ILLEGAL_MQQMPROPS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_MQQUEUEPROPS, L"MQ_ERROR_ILLEGAL_MQQUEUEPROPS"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_OPERATION, L"MQ_ERROR_ILLEGAL_OPERATION"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_PROPERTY_SIZE, L"MQ_ERROR_ILLEGAL_PROPERTY_SIZE"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_PROPERTY_VALUE, L"MQ_ERROR_ILLEGAL_PROPERTY_VALUE"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_PROPERTY_VT, L"MQ_ERROR_ILLEGAL_PROPERTY_VT"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_PROPID, L"MQ_ERROR_ILLEGAL_PROPID"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_QUEUE_PATHNAME, L"MQ_ERROR_ILLEGAL_QUEUE_PATHNAME"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_RELATION, L"MQ_ERROR_ILLEGAL_RELATION"));
	Descriptions.insert(HResult_Pair(MQ_ERROR_ILLEGAL_RESTRICTION_PROPID, L"MQ_ERROR_ILLEGAL_RESTRICTION_PROPID"));
	//TODO: To complete list - http://msdn.microsoft.com/en-us/library/windows/desktop/ms700106(v=vs.85).aspx
	//TODO: I could've had a string list but for the sake of simplicity I've used a hash_map

	if (FAILED(hr) && Descriptions.find(hr) != Descriptions.end()) // Let's just return if HRESULT failed
		retval =  Descriptions.at(hr);

	return retval;
}


/******************* Exports *********************************/
MSMQBRIDGE_API HRESULT CreateQueue(const wchar_t* queuePath, const wchar_t* queueLabel, int& queueId, int open) {
	auto newQueue = CMSMQBridge(true);
	auto retval = newQueue.CreateQueue(std::wstring(queuePath), std::wstring(queueLabel), false, queueId, open);
	return (retval.hr);
}


MSMQBRIDGE_API HRESULT DeleteQueueById(const int queueId) {
	auto newQueue = CMSMQBridge(true);
	auto retval = newQueue.DeleteQueue(queueId);
	return (retval.hr);
}


MSMQBRIDGE_API HRESULT DeleteQueueByPath(const wchar_t* queuePath) {
	auto newQueue = CMSMQBridge(true);
	auto retval = newQueue.DeleteQueue(std::wstring(queuePath));
	return (retval.hr);
}

