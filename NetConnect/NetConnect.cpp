#include <iostream>
#include <stdlib.h>
#include <string.h> 
#include <afx.h>
#include <windows.h>
#include <ATLComTime.h>
#define _NET "rasdial ���� [�˴���д�������] "
#define _TEST "echo "

void newcode(CString &con);

char pwd[6];//pwd�洢���������
CString path;//�洢·��
CString curTime_s;//�洢��ǰʱ����ַ�������
COleDateTime curTime;//�洢��ǰʱ���date����

int main()
{
	using namespace std;
	CString pwd_s;//pwd_s��ȡ�����ļ�������
	CString con;//�洢����̨����
	CString lastTime;////�洢��ȡ��ʱ��
	
	double lastTime_f = 0;//�洢��һ�����������ʱ��ĸ���ֵ
	
	curTime = COleDateTime::GetCurrentTime();
	//��ͷΪATLComTime.h
	con = _NET;
	path = "";

	//��ȡ�����ļ�·��
	GetCurrentDirectory(MAX_PATH, path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	path += "\\setting.ini";

	//�������ļ�
	DWORD rtv = GetPrivateProfileString("Infor", "hours", "", lastTime.GetBuffer(30), 30, path);
	//�����ȡʧ�ܣ�rtvΪdefault�ַ����ĳ��ȣ�Ҳ����0.
	if (!rtv) {
		newcode(con);
	}
	else {
		sscanf_s(lastTime, "%lf", &lastTime_f);
		if (double(curTime) - lastTime_f < 6) {
			GetPrivateProfileString("Infor", "pwd", "", pwd_s.GetBuffer(30), 30, path);
			if(pwd_s.GetLength() == 6)
				con += pwd_s;
			else
				newcode(con);
		}
		else {
			newcode(con);
		}
	}
	//����ָ�� 
	int a = system(con);	//system������stdlib.h��
	//����ɹ�����ֵΪ0
	if (a) {
		newcode(con);
	}
	//system("pause");
	return 0;
}
void newcode(CString &con)
{
	using namespace std;
	cout << "--------------------\n���������룺";
	cin >> pwd;
	con += pwd;
	//д�����ļ�
	WritePrivateProfileString("Infor", "pwd", pwd, path);//д������
	curTime_s.Format("%lf", curTime.m_dt);//��double��ת��ΪCString��
	WritePrivateProfileString("Infor", "hours", curTime_s, path);//д��ʱ��
}