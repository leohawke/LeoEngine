//**************************************************************************/
// Copyright (c) 1998-2007 Autodesk, Inc.
// All rights reserved.
// 
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information written by Autodesk, Inc., and are
// protected by Federal copyright law. They may not be disclosed to third
// parties or copied or duplicated in any form, in whole or in part, without
// the prior written consent of Autodesk, Inc.
//**************************************************************************/
// DESCRIPTION: Appwizard generated plugin
// AUTHOR: 
//***************************************************************************/

#include "IEnumProc.h"

#define Export_l3d_CLASS_ID	Class_ID(0x72d5f650, 0x8fb7456f)

class Export_l3d : public SceneExport {
public:
	//Constructor/Destructor
	Export_l3d();
	~Export_l3d();

	int				ExtCount();					// Number of extensions supported
	const TCHAR *	Ext(int n);					// Extension #n (i.e. "3DS")
	const TCHAR *	LongDesc();					// Long ASCII description (i.e. "Autodesk 3D Studio File")
	const TCHAR *	ShortDesc();				// Short ASCII description (i.e. "3D Studio")
	const TCHAR *	AuthorName();				// ASCII Author name
	const TCHAR *	CopyrightMessage();			// ASCII Copyright message
	const TCHAR *	OtherMessage1();			// Other message #1
	const TCHAR *	OtherMessage2();			// Other message #2
	unsigned int	Version();					// Version number * 100 (i.e. v3.01 = 301)
	void			ShowAbout(HWND hWnd);		// Show DLL's "About..." box

	BOOL SupportsOptions(int ext, DWORD options);
	int  DoExport(const TCHAR *name,ExpInterface *ei,Interface *i, BOOL suppressPrompts=FALSE, DWORD options=0);
};



class Export_l3dClassDesc : public ClassDesc2 
{
public:
	virtual int IsPublic() 							{ return TRUE; }
	virtual void* Create(BOOL /*loading = FALSE*/) 		{ return new Export_l3d(); }
	virtual const TCHAR *	ClassName() 			{ return GetString(IDS_CLASS_NAME); }
	virtual SClass_ID SuperClassID() 				{ return SCENE_EXPORT_CLASS_ID; }
	virtual Class_ID ClassID() 						{ return Export_l3d_CLASS_ID; }
	virtual const TCHAR* Category() 				{ return GetString(IDS_CATEGORY); }

	virtual const TCHAR* InternalName() 			{ return _T("Export_l3d"); }	// returns fixed parsable name (scripter-visible name)
	virtual HINSTANCE HInstance() 					{ return hInstance; }					// returns owning module handle
	

};


ClassDesc2* GetExport_l3dDesc() { 
	static Export_l3dClassDesc Export_l3dDesc;
	return &Export_l3dDesc; 
}





INT_PTR CALLBACK Export_l3dOptionsDlgProc(HWND hWnd,UINT message,WPARAM,LPARAM lParam) {
	static Export_l3d* imp = nullptr;

	switch(message) {
		case WM_INITDIALOG:
			imp = (Export_l3d *)lParam;
			CenterWindow(hWnd,GetParent(hWnd));
			return TRUE;

		case WM_CLOSE:
			EndDialog(hWnd, 0);
			return 1;
	}
	return 0;
}


//--- Export_l3d -------------------------------------------------------
Export_l3d::Export_l3d()
{

}

Export_l3d::~Export_l3d() 
{

}

int Export_l3d::ExtCount()
{
	return 1;
}

const TCHAR *Export_l3d::Ext(int /*i*/)
{		
	return _T("l3d");
}

const TCHAR *Export_l3d::LongDesc()
{
	return _T("l3d 0.01 Model File");
}

const TCHAR *Export_l3d::ShortDesc() 
{			
	return _T("l3d file");
}

const TCHAR *Export_l3d::AuthorName()
{			
	return _T("Leo-Hawke");
}

const TCHAR *Export_l3d::CopyrightMessage() 
{	
	return _T("LGPL V3");
}

const TCHAR *Export_l3d::OtherMessage1() 
{		
	//TODO: Return Other message #1 if any
	return _T("");
}

const TCHAR *Export_l3d::OtherMessage2() 
{		
	//TODO: Return other message #2 in any
	return _T("");
}

unsigned int Export_l3d::Version()
{				
	return 1;
}

void Export_l3d::ShowAbout(HWND /*hWnd*/)
{			
	// Optional
}

BOOL Export_l3d::SupportsOptions(int /*ext*/, DWORD /*options*/)
{
	#pragma message(TODO("Decide which options to support.  Simply return true for each option supported by each Extension the exporter supports."))
	return TRUE;
}


int	Export_l3d::DoExport(const TCHAR* name, ExpInterface* ei, Interface* ip, BOOL suppressPrompts, DWORD /*options*/)
{
	AllocConsole();
	/*
	if(!suppressPrompts)
		DialogBoxParam(hInstance, 
				MAKEINTRESOURCE(IDD_PANEL), 
				GetActiveWindow(), 
				Export_l3dOptionsDlgProc, (LPARAM)this);
	*/
	Tree::EnumProc proc(ip);
	ei->theScene->EnumTree(&proc);
	proc.Save(name);
	FreeConsole();
	return TRUE;
}


