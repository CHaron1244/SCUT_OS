#include <iostream>
#include <graphics.h>
#include <fstream>
#include <windows.h>
#include <windows.h>
#include<sstream>

using namespace std;

#define BUF_SIZE 8192

//newfile 123.txt 111111 cmyisniubi
//copy C:\Users\hikari\Desktop\789.txt /cmy host 
//write /cmy/123.txt 123456

/*���ں���*/
void LogIn();
void SysUserInteractiveUI();
void ComUserInteractiveUI();
void ShowError();

/*��������*/
void ShareMemoryClear(LPVOID& ShareMemoryPointer);//��չ����ڴ�
void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer);//�������ڴ��е����ݿ�������


/*ָ��ռ䣬���ڿͻ��������������ָ��*/
	//���������ļ����
HANDLE InstructionSpace = CreateFileMapping(
	INVALID_HANDLE_VALUE,   // �����ļ����
	NULL,   // Ĭ�ϰ�ȫ����
	PAGE_READWRITE,   // �ɶ���д
	0,   // ��λ�ļ���С
	BUF_SIZE,   // ��λ�ļ���С
	"InstructionSpace"   // �����ڴ�����
);

//ӳ�仺������ͼ���õ�ָ�����ڴ��ָ��
LPVOID InstructionSend = MapViewOfFile(
	InstructionSpace,            // �����ڴ�ľ��
	FILE_MAP_ALL_ACCESS, // �ɶ�д���
	0,
	0,
	BUF_SIZE
);

/*Ȩ�޿ռ䣬���ڸ�֪�������û������*/
HANDLE AccSpace = CreateFileMapping(
	INVALID_HANDLE_VALUE,   // �����ļ����
	NULL,   // Ĭ�ϰ�ȫ����
	PAGE_READWRITE,   // �ɶ���д
	0,   // ��λ�ļ���С
	BUF_SIZE,   // ��λ�ļ���С
	"AccSpace"   // �����ڴ�����
);

//ӳ�仺������ͼ���õ�ָ�����ڴ��ָ��
LPVOID AccSend = MapViewOfFile(
	AccSpace,            // �����ڴ�ľ��
	FILE_MAP_ALL_ACCESS, // �ɶ�д���
	0,
	0,
	BUF_SIZE
);


int main()
{
	LogIn();
    return 0;
}

void LogIn() 
{
begin:
	ExMessage m;//����������
	/*��ʼ��*/
	initgraph(350, 250); //���廭����С����ȣ�����
	setbkcolor(WHITE);//���ñ�����ɫ
	cleardevice();//ˢ��
	char account[20] = {};
	char password[15] = {};
	fstream f;

	/*���ÿ�Ϳ��е���Ϣ*/
	setlinecolor(BLACK);
	rectangle(100, 78, 240, 98);//�˺ſ�
	rectangle(100, 128, 240, 148);//�����
	rectangle(60, 178, 290, 208);//��¼��
	rectangle(250, 78, 290, 98);//�˺������
	rectangle(250, 128, 290, 148);//���������
	RECT r1 = { 60,178,290,208 };
	settextcolor(BLACK);
	drawtext(("��¼"), &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//��¼�ı�
	RECT r2 = { 250,78,290,98 };
	settextcolor(BLACK);
	drawtext(("����"), &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//�˺�����
	RECT r3 = { 250,128,290,148 };
	settextcolor(BLACK);
	drawtext(("����"), &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//��������

	/*�����ı�*/
	settextstyle(16, 0, ("����"));//�������ִ�С ��ʽ 
	setbkmode(TRANSPARENT);// ȥ�����ֱ���
	//������������� ����������ʼλ��x ��ʼλ��y ����������ַ�������
	settextcolor(BLACK);
	outtextxy(60, 80, ("�˺�"));
	outtextxy(60, 130, ("����"));
	settextstyle(12, 0, ("����"));
	settextstyle(16, 0, ("����"));
	outtextxy(73, 30, ("��ӭ����hikari�ļ�ϵͳ��"));
	

	/*��ť�趨*/
	
	while (1) {
		m = getmessage(EM_MOUSE | EM_KEY);
		if (m.x >= 60 && m.y >= 178 && m.x <= 290 && m.y <= 208) {//��¼��
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				if (account[0] == '1' && account[1] == '2' && account[2] == '3' && account[3] == '\0'&& password[0] == '1' && password[1] == '2' && password[2] == '3' && password[3] == '\0') {//ϵͳ�û�
					SysUserInteractiveUI();//��������
					goto begin;
				}
				else if (account[0] == '3' && account[1] == '2' && account[2] == '1' && account[3] == '\0' && password[0] == '3' && password[1] == '2' && password[2] == '1' && password[3] == '\0') {//��ͨ�û�
					ComUserInteractiveUI();//��������
					goto begin;
				}
				else {
					ShowError();
					goto begin;
				}
			}
		}
		else if (m.x >= 250 && m.y >= 78 && m.x <= 290 && m.y <= 98) {//�˺�����
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				InputBox(account, 20, "����������˺�");
				clearrectangle(101, 79, 239, 97);
				outtextxy(100, 78, account);
			}
		}
		else if (m.x >= 250 && m.y >= 128 && m.x <= 290 && m.y <= 148) {//��������
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				InputBox(password, 15, "�������������");
				clearrectangle(101, 129, 239, 147);
				for (int i = 0; i < strlen(password); i++) {
					outtextxy(100 + i * 8, 128, L'*');
				}
			}
		}
	}
	return;
}

void ShowError() 
{
	settextstyle(12, 0, "����");
	settextcolor(RED);
	outtextxy(200, 157, "�˺Ż��������");
	settextcolor(BLACK);
	Sleep(300);
	clearrectangle(200, 157, 300, 170);
}

void SysUserInteractiveUI()
{
begin:
	ExMessage m;//����������
	/*��ʼ��*/
	initgraph(550, 350); //���廭����С����ȣ�����
	setbkcolor(WHITE);//���ñ�����ɫ
	cleardevice();//ˢ��
	char instruction[BUF_SIZE] = {};
	char password[15] = {};
	fstream f;

	/*���ÿ�Ϳ��е���Ϣ*/
	setlinecolor(BLACK);
	rectangle(15, 40, 535, 270);//��Ϣ��
	rectangle(15, 310, 450, 330);//�����ı���
	rectangle(465, 15, 535, 35);//���ؿ�
	rectangle(465, 280, 535, 300);//�����
	rectangle(465, 310, 535, 330);//ȷ�Ͽ�
	RECT r1 = { 465,280, 535, 300 };
	settextcolor(BLACK);
	drawtext(_T("����"), &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//����ָ��
	RECT r2 = { 465, 310, 535, 330 };
	settextcolor(BLACK);
	drawtext(_T("ȷ��"), &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//ȷ��ָ��
	RECT r3 = { 465, 15, 535, 35 };
	settextcolor(BLACK);
	drawtext(_T("����"), &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//������һ��

	/*�����ı�*/
	settextstyle(16, 0, "����");//�������ִ�С ��ʽ 
	setbkmode(TRANSPARENT);// ȥ�����ֱ���
	//������������� ����������ʼλ��x ��ʼλ��y ����������ַ�������
	settextcolor(BLACK);
	settextstyle(16, 0, "����");//�������ִ�С ��ʽ 
	outtextxy(15, 15, "��ǰĿ¼:");
	outtextxy(90, 15, ("/cmy"));
	outtextxy(195, 285, "������ָ��");

	/*��ť�趨*/

begin2:
	while (1) {
		m = getmessage(EM_MOUSE | EM_KEY);
		if (m.x >= 465 && m.y >= 280 && m.x <= 535 && m.y <= 300) {//ָ������
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				if (m.message == WM_LBUTTONUP) {//�������ʱ
					InputBox(instruction, BUF_SIZE, "������ָ��");
					clearrectangle(17, 312, 448, 328);
					outtextxy(17, 312, instruction);
				}
			}
		}
		else if (m.x >= 465 && m.y >= 310 && m.x <= 535 && m.y <= 330) {//ȷ��
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				clearrectangle(16, 41, 534, 269);//�����Ϣ��
				char ReplyInfoRecBuffer[BUF_SIZE] = { 0 };//���ܻظ���Ϣ������

				
				//�����ݿ����������ڴ�
				strcpy_s((char*)InstructionSend, strlen(instruction) + 1, instruction);
				strcpy_s((char*)AccSend, strlen("0") + 1, "0");

				/*����write�Ƿ�ռ�õ��ж�*/
				HANDLE WriteSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "WriteSpace");//���ϵشӻظ���Ϣ�ռ��л�ȡ����
				LPVOID WriteRec = MapViewOfFile(WriteSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				istringstream InputSS(instruction);
				string WritingFile;
				if (WriteRec)
				{
					WritingFile = string((char*)WriteRec);
				}
				else
				{
					WritingFile = "";
				}
				
				string InputInstruction;
				string InputFile;
				InputSS >> InputInstruction;
				InputSS >> InputFile;
				if (InputFile == WritingFile && InputInstruction == "write")
				{
					clearrectangle(90, 15, 200, 30);
					outtextxy(90, 15, ReplyInfoRecBuffer);
					outtextxy(16, 41, "��ǰ�ļ����ڱ������û�д�룬���Ժ����ԣ�");
					char NULLChar[] = "";//�������
					strcpy_s((char*)InstructionSend, strlen(NULLChar) + 1, NULLChar);
					goto begin2;
				}

				while (1)
				{
					HANDLE ReplyInfoSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ReplyInfoSpace");//���ϵشӻظ���Ϣ�ռ��л�ȡ����
					LPVOID ReplyInfoRec = MapViewOfFile(ReplyInfoSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
					if (!ReplyInfoRec)continue;
					if (strlen((char*)ReplyInfoRec))
					{
						MemoryInfoCpy(ReplyInfoRecBuffer, ReplyInfoRec);
						break;//�����ȡ�������ݣ�break
					}
				}

				clearrectangle(17, 42, 533, 268);//��������ı���

				/*������ص���Ϣ����Ϣ��*/
				if (ReplyInfoRecBuffer[0] == '/')
				{
					clearrectangle(90, 15, 200, 30);
					outtextxy(90, 15, ReplyInfoRecBuffer);
					outtextxy(16, 41, "��ת�ɹ���");
					continue;
				}
				int CountChangeLine = 1;
				string InputStr(ReplyInfoRecBuffer);
				for (int i = 0; i < InputStr.length(); i++)
				{
					if (InputStr[i] == '\n')
					{
						InputStr[i] = ' ';
						CountChangeLine++;
					}
				}
				istringstream InputSStr(InputStr);
				for (int i = 0; i < CountChangeLine; i++)
				{
					InputSStr >> InputStr;
					outtextxy(16, 41 + 18 * i, InputStr.c_str());
				}
			}
		}
		else if (m.x >= 465 && m.y >= 15 && m.x <= 535 && m.y <= 35) {//����
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				return;
			}
		}
	}

	/*���ָ��ռ��ļ�ӳ��*/
	UnmapViewOfFile(InstructionSend);
	CloseHandle(InstructionSpace);

	return;
}

void ComUserInteractiveUI()
{
begin:
	ExMessage m;//����������
	/*��ʼ��*/
	initgraph(550, 350); //���廭����С����ȣ�����
	setbkcolor(WHITE);//���ñ�����ɫ
	cleardevice();//ˢ��
	char instruction[BUF_SIZE] = {};
	char password[15] = {};
	fstream f;

	/*���ÿ�Ϳ��е���Ϣ*/
	setlinecolor(BLACK);
	rectangle(15, 40, 535, 270);//��Ϣ��
	rectangle(15, 310, 450, 330);//�����ı���
	rectangle(465, 15, 535, 35);//���ؿ�
	rectangle(465, 280, 535, 300);//�����
	rectangle(465, 310, 535, 330);//ȷ�Ͽ�
	RECT r1 = { 465,280, 535, 300 };
	settextcolor(BLACK);
	drawtext(_T("����"), &r1, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//����ָ��
	RECT r2 = { 465, 310, 535, 330 };
	settextcolor(BLACK);
	drawtext(_T("ȷ��"), &r2, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//ȷ��ָ��
	RECT r3 = { 465, 15, 535, 35 };
	settextcolor(BLACK);
	drawtext(_T("����"), &r3, DT_CENTER | DT_VCENTER | DT_SINGLELINE);//������һ��

	/*�����ı�*/
	settextstyle(16, 0, "����");//�������ִ�С ��ʽ 
	setbkmode(TRANSPARENT);// ȥ�����ֱ���
	//������������� ����������ʼλ��x ��ʼλ��y ����������ַ�������
	settextcolor(BLACK);
	settextstyle(16, 0, "����");//�������ִ�С ��ʽ 
	outtextxy(15, 15, "��ǰĿ¼:");
	outtextxy(90, 15, ("/cmy"));
	outtextxy(195, 285, "������ָ��");

	/*��ť�趨*/

begin2:
	while (1) {
		m = getmessage(EM_MOUSE | EM_KEY);
		if (m.x >= 465 && m.y >= 280 && m.x <= 535 && m.y <= 300) {//ָ������
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				if (m.message == WM_LBUTTONUP) {//�������ʱ
					InputBox(instruction, BUF_SIZE, "������ָ��");
					clearrectangle(17, 312, 448, 328);
					outtextxy(17, 312, instruction);
				}
			}
		}
		else if (m.x >= 465 && m.y >= 310 && m.x <= 535 && m.y <= 330) {//ȷ��
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				clearrectangle(16, 41, 534, 269);//�����Ϣ��
				char ReplyInfoRecBuffer[BUF_SIZE] = { 0 };//���ܻظ���Ϣ������


				//�����ݿ����������ڴ�
				strcpy_s((char*)InstructionSend, strlen(instruction) + 1, instruction);
				strcpy_s((char*)AccSend, strlen("1") + 1, "1");

				/*����write�Ƿ�ռ�õ��ж�*/
				HANDLE WriteSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "WriteSpace");//���ϵشӻظ���Ϣ�ռ��л�ȡ����
				LPVOID WriteRec = MapViewOfFile(WriteSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
				istringstream InputSS(instruction);
				string WritingFile;
				if (WriteRec)
				{
					WritingFile = string((char*)WriteRec);
				}
				else
				{
					WritingFile = "";
				}

				string InputInstruction;
				string InputFile;
				InputSS >> InputInstruction;
				InputSS >> InputFile;
				if (InputFile == WritingFile && InputInstruction == "write")
				{
					clearrectangle(90, 15, 200, 30);
					outtextxy(90, 15, ReplyInfoRecBuffer);
					outtextxy(16, 41, "��ǰ�ļ����ڱ������û�д�룬���Ժ����ԣ�");
					char NULLChar[] = "";//�������
					strcpy_s((char*)InstructionSend, strlen(NULLChar) + 1, NULLChar);
					goto begin2;
				}

				while (1)
				{
					HANDLE ReplyInfoSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, NULL, "ReplyInfoSpace");//���ϵشӻظ���Ϣ�ռ��л�ȡ����
					LPVOID ReplyInfoRec = MapViewOfFile(ReplyInfoSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);



					if (!ReplyInfoRec)continue;
					if (strlen((char*)ReplyInfoRec))
					{
						MemoryInfoCpy(ReplyInfoRecBuffer, ReplyInfoRec);
						break;//�����ȡ�������ݣ�break
					}
				}

				clearrectangle(17, 42, 533, 268);//��������ı���

				/*������ص���Ϣ����Ϣ��*/
				if (ReplyInfoRecBuffer[0] == '/')
				{
					clearrectangle(90, 15, 200, 30);
					outtextxy(90, 15, ReplyInfoRecBuffer);
					outtextxy(16, 41, "��ת�ɹ���");
					continue;
				}
				int CountChangeLine = 1;
				string InputStr(ReplyInfoRecBuffer);
				for (int i = 0; i < InputStr.length(); i++)
				{
					if (InputStr[i] == '\n')
					{
						InputStr[i] = ' ';
						CountChangeLine++;
					}
				}
				istringstream InputSStr(InputStr);
				for (int i = 0; i < CountChangeLine; i++)
				{
					InputSStr >> InputStr;
					outtextxy(16, 41 + 18 * i, InputStr.c_str());
				}
			}
		}
		else if (m.x >= 465 && m.y >= 15 && m.x <= 535 && m.y <= 35) {//����
			if (m.message == WM_LBUTTONUP) {//�������ʱ
				return;
			}
		}
	}

	/*���ָ��ռ��ļ�ӳ��*/
	UnmapViewOfFile(InstructionSend);
	CloseHandle(InstructionSpace);

	return;
}


void ShareMemoryClear(LPVOID& ShareMemoryPointer)
{
	char NULLChar[] = "";
	strcpy_s((char*)ShareMemoryPointer, strlen(NULLChar) + 1, NULLChar);
}

void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer)
{
	strcpy_s(Buffer, strlen((char*)ShareMemoryPointer) + 1, (char*)ShareMemoryPointer);
	ShareMemoryClear(ShareMemoryPointer);
}