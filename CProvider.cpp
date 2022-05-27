//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
// CProvider implements ICredentialProvider, which is the main
// interface that logonUI uses to decide which tiles to display.
// In this sample, we will display one tile that uses each of the nine
// available UI controls.

#include <initguid.h>
#include "CProvider.h"
#include "CCredential.h"

CProvider::CProvider():
    referenceCount(1),
    credProviderUserArray(nullptr),
	credentialProviderEvents(nullptr)	
{
}

CProvider::~CProvider()
{
	releaseEnumeratedCredentialPairs();

    if (credProviderUserArray != nullptr)
    {
        credProviderUserArray->Release();
        credProviderUserArray = nullptr;
    }
}

void CProvider::deleteFieldList(FieldList *fieldList)
{
	if (fieldList->size() == 0)
		return;

	for (auto iter: *fieldList)
	{
		delete iter;
	}
	fieldList->clear();
}

CredentialPair * CProvider::createCredentialPair()
{
	// 1. CCredential * 생성
	CCredential *credential = createCredential();
	if (credential == nullptr)
	{
		return nullptr;
	}

	// 2. FieldList * 생성
	FieldList *fieldList = createFieldList();
	if (fieldList == nullptr)
	{
		credential->Release();
		credential = nullptr;
		return nullptr;
	}

	// 3. CredentialPair 생성
	CredentialPair* credentialPair = new CredentialPair(credential, fieldList);
	if (credentialPair == nullptr)
	{
		credential->Release();
		credential = nullptr;
		deleteFieldList(fieldList);
		return nullptr;
	}

	return credentialPair;
}

void CProvider::deleteCredentialPair(CredentialPair * credentialPair)
{
	// 1. CCredential * 삭제
	if (credentialPair->first != nullptr)
	{
		credentialPair->first->Release();
		credentialPair->first = nullptr;
	}

	// 2. FieldList * 삭제
	if (credentialPair->second->size() != 0)
	{
		deleteFieldList(credentialPair->second);
	}
	credentialPair->second->clear();

	// 3. CredentialPair 삭제
	delete credentialPair;

	return;
}

// SetUsageScenario is the provider's cue that it's going to be asked for tiles
// in a subsequent call.
HRESULT CProvider::SetUsageScenario( CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD /*dwFlags*/)
{
    HRESULT hr;

    // Decide which scenarios to support here. Returning E_NOTIMPL simply tells the caller
    // that we're not designed for that scenario.
    switch (cpus)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:
	case CPUS_CHANGE_PASSWORD:
        // The reason why we need flagRecreateCredentials is because ICredentialProviderSetUserArray::SetUserArray() is called after ICredentialProvider::SetUsageScenario(),
        // while we need the ICredentialProviderUserArray during enumeration in ICredentialProvider::GetCredentialCount()
        this->cpUsageScenario = cpus;
		this->flagRecreateCredentials = true;
        hr = S_OK;
        break;

    case CPUS_CREDUI:
        hr = E_NOTIMPL;
        break;

	case CPUS_PLAP:
		break;

    default:
        hr = E_INVALIDARG;
        break;
    }

    return hr;
}

// SetSerialization takes the kind of buffer that you would normally return to LogonUI for
// an authentication attempt.  It's the opposite of ICredentialProviderCredential::GetSerialization.
// GetSerialization is implement by a credential and serializes that credential.  Instead,
// SetSerialization takes the serialization and uses it to create a tile.
//
// SetSerialization is called for two main scenarios.  The first scenario is in the credui case
// where it is prepopulating a tile with credentials that the user chose to store in the OS.
// The second situation is in a remote logon case where the remote client may wish to
// prepopulate a tile with a username, or in some cases, completely populate the tile and
// use it to logon without showing any UI.
//
// If you wish to see an example of SetSerialization, please see either the SampleCredentialProvider
// sample or the SampleCredUICredentialProvider sample.  [The logonUI team says, "The original sample that
// this was built on top of didn't have SetSerialization.  And when we decided SetSerialization was
// important enough to have in the sample, it ended up being a non-trivial amount of work to integrate
// it into the main sample.  We felt it was more important to get these samples out to you quickly than to
// hold them in order to do the work to integrate the SetSerialization changes from SampleCredentialProvider
// into this sample.
HRESULT CProvider::SetSerialization( _In_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION const * /*pcpcs*/)
{
    return E_NOTIMPL;
}

// Called by LogonUI to give you a callback.  Providers often use the callback if they
// some event would cause them to need to change the set of tiles that they enumerated.
HRESULT CProvider::Advise( _In_ ICredentialProviderEvents * pcpe, _In_ UINT_PTR upAdviseContext)
{
	if (this->credentialProviderEvents != NULL)
	{
		this->credentialProviderEvents->Release();
	}
	this->credentialProviderEvents = pcpe;
	this->upAdviseContext = upAdviseContext;
	this->credentialProviderEvents->AddRef();

	return S_OK;
}

// Called by LogonUI when the ICredentialProviderEvents callback is no longer valid.
HRESULT CProvider::UnAdvise()
{
	if (this->credentialProviderEvents != NULL)
	{
		this->credentialProviderEvents->Release();
		this->credentialProviderEvents = NULL;
	}

	return S_OK;
}

// Called by LogonUI to determine the number of fields in your tiles.  This
// does mean that all your tiles must have the same number of fields.
// This number must include both visible and invisible fields. If you want a tile
// to have different fields from the other tiles you enumerate for a given usage
// scenario you must include them all in this count and then hide/show them as desired
// using the field descriptors.
HRESULT CProvider::GetFieldDescriptorCount( _Out_ DWORD *pdwCount)
{
	// FieldList에 속한 Field의 개수를 전달하는 것이 목적이므로, credentialPairList의 index를 0으로 고정해도 무방하다.
	FieldList* fieldList = this->credentialPairList[0]->second;
    *pdwCount = static_cast<DWORD>(fieldList->size());
    return S_OK;
}

// Gets the field descriptor for a particular field.
HRESULT CProvider::GetFieldDescriptorAt( DWORD dwIndex, _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd)
{
    HRESULT hr = E_INVALIDARG;
    *ppcpfd = nullptr;

	// FieldList에 속한 Field들의 guidFieldType를 전달하는 것이 목적이므로, credentialPairList의 index를 0으로 고정해도 무방하다.
	FieldList* fieldList = this->credentialPairList[0]->second;

    if ((dwIndex < fieldList->size()) && ppcpfd)
    {
		DWORD cbStruct = sizeof(**ppcpfd);
		CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR *pcpfd = (CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR*)CoTaskMemAlloc(cbStruct);

		if (pcpfd == nullptr)
			return E_OUTOFMEMORY;

		pcpfd->dwFieldID = fieldList->at(dwIndex)->getFieldId();
		pcpfd->cpft = fieldList->at(dwIndex)->getFieldType();

		if (fieldList->at(dwIndex)->getLabel().size() > 0)
		{
			hr = SHStrDupW(fieldList->at(dwIndex)->getLabel().c_str(), &pcpfd->pszLabel);
			if (FAILED(hr))
			{
				CoTaskMemFree(pcpfd);
				return hr;
			}
		}			
		else
		{
			pcpfd->pszLabel = nullptr;
		}			
		pcpfd->guidFieldType = fieldList->at(dwIndex)->getGuiFieldType();

		*ppcpfd = pcpfd;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

// Sets pdwCount to the number of tiles that we wish to show at this time.
// Sets pdwDefault to the index of the tile which should be used as the default.
// The default tile is the tile which will be shown in the zoomed view by default. If
// more than one provider specifies a default the last used cred prov gets to pick
// the default. If *pbAutoLogonWithDefault is TRUE, LogonUI will immediately call
// GetSerialization on the credential you've specified as the default and will submit
// that credential for authentication without showing any further UI.
HRESULT CProvider::GetCredentialCount( _Out_ DWORD *pdwCount, _Out_ DWORD *pdwDefault, _Out_ BOOL *pbAutoLogonWithDefault)
{
    if (flagRecreateCredentials)
    {
        flagRecreateCredentials = false;
        releaseEnumeratedCredentialPairs();
        createEnumeratedCredentialPairs();
    }

	*pdwCount = static_cast<DWORD>(this->credentialPairList.size());
	*pdwDefault = this->getDefaultCredential();
	*pbAutoLogonWithDefault = FALSE;

	return S_OK;
}

// Returns the credential at the index specified by dwIndex. This function is called by logonUI to enumerate
// the tiles.
HRESULT CProvider::GetCredentialAt( DWORD dwIndex, _Outptr_result_nullonfailure_ ICredentialProviderCredential **ppcpc)
{
    HRESULT hr = E_INVALIDARG;
    *ppcpc = nullptr;

	if ((dwIndex < this->credentialPairList.size()) && ppcpc)
		hr = this->credentialPairList[dwIndex]->first->QueryInterface(IID_PPV_ARGS(ppcpc));

	return hr;
}

// This function will be called by LogonUI after SetUsageScenario succeeds.
// Sets the User Array with the list of users to be enumerated on the logon screen.
HRESULT CProvider::SetUserArray(_In_ ICredentialProviderUserArray *users)
{
    if (credProviderUserArray)
    {
        credProviderUserArray->Release();
    }
    credProviderUserArray = users;
    credProviderUserArray->AddRef();

    return S_OK;
}

void CProvider::createEnumeratedCredentialPairs()
{
    switch (cpUsageScenario)
    {
    case CPUS_LOGON:
    case CPUS_UNLOCK_WORKSTATION:
	case CPUS_CHANGE_PASSWORD:
        {
            enumerateCredentialPairs();
            break;
        }
    default:
        break;
    }
}

void CProvider::releaseEnumeratedCredentialPairs()
{
	if (credentialPairList.size() == 0)
		return;

	for (int i = 0; i < this->credentialPairList.size(); i++)
		deleteCredentialPair(credentialPairList[i]);

	this->credentialPairList.clear();
}

HRESULT CProvider::enumerateCredentialPairs()
{
	HRESULT hr = E_UNEXPECTED;
	if (credProviderUserArray == nullptr)
		return hr;

	DWORD dwUserCount;
	credProviderUserArray->GetCount(&dwUserCount);
	
	if (dwUserCount > 0) // 일반 사용자의 수에 맞춰 Credential 생성
	{
		for (unsigned int i = 0; i < dwUserCount; i++)
		{
			ICredentialProviderUser *pCredUser;
			hr = credProviderUserArray->GetAt(i, &pCredUser);
			if (FAILED(hr))
				continue;

			hr = enumerateOnePair(pCredUser);
			pCredUser->Release();
		}
	}
	if ((dwUserCount == 0) || (IsOS(OS_DOMAINMEMBER) == 1)) // 기타 사용자를 위한 여분의 Credential 생성
		hr = enumerateOnePair(nullptr);
	
	return hr;
}

HRESULT CProvider::enumerateOnePair(ICredentialProviderUser *pCredUser)
{
	HRESULT hr = E_UNEXPECTED;

	// 1. typedef std::pair<CCredential *, FieldList *> CredentialPair 생성 후 밀어넣기
	CredentialPair* credentialPair = CProvider::createCredentialPair();
	if (credentialPair == nullptr)
		return E_POINTER;

	// 2. initialize
	hr = credentialPair->first->initialize(cpUsageScenario, *credentialPair->second, pCredUser);
	if (FAILED(hr))
		deleteCredentialPair(credentialPair);
	else
		this->credentialPairList.push_back(credentialPair);

	return hr;
}


