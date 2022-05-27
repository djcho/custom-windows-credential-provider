#pragma once
#include <credentialprovider.h>
#include <ShlGuid.h>
#include <vector>
#include <string>

#ifdef ISIGNCP_EXPORTS
#define ISIGNCP_API __declspec(dllexport)
#else
#define ISIGNCP_API __declspec(dllimport)
#endif

#pragma warning(disable : 4251)

namespace EeWin
{
namespace CredentialProvider
{
class ISIGNCP_API Field
{
public:
	Field(DWORD fieldId,
		CREDENTIAL_PROVIDER_FIELD_TYPE fieldType,
		std::wstring label,
		CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState,
		GUID guidFieldType);
	virtual ~Field();

	DWORD getFieldId();
	CREDENTIAL_PROVIDER_FIELD_TYPE getFieldType();
	std::wstring getLabel();
	std::wstring getString();
	void setString(std::wstring string);
	CREDENTIAL_PROVIDER_FIELD_STATE getFieldState();
	void setFieldState(CREDENTIAL_PROVIDER_FIELD_STATE fieldState);
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE getFieldInteractiveState();
	void setFieldInteractiveState(CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	GUID getGuiFieldType();

protected:
	DWORD fieldId;
	CREDENTIAL_PROVIDER_FIELD_TYPE fieldType;
	std::wstring label;
	std::wstring string;
	CREDENTIAL_PROVIDER_FIELD_STATE fieldState;
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState;
	GUID guidFieldType;
};

class ISIGNCP_API LargeTextField : public Field
{
public:
	LargeTextField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	~LargeTextField();
};

class ISIGNCP_API SmallTextField : public Field
{
public:
	SmallTextField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	~SmallTextField();
};

class ISIGNCP_API CommandLinkField : public Field
{
public:
	CommandLinkField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	~CommandLinkField();	
};

class ISIGNCP_API EditTextField : public Field
{
public:
	EditTextField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	~EditTextField();
};

class ISIGNCP_API PasswordTextField : public Field
{
public:
	PasswordTextField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	~PasswordTextField();
};

class ISIGNCP_API TileImageField : public Field
{
public:
	TileImageField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState,
		HBITMAP bitmap);
	~TileImageField();
	HBITMAP getBitmap();
private:
	HBITMAP bitmap;
};

class ISIGNCP_API CheckBoxField : public Field
{
public:
	CheckBoxField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	~CheckBoxField();
	BOOL getCheck();
	void setCheck(BOOL check);
private:
	BOOL checked;
};

class ISIGNCP_API ComboBoxField : public Field
{
public:
	ComboBoxField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState);
	ComboBoxField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState,
		std::vector<std::wstring> items,
		DWORD selectedItemNumber);
	~ComboBoxField();
	DWORD getItemCount();
	DWORD getSelectedItemNumber();
	void setSelectedItemNumber(DWORD itemNumber);
	std::wstring getItem(DWORD itemNumber);
	void appendItem(std::wstring item);
	void deleteItem(DWORD itemNumber);
private:
	std::vector<std::wstring> items;
	DWORD selectedItemNumber;
};

class ISIGNCP_API SubmitButtonField : public Field
{
public:
	SubmitButtonField(DWORD fieldId,
		std::wstring label,
		const CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
		const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState,
		DWORD adjacentTo);
	~SubmitButtonField();
	DWORD getValue();
	void setValue(DWORD adjacentTo);
private:
	DWORD adjacentTo;
};

}
}
