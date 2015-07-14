﻿#ifndef tf_advbuttonbase_H
#define tf_advbuttonbase_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/EditablePanel.h>
#include "tf_imagepanel.h"
#include "tf_controls.h"

using namespace vgui;

class CTFButtonBase;

enum MouseState
{
	MOUSE_DEFAULT,
	MOUSE_ENTERED,
	MOUSE_EXITED,
	MOUSE_PRESSED,
	MOUSE_RELEASED
};

#define DEFAULT_PATH		"../vgui/main_menu/"
#define DEFAULT_BG			"MainMenuAdvButtonDefault"
#define ARMED_BG			"MainMenuAdvButtonArmed"
#define DEPRESSED_BG		"MainMenuAdvButtonDepressed"
#define DEFAULT_BORDER		"AdvRoundedButtonDefault"
#define ARMED_BORDER		"AdvRoundedButtonArmed"
#define DEPRESSED_BORDER	"AdvRoundedButtonDepressed"
#define DEFAULT_TEXT		"AdvTextDefault"
#define ARMED_TEXT			"AdvTextArmed"
#define DEPRESSED_TEXT		"AdvTextDepressed"
#define DEFAULT_FONT		"MenuSmallFont"
#define EMPTY_STRING		""
#define GETSCHEME()			scheme()->GetIScheme(GetScheme())
#define pSelectedBG			(IsSelected() ? pArmedBG : pDefaultBG)

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFAdvButtonBase : public Button
{
	friend CTFButtonBase;
public:
	DECLARE_CLASS_SIMPLE(CTFAdvButtonBase, Button);

	CTFAdvButtonBase(vgui::Panel *parent, const char *panelName, const char *text);
	virtual ~CTFAdvButtonBase();
	virtual void Init();
	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void PerformLayout();

	virtual void SendAnimation(MouseState flag);
	virtual void SetDefaultAnimation();
	virtual const char* GetCommandString();
	virtual void SetAutoChange(bool bAutoChange) { m_bAutoChange = bAutoChange; };
	virtual bool IsAutoChange() { return m_bAutoChange; };
	virtual void SetBorderVisible(bool bVisible){ m_bBorderVisible = bVisible; };
	virtual void SetBGVisible(bool bVisible){ m_bBGVisible = bVisible; };
	virtual void SetDisabled(bool bDisabled){ m_bDisabled = bDisabled; };
	virtual bool IsDisabled() { return m_bDisabled; };
	virtual void SetFont(const char *sFont);
	virtual void SetBorder(const char *sBorder);

	virtual void OnThink();

protected:
	bool			m_bBGVisible;
	bool			m_bBorderVisible;
	bool			m_bDisabled;
	bool			m_bShowInt;
	char			pDefaultBG[64];
	char			pArmedBG[64];
	char			pDepressedBG[64];
	char			pDefaultBorder[64];
	char			pArmedBorder[64];
	char			pDepressedBorder[64];
	char			pDefaultText[64];
	char			pArmedText[64];
	char			pDepressedText[64];
	char			m_szCommand[64];
	char			m_szText[64];
	char			m_szFont[64];
	char			m_szTextAlignment[64];
	EditablePanel	*pBGBorder;
	virtual			vgui::Label::Alignment GetAlignment(char* m_szAlignment);
	bool			m_bAutoChange;
	//CTFButtonBase	*pButton;
};


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CTFButtonBase : public Button
{
public:
	DECLARE_CLASS_SIMPLE(CTFButtonBase, Button);

	CTFButtonBase(vgui::Panel *parent, const char *panelName, const char *text);

	virtual void ApplySettings(KeyValues *inResourceData);
	virtual void ApplySchemeSettings(IScheme *pScheme);

	// Set armed button border attributes.
	virtual void SetArmedBorder(vgui::IBorder *border);
	virtual void SetSelectedBorder(vgui::IBorder *border);
	// Get button border attributes.
	virtual IBorder *GetBorder(bool depressed, bool armed, bool selected, bool keyfocus);

	virtual void OnCursorExited();
	virtual void OnCursorEntered();
	virtual void OnMousePressed(vgui::MouseCode code);
	virtual void OnMouseReleased(vgui::MouseCode code);
	virtual MouseState GetState() { return iState; };
	//virtual void SetParent(CTFAdvButtonBase *m_pButton) { m_pParent = m_pButton; };
	//virtual char *GetCommandStr() { return m_pParent->m_szCommand; };

protected:
	virtual void	SetMouseEnteredState(MouseState flag);
	IBorder			*_armedBorder;
	IBorder			*_selectedBorder;
	MouseState		iState;

//private:
//	CTFAdvButtonBase *m_pParent;
};


#endif // tf_advbuttonbase_H
