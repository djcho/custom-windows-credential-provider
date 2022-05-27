#ifndef _EE_WIN_ISIGN_CP_CREDENTIAL_H_
#define _EE_WIN_ISIGN_CP_CREDENTIAL_H_


#include "helpers.h"
#include "../IsignAccountRepo/AccountRepo.h"
#include "../IsignSystemInfo/SystemInfo.h"
#include "../IsignConfigHelper/IsignConfigHelper.h"
#include "../IsignDialog/IsignCpDlg.h"
#include "..\IsignResourceLoader\IsignResourceLoader.h"
#include "FieldController.h"
#include <thread>
#include <windows.h>
#include <strsafe.h>
#include <shlguid.h>

#ifndef ISIGNCP_API
#ifdef ISIGNCP_EXPORTS
#define ISIGNCP_API __declspec(dllexport)
#else
#define ISIGNCP_API __declspec(dllimport)
#endif
#endif

#define THIS_CREDENTIAL reinterpret_cast<ICredentialProviderCredential*>(this)
#define SAFE_CLOSE(handle) if(handle) CloseHandle(handle);

enum AUTHENTICATE_STATE
{
	AUTHENTICATE_STATE_DEFAULT,
	AUTHENTICATE_STATE_SUCCESS,
	AUTHENTICATE_STATE_FAIL,
	AUTHENTICATE_CHANGE_PW,
};

enum CONNECT_RESULT
{
	CONNECT_RESULT_SUCCESS = 0,
	CONNECT_RESULT_INPUT_EMPTY,
	CONNECT_RESULT_PW_CHANGE_SUCCESS,
	CONNECT_RESULT_PW_CHANGE_FAILED,
	CONNECT_RESULT_PW_FORCED_CHANGE_FAILED,
	CONNECT_RESULT_PW_CHANGE_SCENARIO,
	CONNECT_RESULT_GET_NAME_FAILED,
	CONNECT_RESULT_AUTHENTICATE_FAILED,
	CONNECT_RESULT_USER_CANCELED,
	CONNECT_RESULT_GET_BASE_INFO_FAILED,
};

class ISIGNCP_API CCredential : public ICredentialProviderCredential2, ICredentialProviderCredentialWithFieldOptions, IConnectableCredentialProviderCredential
{
public:
	CCredential();
	virtual ~CCredential();

	// IUnknown
	IFACEMETHODIMP_(ULONG) AddRef()
	{
		return ++referenceCount;
	}

	IFACEMETHODIMP_(ULONG) Release()
	{
		long cRef = --referenceCount;

		if (!cRef)
		{
			delete this;
		}
		return cRef;
	}

	IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void **ppv)
	{
#pragma warning( push )
#pragma warning( disable: 4838 )
		static const QITAB qit[] =
		{
			QITABENTMULTI(CCredential, ICredentialProviderCredential, ICredentialProviderCredential2), // IID_ICredentialProviderCredential
			QITABENT(CCredential, IConnectableCredentialProviderCredential), // IID_ICredentialProviderCredential
			QITABENT(CCredential, ICredentialProviderCredential2), // IID_ICredentialProviderCredential2
			QITABENT(CCredential, ICredentialProviderCredentialWithFieldOptions), //IID_ICredentialProviderCredentialWithFieldOptions
			{ 0 },
		};
#pragma warning( pop )
		return QISearch(this, qit, riid, ppv);
	}

	// ICredentialProviderCredential
	IFACEMETHODIMP Advise(_In_ ICredentialProviderCredentialEvents *pcpce);
	IFACEMETHODIMP UnAdvise();
	IFACEMETHODIMP SetSelected(_Out_ BOOL *pbAutoLogon);
	IFACEMETHODIMP SetDeselected();
	IFACEMETHODIMP GetFieldState(DWORD dwFieldID,
		_Out_ CREDENTIAL_PROVIDER_FIELD_STATE *pcpfs,
		_Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE *pcpfis);
	IFACEMETHODIMP GetBitmapValue(DWORD dwFieldID, HBITMAP * phbmp);
	IFACEMETHODIMP GetStringValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ PWSTR *ppwsz);
	IFACEMETHODIMP GetCheckboxValue(DWORD dwFieldID, _Out_ BOOL *pbChecked, _Outptr_result_nullonfailure_ PWSTR *ppwszLabel);
	IFACEMETHODIMP GetComboBoxValueCount(DWORD dwFieldID, _Out_ DWORD *pcItems, _Deref_out_range_(< , *pcItems) _Out_ DWORD *pdwSelectedItem);
	IFACEMETHODIMP GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, _Outptr_result_nullonfailure_ PWSTR *ppwszItem);
	IFACEMETHODIMP GetSubmitButtonValue(DWORD dwFieldID, _Out_ DWORD *pdwAdjacentTo);
	IFACEMETHODIMP SetStringValue(DWORD dwFieldID, _In_ PCWSTR pwz);
	IFACEMETHODIMP SetCheckboxValue(DWORD dwFieldID, BOOL bChecked);
	IFACEMETHODIMP SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem);
	IFACEMETHODIMP CommandLinkClicked(DWORD dwFieldID);

	// ICredentialProviderCredential2
	IFACEMETHODIMP GetUserSid(_Outptr_result_nullonfailure_ PWSTR *ppszSid);
	// ICredentialProviderCredentialWithFieldOptions
	IFACEMETHODIMP GetFieldOptions(DWORD dwFieldID,
		_Out_ CREDENTIAL_PROVIDER_CREDENTIAL_FIELD_OPTIONS *pcpcfo);
	// IConnectableCredentialProviderCredential
	IFACEMETHODIMP Disconnect();

	IFACEMETHODIMP GetSerialization(_Out_ CREDENTIAL_PROVIDER_GET_SERIALIZATION_RESPONSE *pcpgsr,
		_Out_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION *pcpcs,
		_Outptr_result_maybenull_ PWSTR *ppwszOptionalStatusText,
		_Out_ CREDENTIAL_PROVIDER_STATUS_ICON *pcpsiOptionalStatusIcon) = 0;
	IFACEMETHODIMP ReportResult(NTSTATUS ntsStatus,
		NTSTATUS ntsSubstatus,
		_Outptr_result_maybenull_ PWSTR *ppwszOptionalStatusText,
		_Out_ CREDENTIAL_PROVIDER_STATUS_ICON *pcpsiOptionalStatusIcon) = 0;
	IFACEMETHODIMP Connect(_In_ IQueryContinueWithStatus *pqcws) = 0;

	virtual HRESULT initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
		_In_ vector<EeWin::CredentialProvider::Field *> fieldList,
		_In_ ICredentialProviderUser *pcpUser);
	virtual bool isCpusChangePasswordSupport() = 0;	// CPUS_CHANGE_PASSWORD 시나리오를 지원하는 CP인지 여부.

protected:
	virtual GUID getClassID() = 0;
	virtual HINSTANCE getInstance() = 0;

	BOOL isLockedUser();
	HWND getParentWnd();

	EeWin::CredentialProvider::FieldController *fieldController;

	CREDENTIAL_PROVIDER_USAGE_SCENARIO cpUsageScenario; // The usage scenario for which we were enumerated.
	std::wstring domain;
	std::wstring userSid;
	std::wstring userName;
	std::wstring cachedCurrentPassword;
	std::wstring qualifiedUserName; // The user name that's used to pack the authentication buffer
	std::wstring logonStatus;
	bool isOtherUser;
	bool isDomainJoined;
	CONNECT_RESULT connectResult = CONNECT_RESULT_SUCCESS;
	ICredentialProviderCredentialEvents2* credProvCredentialEvents; // Used to update fields.

private:
	long referenceCount;
};


#endif // _EE_WIN_ISIGN_CP_CREDENTIAL_H_