#include "FieldController.h"

namespace EeWin
{
namespace CredentialProvider
{

FieldController::FieldController(ICredentialProviderCredential * credential, ICredentialProviderCredentialEvents2 ** credentialEvent)
	:credential(credential), credentialEvent(credentialEvent)
{
}

FieldController::~FieldController()
{
	
}

void FieldController::addField(Field *field)
{
	if (field->getFieldType() == CPFT_EDIT_TEXT ||
		field->getFieldType() == CPFT_PASSWORD_TEXT)
	{
		field->setString(L"");
	}
	else
		field->setString(field->getLabel());

	this->fields.push_back(field);
}

DWORD FieldController::getFieldCount()
{
	return static_cast<DWORD>(this->fields.size());
}

CREDENTIAL_PROVIDER_FIELD_STATE FieldController::getFieldState(const DWORD fieldId)
{
	return this->fields.at(fieldId)->getFieldState();
}

CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE FieldController::getFieldInteractiveState(const DWORD fieldId)
{
	return this->fields.at(fieldId)->getFieldInteractiveState();
}


void FieldController::setFieldInteractiveState(const DWORD fieldId, CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE fieldInteractiveState)
{
	this->fields.at(fieldId)->setFieldInteractiveState(fieldInteractiveState);
}

DWORD FieldController::getSubmitButtonValue(const DWORD fieldId)
{
	if (this->fields.at(fieldId)->getFieldType() == CPFT_SUBMIT_BUTTON)
	{
		return dynamic_cast<EeWin::CredentialProvider::SubmitButtonField *>(this->fields.at(fieldId))->getValue();
	}
	else
	{
		return (DWORD)-1;
	}
}

HRESULT FieldController::setSubmitButtonValue(const DWORD adjacentTo)
{
	for (auto iter : this->fields)
	{
		if (iter->getFieldType() == CPFT_SUBMIT_BUTTON)
		{
			dynamic_cast<EeWin::CredentialProvider::SubmitButtonField *>(iter)->setValue(adjacentTo);
			if (*this->credentialEvent)
			{
				return (*this->credentialEvent)->SetFieldSubmitButton(this->credential, iter->getFieldId(), adjacentTo);
			}
			return S_OK;
		}
	}

	return E_FAIL;
}

HRESULT FieldController::beginFieldUpdates()
{
	if (*this->credentialEvent)
		return (*this->credentialEvent)->BeginFieldUpdates();
	else
		return E_FAIL;
}

HRESULT FieldController::endFieldUpdates()
{
	if (*this->credentialEvent)
		return (*this->credentialEvent)->EndFieldUpdates();
	else
		return E_FAIL;
}

HRESULT FieldController::focusField(const DWORD fieldId)
{
	if (fieldId >= this->fields.size())
		return E_INVALIDARG;

	if (this->fields[fieldId]->getFieldInteractiveState() == CPFIS_DISABLED ||
		this->fields[fieldId]->getFieldInteractiveState() == CPFIS_FOCUSED)
		return E_INVALIDARG;

	for (const auto &iter : this->fields)
	{
		if (iter->getFieldInteractiveState() == CPFIS_FOCUSED)
		{
			iter->setFieldInteractiveState(CPFIS_NONE);
			if (*this->credentialEvent)
				(*this->credentialEvent)->SetFieldInteractiveState(this->credential, iter->getFieldId(), CPFIS_NONE);
		}
	}

	this->fields[fieldId]->setFieldInteractiveState(CPFIS_FOCUSED);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->SetFieldInteractiveState(this->credential, fieldId, CPFIS_FOCUSED);
	}
	return S_OK;
}

HRESULT FieldController::enableField(const DWORD fieldId, const bool enabled)
{
	if (fieldId >= this->fields.size())
		return E_INVALIDARG;

	CREDENTIAL_PROVIDER_FIELD_INTERACTIVE_STATE cpfis;
	if (enabled)
		cpfis = CPFIS_NONE;
	else
		cpfis = CPFIS_DISABLED;

	this->fields.at(fieldId)->setFieldInteractiveState(cpfis);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->SetFieldInteractiveState(this->credential, fieldId, cpfis);
	}

	return S_OK;
}

HRESULT FieldController::readOnlyField(const DWORD fieldId, const bool readOnly)
{
	// readonly 속성은 deprecated 되었다.
	return enableField(fieldId, !readOnly);
}

HRESULT FieldController::showField(const DWORD fieldId, const bool show)
{
	if (fieldId >= this->fields.size())
		return E_INVALIDARG;

	CREDENTIAL_PROVIDER_FIELD_STATE cpfs;

	if (show)
		cpfs = CPFS_DISPLAY_IN_SELECTED_TILE;
	else
		cpfs = CPFS_HIDDEN;

	// CPFS_DISPLAY_IN_DESELECTED_TILE, CPFS_DISPLAY_IN_BOTH 는 일단 고려하지 않는다.

	this->fields.at(fieldId)->setFieldState(cpfs);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->SetFieldState(this->credential, fieldId, cpfs);
	}
	return S_OK;
}

HRESULT FieldController::setFieldString(const DWORD fieldId, const std::wstring & fieldString)
{
	if (fieldId >= this->fields.size())
		return E_INVALIDARG;

	this->fields.at(fieldId)->setString(fieldString);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->SetFieldString(this->credential, fieldId, fieldString.c_str());
	}
	return S_OK;
}

const std::wstring FieldController::getFieldString(const DWORD fieldId)
{
	if (fieldId >= this->fields.size())
		return L"";

	return this->fields.at(fieldId)->getString();
}

HBITMAP FieldController::getBitmap(const DWORD fieldId)
{
	if (fieldId >= this->fields.size())
		return nullptr;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_TILE_IMAGE)
		return nullptr;

	return dynamic_cast<EeWin::CredentialProvider::TileImageField *>(this->fields.at(fieldId))->getBitmap();
}

DWORD FieldController::getComboBoxValueCount(const DWORD fieldId)
{
	if (fieldId >= this->fields.size())
		return 0;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_COMBOBOX)
		return 0;

	return dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->getItemCount();
}

const std::wstring FieldController::getComboBoxValueAt(const DWORD fieldId, const DWORD itemId)
{
	if (fieldId >= this->fields.size())
		return L"";

	if (this->fields.at(fieldId)->getFieldType() != CPFT_COMBOBOX)
		return L"";

	if(itemId >= dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->getItemCount())
		return L"";

	return dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->getItem(itemId);
}

void FieldController::setComboBoxSelectedValue(const DWORD fieldId, const DWORD itemId)
{
	if (fieldId >= this->fields.size())
		return;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_COMBOBOX)
		return;

	if (itemId >= dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->getItemCount())
		return;

	dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->setSelectedItemNumber(itemId);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->SetFieldComboBoxSelectedItem(this->credential, fieldId, itemId);
	}
}

void FieldController::deleteComboBoxItem(const DWORD fieldId, const DWORD itemId)
{
	if (fieldId >= this->fields.size())
		return;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_COMBOBOX)
		return;

	if (itemId >= dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->getItemCount())
		return;

	dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->deleteItem(itemId);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->DeleteFieldComboBoxItem(this->credential, fieldId, itemId);
	}
}

void FieldController::appendComboBoxItem(const DWORD fieldId, const std::wstring item)
{
	if (fieldId >= this->fields.size())
		return;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_COMBOBOX)
		return;

	dynamic_cast<EeWin::CredentialProvider::ComboBoxField *>(this->fields.at(fieldId))->appendItem(item);
	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->AppendFieldComboBoxItem(this->credential, fieldId, item.c_str());
	}
}

HRESULT FieldController::getCheckBoxValue(const DWORD fieldId, BOOL * checked)
{
	if (fieldId >= this->fields.size())
		return E_INVALIDARG;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_CHECKBOX)
		return E_INVALIDARG;

	*checked = dynamic_cast<EeWin::CredentialProvider::CheckBoxField *>(this->fields.at(fieldId))->getCheck();

	return S_OK;
}

void FieldController::setCheckBoxValue(const DWORD fieldId, BOOL checked, std::wstring lable)
{
	if (fieldId >= this->fields.size())
		return;

	if (this->fields.at(fieldId)->getFieldType() != CPFT_CHECKBOX)
		return;
	
	dynamic_cast<EeWin::CredentialProvider::CheckBoxField *>(this->fields.at(fieldId))->setCheck(checked);
	this->fields.at(fieldId)->setString(lable);

	if (*this->credentialEvent)
	{
		(*this->credentialEvent)->SetFieldCheckbox(this->credential, fieldId, checked, lable.c_str());
	}
}

}
}