#pragma once

#include "Field.h"

#ifdef ISIGNCP_EXPORTS
#define ISIGNCP_API __declspec(dllexport)
#else
#define ISIGNCP_API __declspec(dllimport)
#endif


namespace EeWin
{
namespace CredentialProvider
{

class ISIGNCP_API FieldController
{
public:
	FieldController(ICredentialProviderCredential *credential,
		ICredentialProviderCredentialEvents2** credentialEvent);

	~FieldController();

	void addField(Field *field);
	DWORD getFieldCount();
	CREDENTIAL_PROVIDER_FIELD_STATE getFieldState(const DWORD fieldId);
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE getFieldInteractiveState(const DWORD fieldId);
	void setFieldInteractiveState(const DWORD fieldId, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	DWORD getSubmitButtonValue(const DWORD fieldId);
	HRESULT setSubmitButtonValue(const DWORD adjacentTo);

	HRESULT beginFieldUpdates();
	HRESULT endFieldUpdates();
	HRESULT focusField(const DWORD fieldId);
	HRESULT enableField(const DWORD fieldId, const bool enabled);
	HRESULT readOnlyField(const DWORD fieldId, const bool readOnly);	// deprecated, this method works opposite to enableField.
	HRESULT showField(const DWORD fieldId, const bool show);
	HRESULT setFieldString(const DWORD fieldId, const std::wstring &fieldString);
	const std::wstring getFieldString(const DWORD fieldId);
	HBITMAP getBitmap(const DWORD fieldId);
	DWORD getComboBoxValueCount(const DWORD fieldId);
	const std::wstring getComboBoxValueAt(const DWORD fieldId, const DWORD itemId);
	void setComboBoxSelectedValue(const DWORD fieldId, const DWORD itemId);
	void deleteComboBoxItem(const DWORD fieldId, const DWORD itemId);
	void appendComboBoxItem(const DWORD fieldId, const std::wstring item);
	HRESULT getCheckBoxValue(const DWORD fieldId, BOOL *checked);
	void setCheckBoxValue(const DWORD fieldId, BOOL checked, std::wstring lable);

private:
	ICredentialProviderCredential *credential;
	ICredentialProviderCredentialEvents2** credentialEvent;
	std::vector<Field*> fields;
};

}
}