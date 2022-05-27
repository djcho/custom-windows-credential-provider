//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
//

#include "CCredential.h"
#include <propkey.h>
#include <lm.h>
#include "resource.h"
#include "../common/StringUtil.h"
#include <string>

CCredential::CCredential() :
	referenceCount(1),
	credProvCredentialEvents(nullptr),
	isOtherUser(false),
	isDomainJoined(false),
	cpUsageScenario(CPUS_INVALID),
	fieldController(nullptr)
{
}

CCredential::~CCredential()
{
}

// Initializes one credential with the field information passed in.
// Set the value of the TFI_STATUS_TEXT field to pwzUsername.
HRESULT CCredential::initialize(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus,
	_In_ vector<EeWin::CredentialProvider::Field *> fieldList,
	_In_ ICredentialProviderUser *pcpUser)
{
	this->cpUsageScenario = cpus;

	// password change 시나리오일 경우 password change 시나리오를 지원하는 cp인지 체크한다.
	if (this->cpUsageScenario == CPUS_CHANGE_PASSWORD)
	{
		if (this->isCpusChangePasswordSupport() == false)
			return S_FALSE;
	}

	if (pcpUser != nullptr)
	{
		WCHAR *userName = nullptr;
		WCHAR *qualifiedUserName = nullptr;
		WCHAR *userSid = nullptr;
		WCHAR *logonStatus = nullptr;

		this->isOtherUser = false;
		pcpUser->GetStringValue(PKEY_Identity_UserName, &userName);
		if (userName)
		{
			this->userName = userName;
			CoTaskMemFree(userName);
		}

		pcpUser->GetStringValue(PKEY_Identity_QualifiedUserName, &qualifiedUserName);
		if (qualifiedUserName != nullptr)
		{			
			this->qualifiedUserName = qualifiedUserName;
			CoTaskMemFree(qualifiedUserName);
		}

		pcpUser->GetSid(&userSid);
		if (userSid)
		{
			this->userSid = userSid;
			CoTaskMemFree(userSid);
		}

		pcpUser->GetStringValue(PKEY_Identity_LogonStatusString, &logonStatus);
		if (logonStatus)
		{
			this->logonStatus = logonStatus;
			CoTaskMemFree(logonStatus);
		}
	}
	else
	{
		this->isOtherUser = true;
	}

	if (IsOS(OS_DOMAINMEMBER))
	{
		this->isDomainJoined = true;

		PWSTR tmpDomainPtr = nullptr;
		NETSETUP_JOIN_STATUS DomainStatus = NetSetupUnknownStatus;
		HRESULT hr = NetGetJoinInformation(NULL, &tmpDomainPtr, &DomainStatus);
		if (FAILED(hr))
			return hr;

		this->domain = tmpDomainPtr;
		NetApiBufferFree(tmpDomainPtr);
	}
	else	//AD 아닌 경우에 domain을 가져오지 않으면 로그인이 불가능하다.
	{
		if (this->qualifiedUserName.empty())
		{
			PWSTR tmpDomainPtr = nullptr;
			NETSETUP_JOIN_STATUS DomainStatus = NetSetupUnknownStatus;
			HRESULT hr = NetGetJoinInformation(NULL, &tmpDomainPtr, &DomainStatus);

			this->domain = tmpDomainPtr;
			NetApiBufferFree(tmpDomainPtr);
		}
		else
		{
			PWSTR tempDomain = nullptr;
			PWSTR tempUserName = nullptr;
			HRESULT hr = SplitDomainAndUsername(this->qualifiedUserName.c_str(), &tempDomain, &tempUserName);
			if (FAILED(hr))
				return hr;

			this->domain = tempDomain;
			if (tempDomain)
				CoTaskMemFree(tempDomain);
			if (tempUserName)
				CoTaskMemFree(tempUserName);
		}
	}

	return S_OK;
}

BOOL CCredential::isLockedUser()
{
	WCHAR *logonText[] = { L"잠김", L"Locked",  L"로그인됨", L"Signed-in" };

	if (this->logonStatus.empty())
		return FALSE;

	for (int i = 0; i < _countof(logonText); i++)
	{
		if (this->logonStatus == logonText[i])
		{
			return TRUE;
		}
	}
	return FALSE;
}

// LogonUI calls this in order to give us a callback in case we need to notify it of anything.
HRESULT CCredential::Advise(_In_ ICredentialProviderCredentialEvents *pcpce)
{
	HRESULT hResult = S_OK;

	if (this->credProvCredentialEvents != nullptr)
	{
		this->credProvCredentialEvents->Release();
	}

	hResult = pcpce->QueryInterface(IID_PPV_ARGS(&this->credProvCredentialEvents));

	return hResult;
}

// LogonUI calls this to tell us to release the callback.
HRESULT CCredential::UnAdvise()
{
	if (this->credProvCredentialEvents)
	{
		this->credProvCredentialEvents->Release();
	}
	this->credProvCredentialEvents = nullptr;
	return S_OK;
}

// LogonUI calls this function when our tile is selected (zoomed)
// If you simply want fields to show/hide based on the selected state,
// there's no need to do anything here - you can set that up in the
// field definitions. But if you want to do something
// more complicated, like change the contents of a field when the tile is
// selected, you would do it here.
HRESULT CCredential::SetSelected(_Out_ BOOL *pbAutoLogon)
{
	*pbAutoLogon = FALSE;
	return S_OK;
}

// Similarly to SetSelected, LogonUI calls this when your tile was selected
// and now no longer is. The most common thing to do here (which we do below)
// is to clear out the password field.
HRESULT CCredential::SetDeselected()
{
	return S_OK;
}

// Get info for a particular field of a tile. Called by logonUI to get information
// to display the tile.
HRESULT CCredential::GetFieldState(DWORD dwFieldID,
	_Out_ CREDENTIAL_PROVIDER_FIELD_STATE *pcpfs,
	_Out_ CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE *pcpfis)
{
	HRESULT hr = S_OK;

	// Validate our parameters.
	if (dwFieldID < this->fieldController->getFieldCount())
	{
		*pcpfs = this->fieldController->getFieldState(dwFieldID);
		*pcpfis = this->fieldController->getFieldInteractiveState(dwFieldID);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

IFACEMETHODIMP CCredential::GetBitmapValue(DWORD dwFieldID, HBITMAP * phbmp)
{
	*phbmp = this->fieldController->getBitmap(dwFieldID);
	if (*phbmp)
		return S_OK;
	else
		return E_INVALIDARG;
}

// Sets ppwsz to the string value of the field at the index dwFieldID
HRESULT CCredential::GetStringValue(DWORD dwFieldID, _Outptr_result_nullonfailure_ PWSTR *ppwsz)
{
	HRESULT hr = S_OK;
	*ppwsz = nullptr;

	// Check to make sure dwFieldID is a legitimate index
	if (dwFieldID < this->fieldController->getFieldCount())
	{
		// Make a copy of the string and return that. The caller
		// is responsible for freeing it.
		hr = SHStrDupW(this->fieldController->getFieldString(dwFieldID).c_str(), ppwsz);
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return hr;
}


// Sets pdwAdjacentTo to the index of the field the submit button should be
// adjacent to. We recommend that the submit button is placed next to the last
// field which the user is required to enter information in. Optional fields
// should be below the submit button.
HRESULT CCredential::GetSubmitButtonValue(DWORD dwFieldID, _Out_ DWORD *pdwAdjacentTo)
{
	HRESULT hr = S_OK;
	*pdwAdjacentTo = this->fieldController->getSubmitButtonValue(dwFieldID);

	if (*pdwAdjacentTo == (DWORD)-1)
	{
		hr = E_INVALIDARG;
	}

	return hr;
}

// Sets the value of a field which can accept a string as a value.
// This is called on each keystroke when a user types into an edit field
HRESULT CCredential::SetStringValue(DWORD dwFieldID, _In_ PCWSTR pwz)
{
	return this->fieldController->setFieldString(dwFieldID, pwz);
}

// Returns whether a checkbox is checked or not as well as its label.
HRESULT CCredential::GetCheckboxValue(DWORD dwFieldID, _Out_ BOOL *pbChecked, _Outptr_result_nullonfailure_ PWSTR *ppwszLabel)
{
	return S_OK;
}

// Sets whether the specified checkbox is checked or not.
HRESULT CCredential::SetCheckboxValue(DWORD dwFieldID, BOOL bChecked)
{
	return S_OK;
}

// Returns the number of items to be included in the combobox (pcItems), as well as the
// currently selected item (pdwSelectedItem).
HRESULT CCredential::GetComboBoxValueCount(DWORD dwFieldID, _Out_ DWORD *pcItems, _Deref_out_range_(< , *pcItems) _Out_ DWORD *pdwSelectedItem)
{
	*pcItems = this->fieldController->getComboBoxValueCount(dwFieldID);
	*pdwSelectedItem = 0;
	return S_OK;
}

// Called iteratively to fill the combobox with the string (ppwszItem) at index dwItem.
HRESULT CCredential::GetComboBoxValueAt(DWORD dwFieldID, DWORD dwItem, _Outptr_result_nullonfailure_ PWSTR *ppwszItem)
{
	wstring comboBoxString = this->fieldController->getComboBoxValueAt(dwFieldID, dwItem);
	SHStrDupW(comboBoxString.c_str(), ppwszItem);
	return S_OK;
}

// Called when the user changes the selected item in the combobox.
HRESULT CCredential::SetComboBoxSelectedValue(DWORD dwFieldID, DWORD dwSelectedItem)
{
	this->fieldController->setComboBoxSelectedValue(dwFieldID, dwSelectedItem);
	return S_OK;
}

// Called when the user clicks a command link.
HRESULT CCredential::CommandLinkClicked(DWORD dwFieldID)
{
	return S_OK;
}

// Gets the SID of the user corresponding to the credential.
HRESULT CCredential::GetUserSid(_Outptr_result_nullonfailure_ PWSTR *ppszSid)
{
	*ppszSid = nullptr;
	HRESULT hr = E_UNEXPECTED;
	if (!this->userSid.empty())
	{
		hr = SHStrDupW(this->userSid.c_str(), ppszSid);
	}
	else
	{
		// for link to other user
		hr = S_FALSE;
	}

	return hr;
}

// GetFieldOptions to enable the password reveal button and touch keyboard auto-invoke in the password field.
HRESULT CCredential::GetFieldOptions(DWORD dwFieldID,
	_Out_ CREDENTIAL_PROVIDER_CREDENTIAL_FIELD_OPTIONS *pcpcfo)
{
	*pcpcfo = CPCFO_NONE;

	return S_OK;
}

HWND CCredential::getParentWnd()
{
	HWND parentHandle = nullptr;

	if (this->credProvCredentialEvents)
	{
		this->credProvCredentialEvents->OnCreatingWindow(&parentHandle);
	}	

	return parentHandle;
}

HRESULT CCredential::Disconnect()
{
	return S_OK;
}
