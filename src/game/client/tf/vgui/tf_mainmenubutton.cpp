#include "cbase.h"
#include "tf_mainmenubutton.h"
#include "vgui_controls/Frame.h"
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/IInput.h>
#include "vgui_controls/Button.h"
#include "vgui_controls/ImagePanel.h"
#include "tf_controls.h"
#include <filesystem.h>
#include <vgui_controls/AnimationController.h>
#include "basemodelpanel.h"


using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DECLARE_BUILD_FACTORY_DEFAULT_TEXT(CTFMainMenuButton, CTFMainMenuButtonBase);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFMainMenuButton::CTFMainMenuButton(vgui::Panel *parent, const char *panelName, const char *text) : CTFMainMenuButtonBase(parent, panelName, text)
{
	pButton = new CTFButton(this, "ButtonNew", text);
	pButton->SetParent(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMainMenuButton::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	m_fXShift = inResourceData->GetFloat("xshift", 0.0);
	m_fYShift = inResourceData->GetFloat("yshift", 0.0);

	InvalidateLayout(false, true); // force ApplySchemeSettings to run
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMainMenuButton::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	GetText(m_szText, sizeof(m_szText));
	pButton->SetText(m_szText);
	pButton->SetCommand(m_szCommand);
	pButton->SetVisible(true);
	pButton->SetEnabled(true);
	pButton->SetPos(0, 0);
	pButton->SetZPos(2);
	pButton->SetWide(GetWide());
	pButton->SetTall(GetTall());
	pButton->SetContentAlignment(GetAlignment(m_szTextAlignment));
	pButton->SetFont(GetFont());
	pButton->SetDefaultColor(pScheme->GetColor(pDefaultText, Color(255, 255, 255, 255)), Color(0, 0, 0, 0));
	pButton->SetArmedColor(pScheme->GetColor(pArmedText, Color(255, 255, 255, 255)), Color(0, 0, 0, 0));
	pButton->SetDepressedColor(pScheme->GetColor(pDepressedText, Color(255, 255, 255, 255)), Color(0, 0, 0, 0));
	pButton->SetSelectedColor(pScheme->GetColor(pDepressedText, Color(255, 255, 255, 255)), Color(0, 0, 0, 0));
	pButton->SetArmedSound("ui/buttonrollover.wav");
	pButton->SetDepressedSound("ui/buttonclick.wav");
	pButton->SetReleasedSound("ui/buttonclickrelease.wav");
	if (m_bBorderVisible)
	{
		pButton->SetDefaultBorder(pScheme->GetBorder(pDefaultBorder));
		pButton->SetArmedBorder(pScheme->GetBorder(pArmedBorder));
		pButton->SetDepressedBorder(pScheme->GetBorder(pDepressedBorder));
		pButton->SetSelectedBorder(pScheme->GetBorder(pDepressedBorder));
	}
}

void CTFMainMenuButton::PerformLayout()
{
	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMainMenuButton::OnTick()
{
	BaseClass::OnTick();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMainMenuButton::SetDefaultAnimation()
{
	BaseClass::SetDefaultAnimation();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFMainMenuButton::SendAnimation(MouseState flag)
{
	BaseClass::SendAnimation(flag);

	AnimationController::PublicValue_t p_AnimLeave = { 0, 0, 0, 0 };
	AnimationController::PublicValue_t p_AnimHover = { m_fXShift, m_fYShift, 0, 0 };
	switch (flag)
	{
	//We can add additional stuff like animation here
	case MOUSE_DEFAULT:
		break;
	case MOUSE_ENTERED:
		vgui::GetAnimationController()->RunAnimationCommand(pButton, "Position", p_AnimHover, 0.0f, 0.1f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		break;
	case MOUSE_EXITED:
		vgui::GetAnimationController()->RunAnimationCommand(pButton, "Position", p_AnimLeave, 0.0f, 0.1f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		break;
	case MOUSE_PRESSED:
		break;
	default:
		break;
	}
}



///
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTFButton::CTFButton(vgui::Panel *parent, const char *panelName, const char *text) : CTFButtonBase(parent, panelName, text)
{
	iState = MOUSE_DEFAULT;
	vgui::ivgui()->AddTickSignal(GetVPanel());
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::OnCursorEntered()
{
	//BaseClass::OnCursorEntered();
	BaseClass::BaseClass::OnCursorEntered();
	if (iState != MOUSE_ENTERED)
	{
		SetMouseEnteredState(MOUSE_ENTERED);
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::OnCursorExited()
{
	BaseClass::BaseClass::OnCursorExited();
	
	if (iState != MOUSE_EXITED)
	{
		SetMouseEnteredState(MOUSE_EXITED);
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::OnMousePressed(vgui::MouseCode code)
{
	BaseClass::BaseClass::OnMousePressed(code);
	
	if (code == MOUSE_LEFT && iState != MOUSE_PRESSED)
	{
		SetMouseEnteredState(MOUSE_PRESSED);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::OnMouseReleased(vgui::MouseCode code)
{
	BaseClass::BaseClass::OnMouseReleased(code);
	
	if (code == MOUSE_LEFT && (iState == MOUSE_ENTERED || iState == MOUSE_PRESSED))
	{
		m_pParent->GetParent()->OnCommand(GetCommandStr());
	}
	if (code == MOUSE_LEFT && iState == MOUSE_ENTERED)
	{
		SetMouseEnteredState(MOUSE_ENTERED);
	} 
	else
	{
		SetMouseEnteredState(MOUSE_EXITED);
	}
	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTFButton::SetMouseEnteredState(MouseState flag)
{
	BaseClass::SetMouseEnteredState(flag);
	m_pParent->SendAnimation(flag);
}