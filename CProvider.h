//
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "CCredential.h"
#include "..\IsignResourceLoader\IsignResourceLoader.h"
#include "Field.h"

#include <initguid.h>
#include <Objbase.h>
#include <Psapi.h>
#include <LM.h>

#ifndef ISIGNCP_API
#ifdef ISIGNCP_EXPORTS
#define ISIGNCP_API __declspec(dllexport)
#else
#define ISIGNCP_API __declspec(dllimport)
#endif
#endif

typedef std::vector<EeWin::CredentialProvider::Field *> FieldList;
typedef std::pair<CCredential *, FieldList *> CredentialPair;

class ISIGNCP_API CProvider :	public ICredentialProvider,
					public ICredentialProviderSetUserArray
{
public:
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
        static const QITAB qit[] =
        {
            QITABENT(CProvider, ICredentialProvider), // IID_ICredentialProvider
            QITABENT(CProvider, ICredentialProviderSetUserArray), // IID_ICredentialProviderSetUserArray
            {0},
#pragma warning( push )
#pragma warning( disable: 4838 )
        };
#pragma warning( pop )
        return QISearch(this, qit, riid, ppv);
    }

	// Provider 인터페이스
    IFACEMETHODIMP SetUsageScenario(CREDENTIAL_PROVIDER_USAGE_SCENARIO cpus, DWORD dwFlags);
    IFACEMETHODIMP SetSerialization(_In_ CREDENTIAL_PROVIDER_CREDENTIAL_SERIALIZATION const *pcpcs);
    IFACEMETHODIMP Advise(_In_ ICredentialProviderEvents *pcpe, _In_ UINT_PTR upAdviseContext);
    IFACEMETHODIMP UnAdvise();
    IFACEMETHODIMP GetFieldDescriptorCount(_Out_ DWORD *pdwCount);
    IFACEMETHODIMP GetFieldDescriptorAt(DWORD dwIndex,  _Outptr_result_nullonfailure_ CREDENTIAL_PROVIDER_FIELD_DESCRIPTOR **ppcpfd);
    IFACEMETHODIMP GetCredentialCount(_Out_ DWORD *pdwCount, _Out_ DWORD *pdwDefault, _Out_ BOOL *pbAutoLogonWithDefault);
    IFACEMETHODIMP GetCredentialAt(DWORD dwIndex, _Outptr_result_nullonfailure_ ICredentialProviderCredential **ppcpc);
    IFACEMETHODIMP SetUserArray(_In_ ICredentialProviderUserArray *users);

protected:
    CProvider();
    virtual ~CProvider();

	// 각 인증 방식에 해당하는 Credential의 인스턴스를 생성하는 메서드
	virtual CCredential * createCredential() = 0;
	virtual DWORD getDefaultCredential() { return CREDENTIAL_PROVIDER_NO_DEFAULT; };

	// 각 인증 방식의 세부 Field List를 생성/삭제하는 메서드
	virtual FieldList * createFieldList() = 0;
	void deleteFieldList(FieldList *fieldList);

	// Credential과 FieldList를 묶어 한꺼번에 생성/삭제하는 메서드 
	// typedef std::pair<CCredential *, FieldList *> CredentialPair 이다.
	CredentialPair * createCredentialPair();
	void deleteCredentialPair(CredentialPair * credentialPair);

	ICredentialProviderUserArray *credProviderUserArray;


private:
	void releaseEnumeratedCredentialPairs();
	void createEnumeratedCredentialPairs();
	HRESULT enumerateCredentialPairs();
	HRESULT enumerateOnePair(ICredentialProviderUser *pCredUser);

	vector<CredentialPair *> credentialPairList;
    long referenceCount;            // Used for reference counting.	
    bool flagRecreateCredentials;
    CREDENTIAL_PROVIDER_USAGE_SCENARIO cpUsageScenario;    
	ICredentialProviderEvents *credentialProviderEvents;                    // Used to tell our owner to re-enumerate credentials.
	UINT_PTR upAdviseContext;
	
};
