
// shellcodeLoader.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CshellcodeLoaderApp: 
// �йش����ʵ�֣������ shellcodeLoader.cpp
//

class CshellcodeLoaderApp : public CWinApp
{
public:
	CshellcodeLoaderApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CshellcodeLoaderApp theApp;