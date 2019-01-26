#include <iostream>
#include <stdlib.h>
#include <string.h> 
#include <afx.h>
#include <windows.h>
#include <ATLComTime.h>
#define _NET "rasdial 天翼 [此处填写天翼号码] "
#define _TEST "echo "

void newcode(CString &con);

char pwd[6];//pwd存储输入的密码
CString path;//存储路径
CString curTime_s;//存储当前时间的字符串类型
COleDateTime curTime;//存储当前时间的date类型

int main()
{
	using namespace std;
	CString pwd_s;//pwd_s读取配置文件中密码
	CString con;//存储控制台命令
	CString lastTime;////存储读取的时间
	
	double lastTime_f = 0;//存储上一次密码产生的时间的浮点值
	
	curTime = COleDateTime::GetCurrentTime();
	//标头为ATLComTime.h
	con = _NET;
	path = "";

	//获取配置文件路径
	GetCurrentDirectory(MAX_PATH, path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	path += "\\setting.ini";

	//读配置文件
	DWORD rtv = GetPrivateProfileString("Infor", "hours", "", lastTime.GetBuffer(30), 30, path);
	//如果读取失败，rtv为default字符串的长度，也就是0.
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
	//运行指令 
	int a = system(con);	//system函数在stdlib.h中
	//命令成功返回值为0
	if (a) {
		newcode(con);
	}
	//system("pause");
	return 0;
}
void newcode(CString &con)
{
	using namespace std;
	cout << "--------------------\n请输入密码：";
	cin >> pwd;
	con += pwd;
	//写配置文件
	WritePrivateProfileString("Infor", "pwd", pwd, path);//写入密码
	curTime_s.Format("%lf", curTime.m_dt);//将double型转换为CString型
	WritePrivateProfileString("Infor", "hours", curTime_s, path);//写入时间
}