// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the MSMQBRIDGE_EXPORTS
// symbol defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// MSMQBRIDGE_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef MSMQBRIDGE_EXPORTS
#define MSMQBRIDGE_API __declspec(dllexport)
#else
#define MSMQBRIDGE_API __declspec(dllimport)
#endif


typedef struct {
	int QueueId;
	std::wstring Label;
	bool IsTransactional;
	std::wstring Queuepath;
	std::wstring FormatName;
	QUEUEHANDLE queueHandle;
	MQQUEUEPROPS queueProperties;
	HRESULT QueueStatus[MAX_QUEUE_PROPERTIES];
	QUEUEPROPID QueuePropId[MAX_QUEUE_PROPERTIES];
	MQPROPVARIANT QueuePropVar[MAX_QUEUE_PROPERTIES];

	bool IsOpen() {
		return (queueHandle != NULL);
	}

} QueueInformation;

typedef struct {
	HRESULT hr;
	bool isSuccess;
	std::wstring errMessage;
} QueueOpResult;

typedef enum {
	ById,
	ByPath
} SearchType;


// This class is exported from the MSMQBridge.dll
class MSMQBRIDGE_API CMSMQBridge {
private:
	bool verbosityEnabled;
	void LogException(std::wstring exception);
	std::wstring GetHResultDescription(const HRESULT& hr);
	void CreateFailedQueueOpResult(QueueOpResult& valueToBeReturned, const HRESULT& hr);
	QueueOpResult PrepareMessage(std::wstring label, std::wstring messageBody, MQMSGPROPS& message);
	bool Item_Exist(bool checkBoth, QueueInformation& item, std::wstring expr1, std::wstring expr2);

public:
	CMSMQBridge();
	~CMSMQBridge();
	CMSMQBridge(bool isFullyVerbose);
	QueueOpResult DeleteQueue(int nQueueId);
	QueueOpResult DeleteQueue(std::wstring szQueuePath);
	QueueOpResult SendMessageToQueue(std::wstring pathOrId, std::wstring label, std::wstring message);
	QueueOpResult CreateQueue(std::wstring szQueuePath, std::wstring szQueueLabel, bool isTransactional, int& nQueueId, bool open);

	static std::hash_map<int, QueueInformation> Queues;

};


extern "C"
{
	MSMQBRIDGE_API HRESULT DeleteQueueById(const int queueId);
	MSMQBRIDGE_API HRESULT DeleteQueueByPath(const wchar_t* queuePath);
	MSMQBRIDGE_API HRESULT CreateQueue(const wchar_t* queuePath, const wchar_t* queueLabel, int& queueId, int open);
	MSMQBRIDGE_API HRESULT SendMessageToQueue(const wchar_t* pathOrId, const wchar_t* label, const wchar_t* message);
}