//始于2018年11月22日
//作者：费才献
//单位：南京航空航天大学
//身份：研究生
#include <windows.h>//注意这个头文件的顺序，很重要。如果放在下面，可能有些函数就无法使用。
#include <iostream>
#include <stdlib.h>
#include <string.h> 
#include <stdio.h>
#include <ATLComTime.h>
#include <conio.h>//_getch()需要此头文件

#pragma comment(lib,"Winmm.lib")
#define MUSIC_PATH "C:\\Users\\Nightingale\\OneDrive\\Sounds\\"
//#define DEBUG_TOMATOPROG

typedef struct {
	char name[200] = {0};//任务名
	int time = 0;//任务时间
	int date = 0;
	COleDateTime Time1, Time2; //任务开始结束时间
	char daySeg[5];
}CTask;

//全局变量========================================================================================
int tomatoNum = 0;
int RunDateNow = int(COleDateTime::GetCurrentTime());//当前运行日期 通过类引用方法，没毛病。当然可以用对象名.方法
COleDateTime tomatoStart, tomatoEnd;//番茄开始结束时间
FILE* fp_station;
FILE* fp_nextTask;
FILE *fp;
errno_t err;
const double morning = 200;	//200分钟 8：20~11：40
const int morningEnd = 11 * 60 + 40;
const double afternoon = 220;	//220分钟 2：00~5：40
const int afternoonEnd = 17 * 60 + 40;
const double evening = 240;	//240分钟 7：00~11：00
const int eveningEnd = 23 * 60;
char daySeg[7];//时间段：上午 下午 晚上
double daySegTime;//时间段的长度
int daySegEnd;//时间段的结束时间
char nextTaskName[200] = "0";
CString path, path1;
//================================================================================================

//函数声明========================================================================================
void writeToStation(CTask *task, int i);
void writeToFile(CTask *task, int i);
void Merge(CTask *task, int i);
int dataImport(CTask *task, int i);
void nextTaskRead();
void nextTaskWrite();
int timePercent(CTask *task, int j);
int selTask(CTask *task, int i);
//================================================================================================

int main()
{
	using namespace std;
	HWND hWnd = GetConsoleWindow();
	HDC hdc = GetDC(hWnd);

	HFONT font = CreateFont(
		44, // nHeight
		0, // nWidth
		0, // 字符倾角
		0, // nOrientation
		0, // nWeight
		FALSE, // 斜体
		FALSE, // bUnderline
		0, // cStrikeOut
		ANSI_CHARSET, // nCharSet
		OUT_DEFAULT_PRECIS, // nOutPrecision
		CLIP_DEFAULT_PRECIS, // nClipPrecision
		CLEARTYPE_QUALITY, // nQuality
		DEFAULT_PITCH | FF_SWISS,
		"Arial" // nPitchAndFamily Arial
	);
	SelectObject(hdc, font);

	SetBkColor(hdc, 0x000000);
	SetTextColor(hdc, RGB(255, 98, 66));
	TextOut(hdc, 390, 30, "ALL TASKS", 9);
	ValidateRect(hWnd, 0);
	printf("\n\n\n\n\n");

//内部变量=======================================================================================
	CTask task[40];//任务数组
	int tasks = 0;//本次运行的任务数量，目前仅用于跳过第一次番茄确认
	int end = 0;//结束task     0：结束			1：未结束
	int tomato = 1;//番茄标志  0: 开启新番茄	1：退出程序
	//COleDateTime task[].Time1, task[].Time2;//任务开始结束时间
	COleDateTime restEnd;//休息结束时间
	COleDateTime leftTime;//剩余时间
	double leftMin;//剩余时间
	int tomatoProg=0;//番茄进度
	int restProg;//休息进度
	int restTime;//休息时间
	int i=-1,j;//i为任务序号，j用于遍历任务
	char c;
	bool tomatoTrue;//判断开启番茄时的输入是否正确
	bool WTF=false;//false:禁止数据写入文档  true：允许数据写入文档
	int a=0;
	int timePerDisped = 0;//用时百分比已经显示
//================================================================================================
	tomatoStart = COleDateTime::GetCurrentTime();//获取本次运行日期
	GetCurrentDirectory(MAX_PATH, path.GetBuffer(MAX_PATH));
	path.ReleaseBuffer();
	path += "\\station.dat";
	i=dataImport(task,i);														//数据导入
	j = i+1;
	//wcout << path.GetString() << endl;
	GetCurrentDirectory(MAX_PATH, path1.GetBuffer(MAX_PATH));
	path1.ReleaseBuffer();
	path1 += "\\nextTsk.dat";
	nextTaskRead();
	//-------------------------------------
	//--- 番茄循环
	//-------------------------------------   
	do {

		//开启新番茄
		do {
			if (tasks != 0) {													//跳过第一次番茄开启，既然打开了程序就默认开启番茄
				printf("\n开启新番茄请按 1 ，退出程序请按0 。\n待命中 ：");
				tomato = 2;														//tomato重置为非法值，防止scanf_s未收到整数值，旧值起作用
				scanf_s("%d", &tomato);
				tomatoTrue = ((tomato == 1) || (tomato == 0));					//输入tomato后，计算tomatoTrue
				if (!tomatoTrue) {
					cout << "输入错误，请输入0或1：";
					while ((c = getchar()) != '\n');							//清空缓冲区
				}
			}
			tomatoTrue = ((tomato == 1) || (tomato == 0));						//用于生成跳过第一次番茄的tomatoTrue计算
		} while (!tomatoTrue);

		//番茄内部循环
		if (tomato == 1) {
			//tomato = 2;//抛弃历史输入，否则会妨碍tomato判断
			tomatoStart = COleDateTime::GetCurrentTime();
			do {
				i++;
				if (tasks != 0)
					while ((c = getchar()) != '\n');							//清空缓冲区
				if (nextTaskName[0] != '0') {
					printf("我想起来了，您要进行任务：%s 是吗？\n\n如果您要进行这项任务，就请按 确认键 吧：", nextTaskName);
					if ((c = getchar()) == '\n') {
						PlaySound(TEXT(MUSIC_PATH"Assassin_s Creed Rearmed_wu.wav"), NULL, SND_FILENAME | SND_ASYNC);
						strcpy_s(task[i].name, nextTaskName);
					}
					else {
						while ((c = getchar()) != '\n');
						printf("\n\n上次您走的时候告诉我准备回来处理：%s\n\n您忘记了吗？\n\n如果您回心转意，就请按 确认键 吧。", nextTaskName);
						if ((c = getchar()) == '\n') {
							strcpy_s(task[i].name, nextTaskName);
							PlaySound(TEXT(MUSIC_PATH"Popup.SAO.Menu.wav"), NULL, SND_FILENAME | SND_ASYNC);
						}
						else
							printf("好吧，给您开新任务\n\n");
					}
					while ((c = getchar()) != '\n');
					nextTaskName[0] = '0';//销毁
				}
				else {
					int sum=0;
					for (int k = 0; k <= i; k++)
					{
						if (task[k].time == -1 || task[k].time == 0 ||task[k].name[0] == '0' || task[k].name[0] == '1')
							continue;
						sum++;
					}
					if (sum == 0) {
						printf("\n========开始任务========\n>");						
					}
					else
						printf("\n========按 1 可选择，按 0 可退出，或者 直接输入任务========\n>");
					gets_s(task[i].name, 200);									//获取任务名
				}
								
				if (task[i].name[0] == '0')
					break;
				else if (task[i].name[0] == '1') {
					do {
						a = selTask(task, i);									//选择任务或删除任务
					} while (a);
				}
				else {
					PlaySound(TEXT(MUSIC_PATH"Popup.SAO.Menu.wav"), NULL, SND_FILENAME | SND_ASYNC);
				}
				tasks++;
				task[i].Time1 = COleDateTime::GetCurrentTime();
				end = 1;														//重置结束状态，否则由于超过番茄时间而跳出任务，end保持为0，就不会给出任务耗时
				printf("\n\n结束请按0，待命中：");
				scanf_s("%d", &end);
				PlaySound(TEXT(MUSIC_PATH"Dismiss.SAO.Message.wav"), NULL, SND_FILENAME | SND_ASYNC);
				cout << endl;
				task[i].Time2 = COleDateTime::GetCurrentTime();

				task[i].time = int((task[i].Time2 - task[i].Time1) * 24 * 60);
				task[i].date = int(COleDateTime::GetCurrentTime());
				std::cout << "任务耗时 " << task[i].time << " 分钟" << endl << endl;
				tomatoEnd = COleDateTime::GetCurrentTime();
				tomatoProg = int((tomatoEnd - tomatoStart) * 24 * 60);
				
				
#ifdef DEBUG_TOMATOPROG
				tomatoProg = 26;
#endif // DEBUG_TOMATOPROG

			} while (tomatoProg < 25);											//一个番茄钟内任务循环


			//如果任务被番茄结束时间中断，给出此任务的耗时。输入end=0则不会执行此过程
			if (end==1) {
				task[i].Time2 = COleDateTime::GetCurrentTime();
				task[i].time = int((task[i].Time2 - task[i].Time1) * 24 * 60);
				std::cout << "任务耗时 " << task[i].time << " 分钟" << endl << endl;
			}

			//番茄完成提示
			if (tomatoProg >= 25) {
				printf("\n番茄成熟了，快收割吧，不然番茄会被偷走哟！＾＿－  \n\n");
			}
			else {
				cout << "您获得了一个青番茄，加油！＾＿－  " << endl << endl;
			}
			tomatoNum++;
			
			//任务成果展示
			printf( "\n\n您进行了如下任务：");
			//PlaySound(MUSIC_PATH "Popup.SAO.Message.wav", NULL, SND_FILENAME | SND_ASYNC);
			PlaySound(MUSIC_PATH "Assassin_s Creed Rearmed.wav", NULL, SND_FILENAME | SND_ASYNC);
			for ( ; j <= i; j++)											//由于任务会被强制退出，所以最后一个任务名不一定是0.这里要用<=
			{
				if (task[j].name[0] == '0' || task[j].name[0] == '1' || task[j].time == -1 || task[j].time == 0)
					continue;												//不显示任务名为0或1的任务，0和1是选择命令
				printf("\n-----------------------------------------------------------------\n");
				printf("| %-50s| 耗时%d分钟 |\n", task[j].name, task[j].time);
				timePerDisped = timePercent(task, j);						//显示用时百分比
				printf("-----------------------------------------------------------------\n");
			}
			if (timePerDisped) {//显示了用时百分比，才显示剩余时间
				leftTime = COleDateTime::GetCurrentTime();
				leftMin = (daySegEnd - (leftTime.GetHour() * 60 + leftTime.GetMinute())) / daySegTime * 100;
				if (leftMin > 100)
					printf("\n%s工作时间还未开始呢! 你急啥！\n\n", daySeg);
				else if (leftMin < 0)
					printf("工作时间已经过了，快点弄好回去吧！\n\n");
				else
					printf("您还剩 %.1f%% 的%s时间\n\n", leftMin, daySeg);
				timePerDisped = 0;//百分比显示重置为未显示
			}
			j = i + 1;//下一个番茄的任务的起点

			//如果没有退出，那么就执行休息
			if (task[i].name[0] != '0') {
				cout << "您要暂停几分钟呢？ 告诉我吧：";
				cin >> restTime;
				cout <<endl<< "好的，" << restTime << "分钟后再回来吧，我会提醒您的。(〃'▽'〃)" << endl;
				do {
					restEnd = COleDateTime::GetCurrentTime();
					restProg = int((restEnd - tomatoEnd) * 24 * 60);
				} while (restProg < restTime);
				mciSendString("play " MUSIC_PATH "stop.mp3", NULL, 0, NULL);
			}
		}
		
	} while (tomato == 1 && task[i].name[0] != '0');


	//合并相同任务
	Merge(task, i);


	//-------------------------------------
	//--- 控制台显示成果
	//-------------------------------------
	printf("\n\n您完成了%d个番茄，任务汇总如下：\n", tomatoNum);
	for (j=0; j <=i; j++) 
	{
		if (task[j].name[0] == '0' || task[j].name[0] == '1' || task[j].time == -1 || task[j].time == 0)
			continue;
		cout << endl;
		printf("-----------------------------------------------------------------\n");
		if(task[j].time>59)
			printf("| %-38s| %s：耗时%d小时%2d分钟 |\n", task[j].name, task[j].daySeg, task[j].time / 60, task[j].time % 60);
		else
			printf("| %-44s| %s：耗时%d分钟 |\n", task[j].name, task[j].daySeg, task[j].time);
		timePercent(task, j);
		printf("-----------------------------------------------------------------\n");
	}

	//数据写入station
	writeToStation(task, i);
	nextTaskWrite();
	
	return 0;
}

//数据导入内存
int dataImport(CTask *task, int i) {
	fopen_s(&fp_station, path, "a+");			//将打开并读取文件和追加，如果它不存在，则创建该文件。
	while (fread(&task[++i], sizeof(CTask), 1, fp_station) == 1);//数据读入内存
	i--;
	if (task[0].date != RunDateNow && i!=-1)	//日期不同且文件不为空，i=-1表明文件为空
		writeToFile(task, i+1);					//数据写入文件
	fclose(fp_station);
	fp_station = NULL;
	return i;
}

//数据写入station
void writeToStation(CTask *task, int i) {		//数据写入station
	int j = 0;
	fopen_s(&fp_station, path, "w");			//重写文件
	for (; j < i; j++) {						//跳过退出时输入的任务名0
#ifdef DEBUG_FEI
		if (task[j].time == -1)
#else
		if (task[j].time == -1 || task[j].time == 0 || task[j].name[0] == '0')//跳过由于合并而时间被置为-1的任务，跳过执行时间不超过1分钟的任务
#endif // DEBUG_FEI
			continue;
		fwrite(&task[j], sizeof(CTask), 1, fp_station);
	}
	fclose(fp_station);
	fp_station = NULL;
}

//数据写入文件
void writeToFile(CTask *task, int i) {
	using namespace std;
	err = fopen_s(&fp, "C:\\Users\\Nightingale\\Desktop\\Tomato.doc", "a");

	COleDateTimeSpan t = 1;
	tomatoStart -= t;//今天减1以获得一个昨天的时间
	if (tomatoStart.GetDayOfWeek() == 1) {
		fprintf(fp, "日期：%s\t 星期7\n", LPCSTR(tomatoStart.Format("%Y-%m-%d")));
	}
	else {
		fprintf(fp, "日期：%s\t 星期%d\n", LPCSTR(tomatoStart.Format("%Y-%m-%d")), (tomatoStart.GetDayOfWeek() - 1));
	}
	fprintf(fp, "任务情况如下：\n");
	for (int j = 0; j < i; j++)//跳过退出时输入的任务名0
	{
		if (task[j].time == -1 || task[j].time == 0)//跳过由于合并而时间被置为-1的任务，跳过执行时间不超过1分钟的任务
			continue;
		cout << endl;
		fprintf(fp, "-----------------------------------------------------------------\n");
		if (task[j].time > 59)
			fprintf(fp,"| %-38s| %s：耗时%d小时%2d分钟 |\n", task[j].name, task[j].daySeg, task[j].time / 60, task[j].time % 60);
		else
			fprintf(fp,"| %-44s| %s：耗时%d分钟 |\n", task[j].name, task[j].daySeg, task[j].time);
		task[j].time = 0;//销毁数据
	}
	fprintf(fp, "-----------------------------------------------------------------\n\n");
	fclose(fp);
	fp = NULL;
}

//数据合并
void Merge(CTask *task,int i) {
	int j;
	for (int m = 0; m < i; m++) {
		for (j = m + 1; j < i; j++)
		{
			if (strcmp(task[m].name, task[j].name) == 0 && strcmp(task[m].daySeg, task[j].daySeg) == 0) {//同一时间段的相同任务合并
				task[m].time += task[j].time;
				task[j].time = -1;//被合并的任务时间置为-1
			}
		}
	}
}

//显示时间分配
int timePercent(CTask *task, int j) {
	if (task[j].Time1.GetHour() < 12) {
		strcpy_s(task[j].daySeg, "上午");
		daySegTime = morning;
		daySegEnd = morningEnd;
		printf("\n您分配了 %.1f%% 的上午时间用于: %s\n", task[j].time / morning * 100, task[j].name);
	}
	else if (task[j].Time1.GetHour() >= 12 && task[j].Time1.GetHour() < 18) {
		strcpy_s(task[j].daySeg, "下午");
		daySegTime = afternoon;
		daySegEnd = afternoonEnd;
		printf("\n您分配了 %.1f%% 的下午时间用于: %s\n", task[j].time / afternoon * 100, task[j].name);
	}
	else if (task[j].Time1.GetHour() >= 18) {
		strcpy_s(task[j].daySeg, "晚上");
		daySegTime = evening;
		daySegEnd = eveningEnd;
		printf("\n您分配了 %.1f%% 的晚上时间用于: %s\n", task[j].time / evening * 100, task[j].name);
	}
	return 1;
}

int selTask(CTask *task, int i) {
	using namespace std;
	int delTask = 0;//是否删除了任务，如果是则为1
	char index[200] = { 0 };//存储选择时输入数据
	int index0, index1;//存储index字符数组的字符对应整数值。

	for (int j = 0; j < i; j++)//第i个task存储的是输入的1，不是任务，故不访问
	{
		if (task[j].name[0] == '0' || task[j].name[0] == '1' || task[j].time == -1)
			continue;
		printf("\n%d. %s\n", j, task[j].name);
	}
	printf("\n======请选择任务序号，序号前加d可删除任务，可直接输入任务======\n\n请指示：");
	gets_s(index, 20);
	//while ((c = getchar()) != '\n');//清空缓冲区
	index0 = int(index[0]) - 48;

	if (index0 >= 0 && index0 < i) {//先判断第一个字符值是否在0~i之间
		int m;
		if (int(index[1]) != 0)//如果有第二个字符，将两个字符合成期望的数字
			index0 = index0 * 10 + int(index[1]);//有的话，
		for (m = 0; m < i; m++)//找到这个任务，使用这个任务名，继续执行这个任务
		{
			if (index0 == m) {
				strcpy_s(task[i].name, task[index0].name);
				break;
			}
		}
		if (m == i) {
			if (index0 == 0)
				printf("\n\nt提醒您，这里输入 0 不是退出命令哦！\n\n");
			
			printf("\n列表中没有这个任务，已将它视为新任务。\n\n");
			strcpy_s(task[i].name, index);
		}
	}
	else {
		if (int(index[1]) == 0)
			strcpy_s(task[i].name, index);//超出选择范围则视为输入任务，获取任务名
		else {
			index1 = int(index[1]) - 48;
			if (index0 == (int('d') - 48) && index1 >= 0 && index1 < i) {//如果第一个字符为d，说明要删除任务
				if (int(index[2]) != 0)//如果有第三个字符，将第二个字符和第二个字符合成期望的数字
					index1 = index1 * 10 + int(index[2]);
				printf("\n您删除了任务：%s\n\n剩余任务列表如下：\n\n", task[index1].name);
				strcpy_s(task[index1].name, "0");//删除指定任务
				delTask = 1;//指示执行了任务删除操作
				i--;
				PlaySound(TEXT(MUSIC_PATH"Dismiss.ALO.Message.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
			else {
				strcpy_s(task[i].name, index);//若不选择则视为输入任务，获取任务名
				PlaySound(TEXT(MUSIC_PATH"Popup.SAO.Menu.wav"), NULL, SND_FILENAME | SND_ASYNC);
			}
		}
	}
	return delTask;
}

void nextTaskRead() {
	fopen_s(&fp_nextTask, path1, "r");
	if (fp_nextTask != NULL) {
		fread(nextTaskName, sizeof(nextTaskName), 1, fp_nextTask);
		fclose(fp_nextTask);
		fp_nextTask = NULL;
	}
}
void nextTaskWrite() {
	int c;
	fopen_s(&fp_nextTask, path1, "w");
	
	printf("\n\n再见！ \n\n等会儿，可不可以， 先告诉我接下来您要干什么呀？有点小好奇。\n\n嗯，就告诉我一个人：");
	while ((c = getchar()) != '\n');
	gets_s(nextTaskName, 200);//获取任务名
	if (nextTaskName[0] == '0')
		printf("\n\n好吧，保留您的小秘密。\n\n不过，您要是累了，请您一定要稍稍休息一下。\n\n");
	else
		printf("\n\n收到，谢谢您耐心的陪伴，再会！\n\n");
	fprintf(fp_nextTask, "%s", nextTaskName);
	fclose(fp_nextTask);
	fp_nextTask = NULL;
	system("pause");
}