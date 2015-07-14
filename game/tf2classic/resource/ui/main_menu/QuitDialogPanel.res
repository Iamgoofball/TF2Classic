"Resource/UI/main_menu/QuitDialogPanel.res"
{
	"CTFQuitDialogPanel"
	{
		"ControlName"	"EditablePanel"
		"fieldName"		"CTFQuitDialogPanel"
		"xpos"			"c-200"
		"ypos"			"c-100"
		"wide"			"400"
		"tall"			"200"
		"autoResize"	"0"
		"pinCorner"		"0"
		"visible"		"1"
		"enabled"		"1"
		"alpha"			"0"
		"border"		"TFFatLineBorderOpaque"
	}
	
	"Logo"
	{
		"ControlName"	"CTFImagePanel"
		"fieldName"		"Logo"
		"xpos"			"10"
		"ypos"			"55"	
		"zpos"			"2"		
		"wide"			"75"
		"tall"			"75"
		"visible"		"0"
		"enabled"		"1"
		"image"			"../hud/eng_eureka_tele_home"
		"scaleImage"	"1"	
	}		
		
	"QuitLabel"
	{
		"ControlName"		"CExLabel"
		"fieldName"			"QuitLabel"
		"xpos"				"50"
		"ypos"				"70"
		"zpos"				"5"
		"wide"				"300"
		"tall"				"50"
		"autoResize"		"0"
		"pinCorner"			"0"
		"visible"			"1"
		"enabled"			"1"
		"labelText"			"Are you sure?"
		"textAlignment"		"center"
		"font"				"ScoreboardTeamNameLarge"
		"fgcolor"			"White"
	}	
	
	"ConfirmButton"
	{
		"ControlName"		"CTFAdvButton"
		"fieldName"			"ConfirmButton"
		"xpos"				"290"
		"ypos"				"180"
		"zpos"				"6"
		"wide"				"50"
		"tall"				"20"
		"visible"			"1"
		"enabled"			"1"
		"labelText" 		"QUIT"
		"textAlignment"		"center"
		"bgvisible"			"0"
		"bordervisible"		"0"
		"command"			"quitconfirm"		
	}			

	"CancelButton"
	{
		"ControlName"		"CTFAdvButton"
		"fieldName"			"CancelButton"
		"xpos"				"340"
		"ypos"				"180"
		"zpos"				"6"
		"wide"				"60"
		"tall"				"20"
		"visible"			"1"
		"enabled"			"1"
		"labelText" 		"Cancel"
		"textAlignment"		"east"
		"bgvisible"			"0"
		"bordervisible"		"0"
		"command"			"vguicancel"		
	}	
}
		