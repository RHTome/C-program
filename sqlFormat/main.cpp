
/********************************************************************************
 * Date: 2019-1-26																*
 * By Calix Fei																	*
 * Copyright© 2019, 1952 NanJing University of aeronaustics and atronaustics.	*
 * All rights reserved.															*
 * This program is used for formatting SQL.										*
 ********************************************************************************/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <afx.h>
#include <iostream>

void dataDefine();
void lineFeed(CString &Buffer);
void wordReplace(CString &Buffer);
BOOL CopyToClipboard(const char* pszData, const int nDataLen);
BOOL GetTextFromClipboard();

CStringArray foreFeedSearchWord;		//前换行查找词
CStringArray lattFeedSearchWord;		//后换行查找词
CStringArray ReplaceWord;				//替换词
const int fileLen = 10000;				//设置文件内容最大长度

const CString SPACE = " ";				//空格
const CString ENTER = "\n";				//回车


int main()
{
	dataDefine();																					//数据定义
	TCHAR* pszFileNameRead  = _T("C:/Users/Nightingale/Desktop/cope_doc.txt");						//读文件路径
	TCHAR* pszFileNameWrite = _T("C:/Users/Nightingale/Desktop/ok_doc.sql");						//写文件路径

	CStdioFile myFile;																				//文件对象
	CString Buffer;																					//文件内容
	CFileException fileException;																	//文件异常
	setlocale(LC_CTYPE, ("chs"));																	//中文支持
	

	//读文件
	if (!myFile.Open(pszFileNameRead, CFile::typeText | CFile::modeReadWrite, &fileException))		
	{
		TRACE("Can't open file %s, error = %u\n", pszFileNameRead, fileException.m_cause);
	}
	else
	{
		char Buf[fileLen]="";																		//读取文件，存入Buffer
		myFile.Read(Buf, fileLen);
		Buffer = Buf;
		
		lineFeed(Buffer);																			//换行
		wordReplace(Buffer);																		//替换
		//printf("%S", Buffer);	
		//std::wcout << Buffer.GetBuffer();
	}

	//读完清空
	myFile.SetLength(0);
	myFile.Close();

	//写文件
	if (!myFile.Open(pszFileNameWrite, CFile::modeCreate| CFile::modeNoTruncate | CFile::modeWrite, &fileException))
	{
		TRACE("Can't open file %s, error = %u\n", pszFileNameWrite, fileException.m_cause);
	}
	else
	{
		myFile.WriteString(Buffer);											
	}

	//关闭文件
	myFile.Close();

	//打开文件
	if(system("E:/feicaixian/SublimeText3/subl.exe C:/Users/Nightingale/Desktop/ok_doc.sql"))
		MessageBox(NULL, _T("请在源码中配置subl路径"), _T("提示"), MB_OK);
	//复制结果到剪贴板
	char temp[fileLen];
	::wsprintfA(temp, "%ls", (LPCTSTR)Buffer);
	const char * pszData = temp;
	if (!CopyToClipboard(pszData, fileLen))
		MessageBox(NULL, _T("复制到剪贴板出现错误"), _T("提示"), MB_OK);

	return 0;
}

//自动换行
void lineFeed(CString &Buffer)
{
	int m_index = 0;																		//当前查找到的位置
	int m_index_pre = 0;																	//上次查找到的位置
	CString BufferTemp = Buffer;
	BufferTemp.MakeUpper();																	//将文件内容复制一份，进行大写转换，因为CString的find函数区分大小写
	
	//后换行
	for (int i = 0; i < foreFeedSearchWord.GetSize(); i++)									//遍历foreFeedSearchWord中的所有查找词
	{
		while (m_index != -1)																//未找到表示遍历完毕，退出循环
		{
			m_index = BufferTemp.Find(foreFeedSearchWord.GetAt(i), m_index_pre);			//从上次查找位置的后一个字符继续向后查找

			if (m_index != -1)																//找到指定字符才能插入
			{
				if (BufferTemp.GetAt(m_index + foreFeedSearchWord.GetAt(i).GetLength()) == SPACE)	//如果查找词后面是空格，那么从空格开始换行
				{
					Buffer.Insert(m_index + foreFeedSearchWord.GetAt(i).GetLength() + 1, '\n');
					BufferTemp.Insert(m_index + foreFeedSearchWord.GetAt(i).GetLength() + 1, '\n');
				}
				else
				{
					if (BufferTemp.GetAt(m_index + 1) != ENTER)										//如果查找词后面是换行，那么不用换行
					{
						Buffer.Insert(m_index + foreFeedSearchWord.GetAt(i).GetLength(), '\n');
						BufferTemp.Insert(m_index + foreFeedSearchWord.GetAt(i).GetLength(), '\n');	//两份文件内容同步修改，保持查找词位置一致
					}
				}
				m_index_pre = m_index + 1;															//下次查找不能从上次查找位置开始，否则找到的还是上次的位置
			}
		}
		m_index_pre = 0;																			//m_index_pre复位，继续从头开始查找下一个查找词
		m_index = 0;																				//m_index复位，上一个查找词查完时m_index=-1
	}

	//前换行
	for (int i = 0; i < lattFeedSearchWord.GetSize(); i++)									//遍历foreFeedSearchWord中的所有查找词
	{
		while (m_index != -1)																//未找到表示遍历完毕，退出循环
		{
			m_index = BufferTemp.Find(lattFeedSearchWord.GetAt(i), m_index_pre);			//从上次查找位置的后一个字符继续向后查找

			if (m_index != -1)																//找到指定字符才能插入
			{
				if (BufferTemp.GetAt(m_index - 1) != ENTER)									//如果查找词前面是换行，那么不用换行
				{
					Buffer.Insert(m_index, '\n');
					BufferTemp.Insert(m_index, '\n');										//两份文件内容同步修改，保持查找词位置一致
				}
				m_index_pre = m_index + 1;													//下次查找不能从上次查找位置开始，否则找到的还是上次的位置
			}
		}
		m_index_pre = 0;																	//m_index_pre复位，继续从头开始查找下一个查找词
		m_index = 0;																		//m_index复位，上一个查找词查完时m_index=-1
	}
}

//单词替换
void wordReplace(CString &Buffer)
{
	int m_index = 0;																	//当前查找到的位置
	int m_index_pre = 0;																//上次查找到的位置
	CString BufferTemp = Buffer;
	CString Word;
	BufferTemp.MakeUpper();																//将文件内容复制一份，进行大写转换，因为CString的find函数区分大小写
	for (int i = 0; i < ReplaceWord.GetSize(); i++)										//遍历SearchWord中的所有查找词
	{
		m_index = BufferTemp.Find(ReplaceWord.GetAt(i));								//从0开始检索，检索不到则返回 - 1

		if (m_index != -1)																//找到指定字符才能插入。
		{
			Word = Buffer.Mid(m_index, ReplaceWord.GetAt(i).GetLength());
			Buffer.Replace(Word, ReplaceWord.GetAt(i));
		}
		m_index = 0;																	//m_index复位，上一个查找词查完时m_index=-1
	}
}


//数据定义
void dataDefine()
{
	foreFeedSearchWord.Add(",");														//定义后换行查找词

	lattFeedSearchWord.Add("FROM");														//定义前换行查找词
	lattFeedSearchWord.Add("ON");
	lattFeedSearchWord.Add("ORDER");
	lattFeedSearchWord.Add("AND");

	ReplaceWord.Add("FROM");															//定义替换词
	ReplaceWord.Add("SELECT");
	ReplaceWord.Add("ON");
	ReplaceWord.Add("ORDER");
	ReplaceWord.Add("AND");
}

// 复制数据到剪贴板
BOOL CopyToClipboard(const char* pszData, const int nDataLen)
{
	if (::OpenClipboard(NULL))
	{
		::EmptyClipboard();
		HGLOBAL clipbuffer;
		char *buffer;
		clipbuffer = ::GlobalAlloc(GMEM_DDESHARE, nDataLen + 1);
		buffer = (char *)::GlobalLock(clipbuffer);
		strcpy(buffer, pszData);
		::GlobalUnlock(clipbuffer);
		::SetClipboardData(CF_TEXT, clipbuffer);
		::CloseClipboard();
		return TRUE;
	}
	return FALSE;
}


// 从剪切板中取得数据
BOOL GetTextFromClipboard()
{
	if (::OpenClipboard(NULL))
	{
		//获得剪贴板数据
		HGLOBAL hMem = GetClipboardData(CF_TEXT);
		if (NULL != hMem)
		{
			char* lpStr = (char*)::GlobalLock(hMem);
			if (NULL != lpStr)
			{
				//MessageBox(0, lpStr, "", 0);
				::GlobalUnlock(hMem);
			}
		}
		::CloseClipboard();
		return TRUE;
	}
	return FALSE;
}
