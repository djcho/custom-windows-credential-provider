#include "Field.h"
#include <shlwapi.h>

namespace EeWin
{
namespace CredentialProvider
{


Field::Field(DWORD fieldId,
	CREDENTIAL_PROVIDER_FIELD_TYPE fieldType,
	std::wstring label,
	CREDENTIAL_PROVIDER_FIELD_STATE fieldState,
	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState,
	GUID guidFieldType)
	:fieldId(fieldId), fieldType(fieldType), label(label), fieldState(fieldState), fieldInteractiveState(fieldInteractiveState), guidFieldType(guidFieldType)
{
}

Field::~Field()
{
}

DWORD Field::getFieldId()
{
	return this->fieldId;
}

CREDENTIAL_PROVIDER_FIELD_TYPE Field::getFieldType()
{
	return this->fieldType;
}

std::wstring Field::getLabel()
{
	return this->label;
}

std::wstring Field::getString()
{
	return this->string;
}

void Field::setString(std::wstring string)
{
	this->string = string;
}

CREDENTIAL_PROVIDER_FIELD_STATE Field::getFieldState()
{
	return this->fieldState;
}

void Field::setFieldState(CREDENTIAL_PROVIDER_FIELD_STATE fieldState)
{
	this->fieldState = fieldState;
}

CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE Field::getFieldInteractiveState()
{
	return this->fieldInteractiveState;
}

void Field::setFieldInteractiveState(CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
{
	this->fieldInteractiveState = fieldInteractiveState;
}

GUID Field::getGuiFieldType()
{
	return this->guidFieldType;
}


LargeTextField::LargeTextField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	:Field(fieldId, CPFT_LARGE_TEXT, label, fieldState, fieldInteractiveState, { 0 })
{
}

LargeTextField::~LargeTextField()
{
}

SmallTextField::SmallTextField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	:Field(fieldId, CPFT_SMALL_TEXT, label, fieldState, fieldInteractiveState, CPFG_CREDENTIAL_PROVIDER_LABEL)
{
}

SmallTextField::~SmallTextField()
{
}

CommandLinkField::CommandLinkField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	:Field(fieldId, CPFT_COMMAND_LINK, label, fieldState, fieldInteractiveState, { 0 })	
{
}

CommandLinkField::~CommandLinkField()
{
}

EditTextField::EditTextField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	:Field(fieldId, CPFT_EDIT_TEXT, label, fieldState, fieldInteractiveState, { 0 })
{
}

EditTextField::~EditTextField()
{
}

PasswordTextField::PasswordTextField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	:Field(fieldId, CPFT_PASSWORD_TEXT, label, fieldState, fieldInteractiveState, { 0 })
{
}

PasswordTextField::~PasswordTextField()
{
}

TileImageField::TileImageField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState, HBITMAP bitmap)
	:Field(fieldId, CPFT_TILE_IMAGE, label, fieldState, fieldInteractiveState, CPFG_CREDENTIAL_PROVIDER_LOGO),
	bitmap(bitmap)
{
}

TileImageField::~TileImageField()
{
}

HBITMAP TileImageField::getBitmap()
{
	return this->bitmap;
}

CheckBoxField::CheckBoxField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	:Field(fieldId, CPFT_CHECKBOX, label, fieldState, fieldInteractiveState, { 0 })
{
}

CheckBoxField::~CheckBoxField()
{
}

BOOL CheckBoxField::getCheck()
{
	return checked;
}

void CheckBoxField::setCheck(BOOL check)
{
	checked = check;
}

ComboBoxField::ComboBoxField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState, std::vector<std::wstring> items, DWORD selectedItemNumber)
	:Field(fieldId, CPFT_COMBOBOX, label, fieldState, fieldInteractiveState, { 0 }),
	items(items), selectedItemNumber(selectedItemNumber)
{
}

ComboBoxField::ComboBoxField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
	: Field(fieldId, CPFT_COMBOBOX, label, fieldState, fieldInteractiveState, { 0 })
{
	this->items.push_back(label);
	this->selectedItemNumber = 0;
}

ComboBoxField::~ComboBoxField()
{
}

DWORD ComboBoxField::getItemCount()
{
	return static_cast<DWORD>(this->items.size());
}

DWORD ComboBoxField::getSelectedItemNumber()
{
	return this->selectedItemNumber;
}

void ComboBoxField::setSelectedItemNumber(DWORD itemNumber)
{
	if (itemNumber >= this->items.size())
		return;
	this->selectedItemNumber = itemNumber;
}

std::wstring ComboBoxField::getItem(DWORD itemNumber)
{
	if (itemNumber >= this->items.size())
		return std::wstring();		
	else
		return this->items.at(itemNumber);		
}

void ComboBoxField::appendItem(std::wstring item)
{
	this->items.push_back(item);
}

void ComboBoxField::deleteItem(DWORD itemNumber)
{
	if (itemNumber >= this->items.size())
		return;
	this->items.erase(this->items.begin() + itemNumber);
}

SubmitButtonField::SubmitButtonField(DWORD fieldId, std::wstring label, const CREDENTIAL_PROVIDER_FIELD_STATE fieldState, const CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState, DWORD adjacentTo)
	:Field(fieldId, CPFT_SUBMIT_BUTTON, label, fieldState, fieldInteractiveState, { 0 }),
	adjacentTo(adjacentTo)
{
}

SubmitButtonField::~SubmitButtonField()
{
}

DWORD SubmitButtonField::getValue()
{
	return this->adjacentTo;
}

void SubmitButtonField::setValue(DWORD adjacentTo)
{
	this->adjacentTo = adjacentTo;
}

}
}

