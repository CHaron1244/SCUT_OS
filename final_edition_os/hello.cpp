#include <iostream>
#include <cmath>
#include <cstring>
#include <iostream>
#include <windows.h>
#include <fstream>
#include <sstream>
#include <io.h>
#include <vector>
#include <shared_mutex>
#include <mutex>

#define BUF_SIZE 4096

using namespace std;

string directory = "/root";
// ��д��������ʵ�ֹ�����ͻ���д
std::shared_timed_mutex Mutex;

//i�ڵ���
class INode{
	public:
    char FileName[8];//�ļ�����
    int type;//��ʾ�ļ����ͣ�0ΪĿ¼��1Ϊ�ļ�
    int AdAccess[3];//����Ա�ļ��������ͣ��ֱ��ʾ�ɶ�/��д/��ִ��
    int UserAccess[3];//��ͨ�û��ļ��������ͣ��ֱ��ʾ�ɶ�/��д/��ִ��
    char ParNode[32];//��Ÿ���㣬32��ʾ���Ǹ��ڵ�ĵ�ַλ����ȫ0��ʾû�и��ڵ� 
    char ChildNode[3][32];//����ӽ�㣬�ļ�û���ӽ�㣬ÿ��Ŀ¼�������3���ӽڵ� 
    char DiskAddress[32];//����ļ���Ӧ�����ݣ�Ŀ¼����Ĭ��Ϊ��  ??
    INode()  //�޲γ�ʼ��
    {
		// ����Ŀ¼û������ 
        for (int i = 0; i < 8; i++)
            FileName[i] = 0;
        type = 0; // ��ʼ��ΪĿ¼
        // ����Ա����ͨ�û�������Ȩ��Ϊ���ɶ�����д�Ͳ���ִ�� 
        for (int i = 0; i < 3; i++)
        {
            AdAccess[i] = 0;
            UserAccess[i] = 0;
        }
        // ����Ϊû�и��ڵ��Լ�����Ϊ�� 
        for (int i = 0; i < 32; i++)
        {
            ParNode[i] = 0;
            DiskAddress[i] = 0;
        }
        // ����Ϊû���ӽڵ� 
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 32; j++)
                ChildNode[i][j] = 0;
    }
    INode(char* FN, int t, int* AdAcc, int* UserAcc, char* PNode, char** CNode, char* DAdd)  //�вγ�ʼ��
    {
    	// �����ļ����� 
        for (int i = 0; i < 8; i++)
            FileName[i] = FN[i];
        type = t; //�����ļ�����
		// ����Ȩ�� 
        for (int i = 0; i < 3; i++)
        {
            AdAccess[i] = AdAcc[i];
            UserAccess[i] = UserAcc[i];
        }
        // ���ø��ڵ�ʹ��̿��ַ 
        for (int i = 0; i < 32; i++)
        {
            ParNode[i] = PNode[i];
            DiskAddress[i] = DAdd[i];
        }
        // �����ӽڵ� 
        for (int i = 0; i < 3; i++)
            for (int j = 0; j < 32; j++)
                ChildNode[i][j] = CNode[i][j];
    }
}; 

// ���뺯��
int* Char2EightInt(char Char);//������ת��Ϊ8λ�������������飬���������ı�
char EightInt2Char(int* EightInt);//��8λ��������������ת��Ϊ���ţ���������ı�

int PointerSqe2BlockSqe(int PointerSqe);//��ָ�����ת��Ϊ���̿����
// 32λ����int��ַ<==>���̿��ַ
int* BlockSqe2Add(int BlockSqe);//�����̿����ת��Ϊ32λ��ַ
int Add2BlockSqe(int* Add);//��32λ��ַת��Ϊ���̿����

// ���ܺ��� 
void Init(fstream& Disk_Pointer);//��ʼ�����������ڽ�Ӳ�̽��г�ʼ��
int GetBlockSqe(fstream& Disk_Pointer);//��ȡ��Ӧָ��ĵ�ǰ���̿����
int MatchDocName(fstream& Disk_Pointer, int BlockSqe, string DocName);//����Ӧ���ļ������ض����̿��е������ӽ����ļ������бȽϣ����ƥ�䣬���ش��̿���ţ������ƥ�䣬����0
void JumpPointer(fstream& Disk_Pointer, int BlockSqe, int BlockIn);//��ָ����ת��ָ��λ�á�����BlockSqeΪ���̿���ţ�BlockInΪ���ڵ�ַ
void Reply(string ReplyInfo, LPVOID& ShareMemoryPointer);//���ڿ����ظ�����Ϣ
void ShareMemoryClear(LPVOID& ShareMemoryPointer);//��չ����ڴ�
void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer);//�������ڴ��е����ݿ�������
void BitMapChange(fstream& Disk_Pointer, int BlockSqe, int Tag);//��λͼ�����ĳһ��ֵ��1������0

// ���뺯��
void InputIntArr(fstream& Disk_Pointer, int* IntArr,int Length);//ֱ�ӽ������������뵽Ӳ����
void InputChar(fstream& Disk_Pointer, char Char);//������ת��Ϊ�������������뵽Ӳ����
void InputCharArr(fstream& Disk_Pointer, char* CharArr);//�����Ŵ�ת��Ϊ�������������뵽Ӳ����
void InputDocName(fstream& Disk_Pointer, int BlockSqe, string DocName);//���ļ������뵽Ӳ���У��������ָ��ص�ԭʼ���̿����λ��
void InputParNode(fstream& Disk_Pointer, int BlockSqe, int* IntArr);//����ָ�����뵽Ӳ���У��������ָ��ص�ԭʼ���̿����λ��
void InputAcc(fstream& Disk_Pointer, int BlockSqe, string Acc);//�������������뵽Ӳ����
void InputFileContext(fstream& Disk_Pointer, int BlockSqe, int NewDocBlockSqe, string Context);//�ڶ�Ӧ�Ĵ��̿��������ļ����ݴ��̿��Ӧ�ĵ�ַ,���ڶ�Ӧ���ļ����̿��������ļ�����
void InputFileType(fstream& Disk_Pointer, int BlockSqe, int FileType);//���ļ��������뵽������

// �������
int* Output32Bit(fstream& Disk_Pointer);//���32λ��������
int* Output8Bit(fstream& Disk_Pointer);//���8λ�������� 

// ���Һ���
int FindFreeAreaInINode(fstream& Disk_Pointer);//����Ѱ��λͼ�ж�Ӧ��i��������Ŀ������򣬷���i���Ĵ��̿���ţ��������ָ��ص�ԭʼ���̿����λ��
int FindFreeAreaInDoc(fstream& Disk_Pointer);//����Ѱ��λͼ�ж�Ӧ���ļ�����Ŀ������򣬷����ļ����Ӧ�ĵĴ��̿���ţ��������ָ��ص�ԭʼ���̿����λ
int FindFreeChildNode(fstream& Disk_Pointer, int BlockSqe);//�ж��ض��Ĵ��̿����Ƿ��п��е��ӽ�㣬����У������ӽ����ţ�1/2/3�������û�У�����0����λ�����̿鿪ͷ
char* FindDocName(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳһ32λ��ַ��Ӧ���ļ����̿��Ӧ���ļ����������ļ������飬�������ָ��ص�ԭʼ���̿����λ
int FindDocType(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳ�����̿��Ӧ���ļ����� 
int* FindDocSysAcc(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳ�����̿��Ӧ��ϵͳ��������
int* FindDocComAcc(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳ�����̿��Ӧ����ͨ�û���������
int* FindDocParNodeAdd(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳ��i�ڵ���̿��Ӧ�ĸ�����ַ
int** FindDocChildNodeAdd(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳ�����̿��Ӧ���ӽ���ַ
int* FindDocContextNode(fstream& Disk_Pointer, int BlockSqe);//Ѱ��ĳ�����̿��Ӧ�����ݵĵ�ַ

// ר�ú���
// info
int GetINodeRemain(fstream& Disk_Pointer);//��ȡi����ʣ��ռ�
int GetDocRemain(fstream& Disk_Pointer);//��ȡ�ļ�����ʣ��ռ�

// cd
string cdAddLocation(fstream& Disk_Pointer, string AddBuf);//��ָ����ת����Ӧ·����Ŀ¼��������·�������·������ȷ�����ش�����Ϣ

// dir
void ShowAllChildNodeName(fstream& Disk_Pointer, int BlockSqe, string& ReplyInfo);//����Ѱ��Ŀ����̿�������ӽ������֣�������Ӧ����Ϣ¼�뵽ReplyInfo��
void ShowAllInfo(fstream& Disk_Pointer, int BlockSqe, string& ReplyInfo);//����Ӧ���̿��������Ϣ���ص�ReplyInfo��
int dirAddLocation(fstream& Disk_Pointer, string AddBuf);//��ָ����ת����Ӧ·����Ŀ¼�����·����ȷ�����ض�Ӧ�ļ��Ĵ��̿���ţ����·������ȷ������0

// md
string mdAddLocation(fstream& Disk_Pointer, string AddBuf);//��ָ����ת����Ӧ·����Ŀ¼����������Ҫ�������ļ��������·������ȷ������һ���մ�

// rd 
int IsDirEmpty(fstream& Disk_Pointer, int BlockSqe);//�ж�һ��Ŀ¼���̿��Ƿ�Ϊ��
int rdAddLocation(fstream& Disk_Pointer, string AddBuf);//��ָ����ת����Ӧ·����Ŀ¼��������·�������·������ȷ�����ش�����Ϣ
void rdDir(fstream& Disk_Pointer, int BlockSqe);//ɾ��һ��Ŀ¼���������Ŀ¼��һ��ɾ�� 

// newfile
string FileNameTest(string FileName);//�ж�һ���ļ����ĸ�ʽ�Ƿ���ȷ
string FileAccTest(string Acc);//�жϱ������͵ĸ�ʽ�Ƿ���ȷ
string newfileAddLocation(fstream& Disk_Pointer, string AddBuf);//��ָ����ת����Ӧ·����Ŀ¼����������Ҫ�������ļ��������·������ȷ������һ���մ�

// cat
int catAddLocation(fstream& Disk_Pointer, string AddBuf);//��ָ����ת����Ӧ·����Ŀ¼��������·�������·������ȷ�����ش�����Ϣ
string GetFileContext(fstream& Disk_Pointer, int BlockSqe);//��ȡ��Ӧ���̿������

// copy
string GetWindowsFileName(string path);//��ȡwindows��һ���ļ����ļ���
string CopyNewFile(fstream& Disk_Pointer, istringstream& Buf);//��Copyָ�����½�һ���ļ�

// del
void delDoc(fstream& Disk_Pointer, int BlockSqe);//ɾ��һ���ļ���ָ�����̿飩��

// check
void ConsistencyCheck(fstream& Disk_Pointer, int BlockSqe);//һ���Լ��

// ������ 
int main(){
	fstream Disk_Pointer("..\\disk.txt", ios::in | ios::out); //�ļ�������
    Init(Disk_Pointer);
    JumpPointer(Disk_Pointer, 101, 0);

    /*�ظ���Ϣ�ռ䣬���ڷ�������ͻ��˻ظ���Ϣ*/
    // ����һ���ڴ�ӳ���ļ����ں˶���
    HANDLE ReplyInfoSpace = CreateFileMapping(
        INVALID_HANDLE_VALUE,   // �����ļ����,�̶�д�� 
        NULL,   // Ĭ�ϰ�ȫ����
        PAGE_READWRITE,   // �ɶ���д
        0,   // �ļ�ӳ�����󳤶ȵĸ�32λ
        BUF_SIZE,   // �ļ�ӳ�����󳤶ȵĵ�32λ
        "ReplyInfoSpace"   // �����ڴ�����
    );

	// ���ڴ�ӳ���ļ�ӳ�䵽���̵������ַ��
    LPVOID ReplyInfoSend = MapViewOfFile(
        ReplyInfoSpace,            // �����ڴ�ľ��
        FILE_MAP_ALL_ACCESS, // �ɶ�д���
        0, // �ļ�ӳ����ʼƫ�Ƶĸ�32λ
        0, // �ļ�ӳ����ʼƫ�Ƶĵ�32λ
        BUF_SIZE  // �ļ���Ҫӳ����ֽ�����Ϊ0��ʾӳ�������ļ�ӳ�����
    );

    /*дռ�ÿռ䣬���ڸ��ٿͻ�����ǰд��״̬*/
    HANDLE WriteSpace = CreateFileMapping(
        INVALID_HANDLE_VALUE,   // �����ļ����
        NULL,   // Ĭ�ϰ�ȫ����
        PAGE_READWRITE,   // �ɶ���д
        0,   // �ļ�ӳ�����󳤶ȵĸ�32λ
        BUF_SIZE,   // �ļ�ӳ�����󳤶ȵĵ�32λ
        "WriteSpace"   // �����ڴ�����
    );

    //ӳ�仺������ͼ���õ�ָ�����ڴ��ָ��
    LPVOID WriteSend = MapViewOfFile(
        WriteSpace,            // �����ڴ�ľ��
        FILE_MAP_ALL_ACCESS, // �ɶ�д���
        0, // �ļ�ӳ����ʼƫ�Ƶĸ�32λ
        0, // �ļ�ӳ����ʼƫ�Ƶĵ�32λ
        BUF_SIZE // �ļ���Ҫӳ����ֽ�����Ϊ0��ʾӳ�������ļ�ӳ�����
    );
    char WriteSendBuf[BUF_SIZE] = { 0 }; // д����Ϣ���� 

    /*��rd��ص�����*/
    // ɾ��Ŀ¼���� 
    int rdIsNotEmpty = 0;//01��Ǳ�������ʾrd��Ŀ¼�ǲ��ǿյ�
    int rdOriSqe = 0;
    int rdTarSqe = 0;
    string rdBuf="";//�洢�û����������еĵ�ַ����

    int WriteLock = 0;


    /*��ʼ���ϵؽ�����Ϣ*/
    while (1)
    {
        //Sleep(1000);
        // �򿪹�����ļ�����
        HANDLE InstructionSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "InstructionSpace"); // ���� 
        LPVOID InstructionRec = MapViewOfFile(InstructionSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
        //�ж�ָ���Ƿ�Ϊ�գ������ڴ�δ������ʱ��Ϊ�գ�
		if (!InstructionRec){
            printf("Waiting......\n");
            Sleep(1000);
            continue;
        }

        if (strlen((char*)InstructionRec)){//�жϹ����ڴ����Ƿ�������
            char UserAccBuf[BUF_SIZE] = { 0 };//�û�Ȩ�޻���
            HANDLE AccSpace = OpenFileMapping(FILE_MAP_ALL_ACCESS, false, "AccSpace"); // ����  // ���� 
            LPVOID AccRec = MapViewOfFile(AccSpace, FILE_MAP_ALL_ACCESS, 0, 0, 0);
            MemoryInfoCpy(UserAccBuf, AccRec);

            char InstructionRecBuffer[BUF_SIZE] = { 0 };
            MemoryInfoCpy(InstructionRecBuffer, InstructionRec);// �������ڴ����ݿ�������
            cout << InstructionRecBuffer << endl;
            istringstream Buf(InstructionRecBuffer);//������ת��Ϊ������
            string InstructionBuf;//���ڴ洢���������е�ָ���

            Buf >> InstructionBuf;//�����û������ָ��
            cout << "Get info:" << InstructionRecBuffer << endl;//����ͻ����������Ϣ
            if (rdIsNotEmpty == 1){
                if (InstructionBuf == "y"){
                    rdDir(Disk_Pointer, rdTarSqe);
                    BitMapChange(Disk_Pointer, rdTarSqe, 0);
                    Reply("ɾ�����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSqe, 0);
                    rdIsNotEmpty = 0;
                    rdOriSqe = 0;
                    rdBuf = "";
                    rdTarSqe = 0;
                    continue;
                }
                else if (InstructionBuf == "n"){
                    Reply("����ȡ��", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSqe, 0);
                    rdIsNotEmpty = 0;
                    rdOriSqe = 0;
                    rdBuf = "";
                    rdTarSqe = 0;
                    continue;
                }
                else{
                    Reply("ָ�����",ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSqe, 0);
                    rdIsNotEmpty = 0;
                    rdOriSqe = 0;
                    rdBuf = "";
                    rdTarSqe = 0;
                    continue;
                }
            }
            else if (InstructionBuf == "info"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string ReplyInfo = "";
                string DocSysHost;
                string DocSysSize;
                string DiskBlockSize;
                string SuperBlockSqe;
                string BitMapSqe;
                string RootSqe;
                string INodeSqe;
                string DocSqe;
                DocSysHost = FindDocName(Disk_Pointer, 0); // �ļ�ϵͳ����
                JumpPointer(Disk_Pointer, 0, 64);
                DocSysSize = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // �ļ�ϵͳ��С
                JumpPointer(Disk_Pointer, 0, 96);
                DiskBlockSize = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // ���̿��С
                JumpPointer(Disk_Pointer, 0, 128);
                SuperBlockSqe = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // ���������
                JumpPointer(Disk_Pointer, 0, 160);
                BitMapSqe = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // λͼ���
                JumpPointer(Disk_Pointer, 0, 192);
                RootSqe = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // ��Ŀ¼���
                JumpPointer(Disk_Pointer, 0, 224);
                INodeSqe = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // i��������
                JumpPointer(Disk_Pointer, 0, 256);
                DocSqe = to_string(Add2BlockSqe(Output32Bit(Disk_Pointer))); // �ļ������
                ReplyInfo = ReplyInfo+"�ļ�ϵͳ������:"+DocSysHost + "\n";
                ReplyInfo = ReplyInfo + "Ӳ�̴�С:" + DocSysSize +"MB" + "\n";
                ReplyInfo = ReplyInfo + "���̿��С:" + DiskBlockSize +"B" +"\n";
                ReplyInfo = ReplyInfo + "���������:" + SuperBlockSqe + "\n";
                ReplyInfo = ReplyInfo + "λͼ���:" + BitMapSqe + "\n";
                ReplyInfo = ReplyInfo + "��������:" + RootSqe + "\n";
                ReplyInfo = ReplyInfo + "I��������:" + INodeSqe + "\n";
                ReplyInfo = ReplyInfo + "I�����ʣ��ռ�:" + to_string(GetINodeRemain(Disk_Pointer)) + "B\n";
                ReplyInfo = ReplyInfo + "�ļ������:" + DocSqe + "\n";
                ReplyInfo = ReplyInfo + "�ļ���ʣ��ռ�:" + to_string(GetDocRemain(Disk_Pointer)) + "B";
                JumpPointer(Disk_Pointer, OriSqe, 0);
                Reply(ReplyInfo, ReplyInfoSend);
                continue;
            }
            else if (InstructionBuf == "cd"){
                string cdBuf;//�û��洢���������еĵ�ַ����
                Buf >> cdBuf;
                string ReplyInfo= cdAddLocation(Disk_Pointer, cdBuf);
                Reply(ReplyInfo, ReplyInfoSend);
            }
            else if (InstructionBuf == "dir"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string ReplyInfo="";
                string dirBuf;//�û��洢���������еĵ�ַ����
                Buf >> dirBuf;
                if (dirBuf == "/s"){//�����/s����
                    ReplyInfo += "��ǰĿ¼�µ������ļ�����Ŀ¼Ϊ��";
                    ShowAllChildNodeName(Disk_Pointer, GetBlockSqe(Disk_Pointer), ReplyInfo);
                    Reply(ReplyInfo, ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                int CurBlockSqe=dirAddLocation(Disk_Pointer, dirBuf);//�ļ�ָ�붨λ
                if (CurBlockSqe == 0){//������ش�����Ϣ
                    Reply("��ַ���󣬴���ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }
                ReplyInfo += "��ǰĿ¼�µ���ϢΪ��";
                ShowAllInfo(Disk_Pointer, CurBlockSqe, ReplyInfo);
                Reply(ReplyInfo, ReplyInfoSend);
                continue;

                JumpPointer(Disk_Pointer, OriSqe, 0);
            }
            else if (InstructionBuf == "md"){//������Ŀ¼
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string mdBuf;//�û��洢���������еĵ�ַ����
                Buf >> mdBuf;
                
                // �ҵ�Ҫ������Ŀ¼���֣�����ָ���ƶ�����Ŀ¼�ĸ�Ŀ¼��
                string CurDirName=mdAddLocation(Disk_Pointer, mdBuf);
                if (CurDirName == ""){
                    Reply("��ַ���󣬴���ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�ж��½�Ŀ¼�ĳ���*/
                if (CurDirName.length() > 8){//Ŀ¼�����ܴ���8λ
                    Reply("Ŀ¼������������ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                int NewINodeBlockSqe = FindFreeAreaInINode(Disk_Pointer);

                /*�ж��ļ�ϵͳ�Ƿ��пռ�*/
                if (!NewINodeBlockSqe){//����ļ�ϵͳ����û�пռ䣬����ʧ��
                    Reply("�ļ�ϵͳ���޿��пռ䣬����ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�ж��������ļ����Ƿ����ӽ�����ļ����ظ�*/
                if (MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName)){
                    Reply("�ļ����ظ�������ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�ж��ӽ���ǲ�������*/
                int FreeChildSqe = FindFreeChildNode(Disk_Pointer, GetBlockSqe(Disk_Pointer));
                if (FreeChildSqe){//û�������ӽ����Ϊ�Ѿ�ʹ�ã�������32λ��ַ
                    JumpPointer(Disk_Pointer, GetBlockSqe(Disk_Pointer), 112 + 32 * (FreeChildSqe - 1));//��ת����Ӧ���ӽ��
                    int* INode = BlockSqe2Add(NewINodeBlockSqe);
                    INode[0] = 1;//����λ��Ϊ1����ʾ���ӽ���Ѿ���ʹ��
                    InputIntArr(Disk_Pointer, INode,32);//����32λ�ӽ��
                    JumpPointer(Disk_Pointer, GetBlockSqe(Disk_Pointer), 0);//���ؿ�ͷ
                }
                else{
                    Reply("��ǰ�ļ������ļ��Ѵ����ޣ�����ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�����ļ����г�ʼ��*/
                InputDocName(Disk_Pointer, NewINodeBlockSqe, CurDirName);
                InputParNode(Disk_Pointer, NewINodeBlockSqe, BlockSqe2Add(GetBlockSqe(Disk_Pointer)));
                InputAcc(Disk_Pointer, NewINodeBlockSqe, "000001");
                // ��ʼ��I�ڵ���ӽڵ㲿��
                JumpPointer(Disk_Pointer, NewINodeBlockSqe, 112);
                for(int i = 0; i < 3 * 32; i++){
                    Disk_Pointer << "0";
                }
                BitMapChange(Disk_Pointer, NewINodeBlockSqe, 1);
                Reply("Ŀ¼�����ɹ���", ReplyInfoSend);
                JumpPointer(Disk_Pointer, OriSqe, 0);
                continue;
            }
            else if (InstructionBuf == "rd"){
                rdOriSqe = GetBlockSqe(Disk_Pointer);
                Buf >> rdBuf;
                rdTarSqe=rdAddLocation(Disk_Pointer, rdBuf);//Ѱ��Ӧ��ɾ����Ŀ����̿�
                if (rdTarSqe == 1){//����Ǹ�Ŀ¼
                    Reply("��Ŀ¼����ɾ����ɾ��ʧ�ܣ�", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSqe, 0);
                    continue;
                }
                if (FindDocType(Disk_Pointer, rdTarSqe) != 0){//���Ŀ���ļ�����Ŀ¼
                    Reply("Ŀ���ļ�Ϊ��ͨ�ļ���ɾ��ʧ�ܣ�", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSqe, 0);
                    continue;
                }
                if (rdTarSqe){
                    if (!IsDirEmpty(Disk_Pointer, rdTarSqe)){//����ǿ�
                        rdIsNotEmpty = 1;
                        Reply("Ŀ��Ŀ¼�ǿգ��Ƿ���Ҫɾ����y/n", ReplyInfoSend);
                        continue;
                    }
                    else{//����ǿյ�
                        rdDir(Disk_Pointer, rdTarSqe);//ɾ����Ӧ���
                        BitMapChange(Disk_Pointer, rdTarSqe, 0);
                        Reply("Ŀ��Ŀ¼ɾ�����", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, rdOriSqe, 0);
                        rdIsNotEmpty = 0;//������ϵ����0
                        rdOriSqe = 0;
                        rdBuf = "";
                        rdTarSqe = 0;
                        continue;
                    }
                }
                else{//���û���Ҷ�Ŀ����̿飬���ش�����Ϣ
                    Reply("��ַ����",ReplyInfoSend);
                    JumpPointer(Disk_Pointer, rdOriSqe, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, rdOriSqe, 0);
            }
            else if (InstructionBuf == "newfile"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string newfileNameBuf;//�û��洢���������еĵ�ַ����
                string newfileAccBuf;
                string newfileContextBuf;
                Buf >> newfileNameBuf;
                Buf >> newfileAccBuf;
                Buf >> newfileContextBuf;
                

                string CurDirName = newfileAddLocation(Disk_Pointer, newfileNameBuf);
                /*�ļ�����ʽ���*/
                string ReplyInfo = FileNameTest(CurDirName);
                if (ReplyInfo != ""){
                    Reply(ReplyInfo, ReplyInfoSend);
                    continue;
                }

                /*�������͸�ʽ���*/
                ReplyInfo = FileAccTest(newfileAccBuf);
                if (ReplyInfo != ""){
                    Reply(ReplyInfo, ReplyInfoSend);
                    continue;
                }

                int NewINodeBlockSqe = FindFreeAreaInINode(Disk_Pointer);
                int NewDocBlockSqe = FindFreeAreaInDoc(Disk_Pointer);

                /*�ж��ļ�ϵͳ�Ƿ��пռ�*/
                if (!NewINodeBlockSqe){//����ļ�ϵͳ����û�пռ䣬����ʧ��
                    Reply("�ļ�ϵͳ���޿��пռ䣬����ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }
                if (!NewDocBlockSqe){//����ļ�ϵͳ����û�пռ䣬����ʧ��
                    Reply("�ļ�ϵͳ���޿��пռ䣬����ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�ж��������ļ����Ƿ����ӽ�����ļ����ظ�*/
                if (MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName)){
                    Reply("�ļ����ظ�������ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�ж��ӽ���ǲ�������*/
                int FreeChildSqe = FindFreeChildNode(Disk_Pointer, GetBlockSqe(Disk_Pointer));
                if (FreeChildSqe){//û�������ӽ����Ϊ�Ѿ�ʹ�ã�������32λ��ַ
                    JumpPointer(Disk_Pointer, GetBlockSqe(Disk_Pointer), 112 + 32 * (FreeChildSqe - 1));//��ת����Ӧ���ӽ��
                    int* INode = BlockSqe2Add(NewINodeBlockSqe);
                    INode[0] = 1;//����λ��Ϊ1����ʾ���ӽ���Ѿ���ʹ��
                    InputIntArr(Disk_Pointer, INode, 32);//����32λ�ӽ��
                    JumpPointer(Disk_Pointer, GetBlockSqe(Disk_Pointer), 0);//���ؿ�ͷ
                }
                else{
                    Reply("��ǰ�ļ������ļ��Ѵ����ޣ�����ʧ�ܣ�", ReplyInfoSend);
                    continue;
                }

                /*�����ļ����г�ʼ��*/
                InputDocName(Disk_Pointer, NewINodeBlockSqe, CurDirName);//�����ļ���
                InputParNode(Disk_Pointer, NewINodeBlockSqe, BlockSqe2Add(GetBlockSqe(Disk_Pointer)));//���븸����ַ
                InputAcc(Disk_Pointer, NewINodeBlockSqe, newfileAccBuf);//�����ļ���������
                InputFileContext(Disk_Pointer, NewINodeBlockSqe, NewDocBlockSqe,newfileContextBuf);//�����ļ�����
                InputFileType(Disk_Pointer, NewINodeBlockSqe, 1);//�����ļ�����
                BitMapChange(Disk_Pointer, NewINodeBlockSqe, 1);
                BitMapChange(Disk_Pointer, NewDocBlockSqe, 1);
                Reply("�ļ������ɹ���", ReplyInfoSend);
                JumpPointer(Disk_Pointer, OriSqe, 0);
                continue;
            }
            else if (InstructionBuf == "cat"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string mdBuf;//�û��洢���������еĵ�ַ����
                Buf >> mdBuf;
                int TarSqe=catAddLocation(Disk_Pointer, mdBuf);
                if (FindDocType(Disk_Pointer, TarSqe) != 1){//���Ŀ���ļ�������ͨ�ļ�
                    Reply("Ŀ���ļ�������ͨ�ļ���ָ�����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                if (TarSqe){
                    int* ComAcc = FindDocComAcc(Disk_Pointer, TarSqe);//��ѯ��ͨ�û�����Ȩ��
                    int* SysAcc = FindDocSysAcc(Disk_Pointer, TarSqe);//��ѯϵͳ�û�����Ȩ��
                    if (!((ComAcc[0] == 1 && UserAccBuf[0] == '1') || (SysAcc[0] == 1 && UserAccBuf[0] == '0'))){//�ж�����Ȩ��д����ļ�
                        Reply("��Ȩ�޶�ȡ���ļ���", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }
                    Reply(GetFileContext(Disk_Pointer, Add2BlockSqe(FindDocContextNode(Disk_Pointer, TarSqe))), ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                else{
                    Reply("��ַ����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, OriSqe, 0);
            }
            else if (InstructionBuf == "write"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string writeBuf;//�û��洢���������еĵ�ַ����
                string InputInfo;//��������ļ�����
                Buf >> writeBuf;
                Buf >> InputInfo;
                int TarSqe=catAddLocation(Disk_Pointer, writeBuf);
                if (FindDocType(Disk_Pointer, TarSqe) != 1){//���Ŀ���ļ�������ͨ�ļ�
                    Reply("Ŀ���ļ�������ͨ�ļ���ָ�����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                if (TarSqe){
                    int* ComAcc = FindDocComAcc(Disk_Pointer, TarSqe);//��ѯ��ͨ�û�����Ȩ��
                    int* SysAcc = FindDocSysAcc(Disk_Pointer, TarSqe);//��ѯϵͳ�û�����Ȩ��
                    if (InputInfo.length() > 128){
                        Reply("�ļ����ݹ��࣬�޷�д�룡", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }
                    if (!((ComAcc[1] == 1 && UserAccBuf[0] == '1') || (SysAcc[1] == 1 && UserAccBuf[0] == '0'))){//�ж�����Ȩ��д����ļ�
                        Reply("��Ȩ��д����ļ���", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }
                    /*���Դ�ļ�*/
                    JumpPointer(Disk_Pointer, Add2BlockSqe(FindDocContextNode(Disk_Pointer, TarSqe)), 0);
                    for (int i = 0; i < 1024; i++)
                        Disk_Pointer << 0;
                    JumpPointer(Disk_Pointer, Add2BlockSqe(FindDocContextNode(Disk_Pointer, TarSqe)), 0);
                    
                    /*дʱ�ӳ�*/
                    strcpy_s((char*)WriteSend, writeBuf.length() + 1, const_cast<char*>(writeBuf.c_str()));
                    Sleep(10000);
                    ShareMemoryClear(WriteSend);

                    /*д��*/
                    InputCharArr(Disk_Pointer, const_cast<char*>(InputInfo.c_str()));
                    Reply("д��ɹ���", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                else{
                    Reply("��ַ����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                JumpPointer(Disk_Pointer, OriSqe, 0);
            }
            else if (InstructionBuf == "copy"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string OriAdd;
                string TarAdd;
                string Parameter;

                Buf >> OriAdd;
                Buf >> TarAdd;
                Buf >> Parameter;

                if (Parameter == ""){//���û�в���
                
                    /*Ѱ��Դ�ļ�����*/
                    int OriAddSqe = catAddLocation(Disk_Pointer, OriAdd);
                    string OriContext;
                    if (OriAddSqe)
                        OriContext = GetFileContext(Disk_Pointer, Add2BlockSqe(FindDocContextNode(Disk_Pointer, OriAddSqe)));
                    else{
                        Reply("Դ��ַ����", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }

                    int TarAddSqe = catAddLocation(Disk_Pointer, TarAdd);
                    if (TarAddSqe){
                        int* TarAddDocAdd = FindDocContextNode(Disk_Pointer, TarAddSqe);
                        JumpPointer(Disk_Pointer, Add2BlockSqe(TarAddDocAdd), 0);
                        for (int i = 0; i < 1024; i++)
                            Disk_Pointer << 0;
                        JumpPointer(Disk_Pointer, Add2BlockSqe(TarAddDocAdd), 0);
                        InputCharArr(Disk_Pointer, const_cast<char*>(OriContext.c_str()));
                        Reply("���Ƴɹ���", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }
                    else{
                        Reply("Ŀ���ַ����", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }
                }
                else if(Parameter=="host"){
                    string TempOriContext;
                    string OriContext;
                    string OriFileName=GetWindowsFileName(OriAdd);
                    fstream Disk_Pointer2(OriAdd, ios::in | ios::out); // ����

                    /*Ѱ��Դ�ļ�����*/
                    if (Disk_Pointer2){
                        while (!Disk_Pointer2.eof()){
                            Disk_Pointer2 >> TempOriContext;
                            OriContext += TempOriContext;
                        }
                        //GetWindowsFileName(OriAdd,OriFileName)
                    }
                    else{
                        Reply("Դ��ַ����", ReplyInfoSend);
                        JumpPointer(Disk_Pointer, OriSqe, 0);
                        continue;
                    }
                    Disk_Pointer2.close();

                    /*������Ŀ���ļ���*/
                    string InputInstruciton = "";
                    InputInstruciton += OriFileName;
                    InputInstruciton += " 111111 ";
                    InputInstruciton += OriContext;
                    istringstream Inputsstr(InputInstruciton);
                    Reply(CopyNewFile(Disk_Pointer, Inputsstr), ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                else{
                    Reply("��������", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }

                JumpPointer(Disk_Pointer, OriSqe, 0);
            }
            else if (InstructionBuf == "del"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                string delBuf;//�û��洢���������еĵ�ַ����
                Buf >> delBuf;
                int TarSqe=catAddLocation(Disk_Pointer, delBuf);
                if (FindDocType(Disk_Pointer, TarSqe) != 1){//���Ŀ���ļ�������ͨ�ļ�
                    Reply("Ŀ���ļ�������ͨ�ļ���ָ�����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                if (TarSqe){
                    delDoc(Disk_Pointer, TarSqe);
                    Reply("ɾ���ɹ���", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
                else{
                    Reply("��ַ����", ReplyInfoSend);
                    JumpPointer(Disk_Pointer, OriSqe, 0);
                    continue;
                }
            }
            else if (InstructionBuf == "check"){
                int OriSqe = GetBlockSqe(Disk_Pointer);
                ConsistencyCheck(Disk_Pointer,101);
                JumpPointer(Disk_Pointer, OriSqe, 0);
                Reply("�ļ�һ���Լ����ɣ�", ReplyInfoSend);
                continue;
            }
            else{
                Reply("ָ�����", ReplyInfoSend);
                continue;
            }
        }
        else{
            printf("Waiting......\n");
            Sleep(1000);
        }
    }
    return 0;

    /*����ظ���Ϣ�ռ��ļ�ӳ��*/
    UnmapViewOfFile(ReplyInfoSend);
    CloseHandle(ReplyInfoSpace);
}

// ���뺯��
int* Char2EightInt(char Char){
    int IntNum = int(Char);//���ַ�ת��ΪASCII��
    int* BiInt = new int[8];//�������
    for (int i = 0; i < 8; i++){//�ó�nȡ�෨ת��Ϊ��������
        BiInt[7 - i] = IntNum % 2;
        IntNum = IntNum / 2;
    }
    return BiInt;
} 

char EightInt2Char(int* EightInt){
    int IntNum = 0;
    for (int i = 7; i >= 0; i--)//���������������ۼ�
        IntNum += EightInt[i] * pow(2, 7 - i);
    return char(IntNum);//����ASCII��ֵ
}

int PointerSqe2BlockSqe(int PointerSqe){
    return PointerSqe / 1024;
}

int* BlockSqe2Add(int BlockSqe){
    int* Add;
    Add = new int[32];
    for (int i = 0; i < 32; i++)
        Add[i] = 0;
    for (int i = 21; i >= 5; i--){//�ó�nȡ�෨ת��Ϊ��������
        Add[i] = BlockSqe % 2;
        BlockSqe = BlockSqe / 2;
    }
    return Add;
}

int Add2BlockSqe(int* Add){
    int BlockSqe = 0;
    for (int i = 21; i >= 5; i--)//��17λ������������Ϊ���̿���� 
        BlockSqe += Add[i] * pow(2, 21 - i);
    return BlockSqe;
}

//���ܺ��� 
void Init(fstream& Disk_Pointer){
    /*��ʼ����Ŀ¼*/
    Disk_Pointer.seekg(1024 * 101, ios::beg);//���ļ������� Disk_Pointer �е��ļ�ָ���ƶ����ļ��Ŀ�ͷλ����ƫ��101KB��λ�ô�������һ��i�ڵ㴦 
    char RootDirName[] = "root0000";//��Ŀ¼��
    InputCharArr(Disk_Pointer, RootDirName);
    int FileType[] = { 0,0,0,0,0,0,0,0 };//��ʾ�ļ�����ΪĿ¼
    InputIntArr(Disk_Pointer, FileType,8);
    int Access[] = { 0,0,0,0,0,0,0,0 };//Ŀ¼�����۷���Ȩ�����⣬ȫ����Ϊ0
    InputIntArr(Disk_Pointer, Access,8);
    int ParNode[] = { 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
    //��Ŀ¼û�и����
    InputIntArr(Disk_Pointer, ParNode,32);
    int ChildNode[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 , 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
    //��Ŀ¼��ʼʱû���ӽ��
    InputIntArr(Disk_Pointer, ChildNode,96);
    //Ŀ¼û���ļ�����
    int DiskAddress[] = { 0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 ,0,0,0,0,0,0,0,0 };
    InputIntArr(Disk_Pointer, DiskAddress,32);

    /*��ʼ����������*/
    JumpPointer(Disk_Pointer, 0, 0);
    InputCharArr(Disk_Pointer, const_cast<char*>("tyd")); // �ļ�ϵͳ���� 
    JumpPointer(Disk_Pointer, 0, 64);
    InputIntArr(Disk_Pointer, BlockSqe2Add(100), 32); // λͼ��С����100�� 
    JumpPointer(Disk_Pointer, 0, 96);
    InputIntArr(Disk_Pointer, BlockSqe2Add(1024), 32); // ���̿��С 
    JumpPointer(Disk_Pointer, 0, 128); // ���� 
    InputIntArr(Disk_Pointer, BlockSqe2Add(0), 32); // ��������� 
    JumpPointer(Disk_Pointer, 0, 160);
    InputIntArr(Disk_Pointer, BlockSqe2Add(1), 32); // λͼ��� 
    JumpPointer(Disk_Pointer, 0, 192);
    InputIntArr(Disk_Pointer, BlockSqe2Add(101), 32); // ��Ŀ¼��� 
    JumpPointer(Disk_Pointer, 0, 224);
    InputIntArr(Disk_Pointer, BlockSqe2Add(102), 32); // i�ڵ������ 
    JumpPointer(Disk_Pointer, 0, 256);
    InputIntArr(Disk_Pointer, BlockSqe2Add(50102), 32); // �ļ������ 

    /*��ʼ��λͼ��*/
    JumpPointer(Disk_Pointer, 1, 0);//��ת��λͼ��
    for (int i = 0; i < 102; i++)//��ǰ101�����̿���Ϊ��ʹ��
        Disk_Pointer << 1;
    for (int i = 0; i < 1024 * 100 - 102; i++)//����������д��̿���Ϊδʹ��
        Disk_Pointer << 0;
}

int GetBlockSqe(fstream& Disk_Pointer){
    int PointerSqe = Disk_Pointer.tellg();//tellg��ȡ�ļ�������ǰ��ȡλ��,����һ����ʾ�ļ�ָ��λ�õ�����ֵ
    return PointerSqe2BlockSqe(PointerSqe); //PointerSqe / 1024
}

int MatchDocName(fstream& Disk_Pointer, int BlockSqe, string DocName){
    int OriSqe = GetBlockSqe(Disk_Pointer);//ָ�뵱ǰ���̿����
    for (int i = 0; i < 3;i++){//ѭ��3���ж��ӽ���ļ���
        JumpPointer(Disk_Pointer, BlockSqe, 112+32*i);//��ת���ӽ�㲿��
        int* BitDocName = Output32Bit(Disk_Pointer);//����32λ�ӽ���ַ
        char* CharDocName = FindDocName(Disk_Pointer, Add2BlockSqe(BitDocName));//Ѱ���ӽ���ļ���
        if (string(CharDocName) == DocName){//�ж��ӽ���ļ����ǲ���Ҫ�ҵĶ���
            JumpPointer(Disk_Pointer, OriSqe, 0);
            return Add2BlockSqe(BitDocName);//����ǣ������ӽ���Ӧ�Ĵ��̿��
        }
    }
    JumpPointer(Disk_Pointer, OriSqe,0);
    return 0;//������ǣ�����0
}

void JumpPointer(fstream& Disk_Pointer, int BlockSqe, int BlockIn){
    Disk_Pointer.seekg(1024 * BlockSqe + BlockIn, ios::beg);//seekg�������������ļ�ָ���λ��,ios::beg������ʾ������ļ��Ŀ�ͷ����ƫ��
}

void Reply(string ReplyInfo, LPVOID& ShareMemoryPointer){
    std::unique_lock<std::shared_timed_mutex> lock(Mutex);
    strcpy_s((char*)ShareMemoryPointer, strlen(ReplyInfo.c_str()) + 1, ReplyInfo.c_str());//���ʹ�����Ϣ
    cout << "Return Info:" << ReplyInfo << endl;
}

// ��������ڴ� 
void ShareMemoryClear(LPVOID& ShareMemoryPointer){
    char NULLChar[] = "";
    strcpy_s((char*)ShareMemoryPointer, strlen(NULLChar) + 1, NULLChar);
}

void MemoryInfoCpy(char* Buffer, LPVOID& ShareMemoryPointer){
    strcpy_s(Buffer, strlen((char*)ShareMemoryPointer) + 1, (char*)ShareMemoryPointer);
    ShareMemoryClear(ShareMemoryPointer);
}

void BitMapChange(fstream& Disk_Pointer, int BlockSqe, int Tag){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, 1, BlockSqe);
    Disk_Pointer << Tag;
    JumpPointer(Disk_Pointer, OriSqe, 0); // ���� 
}

// ���뺯��
void InputCharArr(fstream& Disk_Pointer, char* CharArr){//��ÿ���ַ����ݵ�8λ�����Ʊ�ʾд�뵱ǰλ��(д������ַ�����)
    for (int i = 0; i < strlen(CharArr); i++)
        InputChar(Disk_Pointer, CharArr[i]);
    return;
} 

void InputChar(fstream& Disk_Pointer, char Char){//��Char����ַ���8λ�����Ʊ�ʾд�뵱ǰλ��(д������ַ�)
    int* InputInt = new int[8];
    InputInt = Char2EightInt(Char);
    InputIntArr(Disk_Pointer, InputInt,8);
}

void InputIntArr(fstream& Disk_Pointer, int* IntArr, int Length){
    for (int i = 0; i < Length; i++)
        Disk_Pointer << IntArr[i];//��ǰλ��д�������е�����(0 or 1)
    return;
}

void InputDocName(fstream& Disk_Pointer,int BlockSqe,string DocName){//���ļ���
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSqe, 0);
    int DocNameLength = DocName.length();//��ȡ�ļ����ĳ���
    char* CharDocName = new char[8];
    for(int i = 0; i < 8; i++) CharDocName[i] = '0';
    InputCharArr(Disk_Pointer, CharDocName);
    JumpPointer(Disk_Pointer, BlockSqe, 0);
    InputCharArr(Disk_Pointer, const_cast<char*>(DocName.c_str()));
    JumpPointer(Disk_Pointer, OriSqe, 0);//ָ���ƻ�ԭʼ���̿鿪ͷ
}

void InputParNode(fstream& Disk_Pointer, int BlockSqe, int* IntArr){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSqe, 80);
    InputIntArr(Disk_Pointer, IntArr,32);
    JumpPointer(Disk_Pointer, OriSqe, 0);
}

void InputAcc(fstream& Disk_Pointer, int BlockSqe, string Acc){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSqe, 72);
    int* TempInputIntArr = new int[8];
    for (int i = 0; i < 8; i++)
        TempInputIntArr[i] = 0;
    for (int i = 1; i < 4; i++)
        TempInputIntArr[i] = Acc[i - 1] - 48;
    for (int i = 5; i < 8; i++)
        TempInputIntArr[i] = Acc[i - 2] - 48;
    InputIntArr(Disk_Pointer, TempInputIntArr, 8);
    JumpPointer(Disk_Pointer, OriSqe, 0);
}

void InputFileContext(fstream& Disk_Pointer, int BlockSqe, int NewDocBlockSqe, string Context){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSqe, 208);
    InputIntArr(Disk_Pointer, BlockSqe2Add(NewDocBlockSqe), 32);
    JumpPointer(Disk_Pointer, NewDocBlockSqe, 0);
    char* arr = new char[128];
    for(int i = 0; i < 128; i++)
        arr[i] = '0';
    InputCharArr(Disk_Pointer, arr);
    JumpPointer(Disk_Pointer, NewDocBlockSqe, 0);
    InputCharArr(Disk_Pointer, const_cast<char*>(Context.c_str()));
    JumpPointer(Disk_Pointer, OriSqe, 0);
}

void InputFileType(fstream& Disk_Pointer, int BlockSqe, int FileType){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSqe, 71);
    Disk_Pointer << FileType;
    JumpPointer(Disk_Pointer, OriSqe, 0);
}

// ������� 
int* Output32Bit(fstream& Disk_Pointer){
    int* Output32Bit = new int[32];
    char TempChar;//'0' or '1'
    for (int i = 0; i < 32; i++){
        Disk_Pointer >> TempChar;
        Output32Bit[i] = int(TempChar) - 48;//0 or 1
    }
    return Output32Bit;
}

int* Output8Bit(fstream& Disk_Pointer){
    int* Output8Bit = new int[8];
    char TempChar;
    for (int i = 0; i < 8; i++){
        Disk_Pointer >> TempChar;
        Output8Bit[i] = int(TempChar) - 48;
    }
    return Output8Bit;
}

// ���Һ���
int FindFreeAreaInINode(fstream& Disk_Pointer){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    int i = 102;
    JumpPointer(Disk_Pointer, 1, i);
    char TempChar;
    for (i; i < 50102; i++){//����λͼ����i���Ĳ���
        Disk_Pointer >> TempChar;
        if (TempChar == '0'){
            JumpPointer(Disk_Pointer, OriSqe, 0);
            return i;
        }
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return 0;//���û�ҵ�������0
}

int FindFreeAreaInDoc(fstream& Disk_Pointer){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, 1, 50102);//����λͼ�����ļ��Ĳ���
    int i = 50102;
    char TempChar;
    for (i; i < 100102; i++){
        Disk_Pointer >> TempChar;
        if (TempChar == '0'){
            JumpPointer(Disk_Pointer, OriSqe, 0);
            return i;
        }
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return 0;//���û�ҵ�������0
}

int FindFreeChildNode(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char IsFree;

    JumpPointer(Disk_Pointer, BlockSqe, 112);//��ת����һ���ӽ���λ��
    Disk_Pointer >> IsFree;
    if (IsFree == '0'){//����ӽ��û�б�ʹ��
        Disk_Pointer.seekg(-1, ios::cur);//��һλ
        JumpPointer(Disk_Pointer, OriSqe, 0);
        return 1;//�����ӽ�����
    }

    JumpPointer(Disk_Pointer, BlockSqe, 112 + 32);//��ת���ڶ����ӽ���λ��
    Disk_Pointer >> IsFree;
    if (IsFree == '0'){//����ӽ��û�б�ʹ��
        Disk_Pointer.seekg(-1, ios::cur);//��һλ
        JumpPointer(Disk_Pointer, OriSqe, 0);
        return 2;//�����ӽ�����
    }

    JumpPointer(Disk_Pointer, BlockSqe, 112 + 32 + 32);//��ת���������ӽ���λ��
    Disk_Pointer >> IsFree;
    if (IsFree == '0'){//����ӽ��û�б�ʹ��
        Disk_Pointer.seekg(-1, ios::cur);//��һλ
        JumpPointer(Disk_Pointer, OriSqe, 0);
        return 3;//�����ӽ�����
    }

    JumpPointer(Disk_Pointer, OriSqe, 0);
    return 0;
}

char* FindDocName(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char* DocName;//Ӧ�÷��ص��ļ���
    int num = 0; //�ļ����ĳ���
    JumpPointer(Disk_Pointer, BlockSqe, 0);
    while(EightInt2Char(Output8Bit(Disk_Pointer)) != '0') num++;
    DocName = new char[num];
    int* TempIntArr;//����װ�����ڷ��������
    JumpPointer(Disk_Pointer, BlockSqe, 0);//��ת����Ӧλ��
    for (int i = 0; i < num; i++){
        TempIntArr = Output8Bit(Disk_Pointer);
        DocName[i] = EightInt2Char(TempIntArr);//����Ӧ�Ķ���������ת��Ϊ����
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return DocName;//�����ļ���
}

// Ŀ¼����0�� �ļ�����1
int FindDocType(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    JumpPointer(Disk_Pointer, BlockSqe, 71);
    Disk_Pointer >> TempChar;
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return int(TempChar)-48;
} 

int* FindDocSysAcc(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    int* SysAcc;
    SysAcc = new int[3];
    JumpPointer(Disk_Pointer, BlockSqe, 73); 
    for (int i = 0; i < 3; i++){
        Disk_Pointer >> TempChar;
        SysAcc[i] = TempChar - 48;
        cout << SysAcc[i];
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return SysAcc;
}

int* FindDocComAcc(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    int* ComAcc;
    ComAcc = new int[3];
    JumpPointer(Disk_Pointer, BlockSqe, 77); // ����
    for (int i = 0; i < 3; i++){
        Disk_Pointer >> TempChar;
        ComAcc[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return ComAcc;
}

// ����i�ڵ��Ӧ�ĸ��ڵ�ĵ�ַ 
int* FindDocParNodeAdd(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    int* ParNodeAdd;
    ParNodeAdd = new int[32];
    JumpPointer(Disk_Pointer, BlockSqe, 80);
    for (int i = 0; i < 32; i++){
        Disk_Pointer >> TempChar;
        ParNodeAdd[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return ParNodeAdd;
}

// ����i�ڵ��Ӧ���ӽڵ�ĵ�ַ 
int** FindDocChildNodeAdd(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    int** ChildNodeAdd;
    ChildNodeAdd = new int* [3];
    for (int i = 0; i < 3; i++)
        ChildNodeAdd[i] = new int[32];
    JumpPointer(Disk_Pointer, BlockSqe, 112);
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 32; j++){
            Disk_Pointer >> TempChar;
            ChildNodeAdd[i][j] = TempChar - 48;
        }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return ChildNodeAdd;
}

// ��i�ڵ��Ӧ���ļ����ݵ�ַ 
int* FindDocContextNode(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    int* ParNodeAdd;
    ParNodeAdd = new int[32];
    JumpPointer(Disk_Pointer, BlockSqe, 208);
    for (int i = 0; i < 32; i++){
        Disk_Pointer >> TempChar;
        ParNodeAdd[i] = TempChar - 48;
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return ParNodeAdd;
}

// ר�ú���
// info
int GetINodeRemain(fstream& Disk_Pointer){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    int CountRemain = 0;
    JumpPointer(Disk_Pointer, 1, 102);
    char TempChar;
    for (int i = 102; i < 50102; i++){
        Disk_Pointer >> TempChar;
        if (TempChar == '0')
            CountRemain++;
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return CountRemain;
}

int GetDocRemain(fstream& Disk_Pointer){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    int CountRemain = 0;
    JumpPointer(Disk_Pointer, 1, 50102);
    char TempChar;
    for (int i = 50102; i < 102400; i++){
        Disk_Pointer >> TempChar;
        if (TempChar == '0')
            CountRemain++;
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return CountRemain;
}

// cd
string cdAddLocation(fstream& Disk_Pointer, string AddBuf){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    string OriAddBuf=AddBuf;
    string CurDirName;//������ʱ�洢��ǰλ��
    if (AddBuf.length() == 0)//�����ַΪ�գ����ش�����Ϣ
        return "��ת��ַȱʧ��";

    if (AddBuf[0] == '/'){
        JumpPointer(Disk_Pointer, 101, 0);//�Ӹ���㿪ʼ����
        /*����ָ��ķָ���/����Ԥ����*/
        for (int i = 0; i < AddBuf.length(); i++)//�ȴ����ַ�е�/����
            if (AddBuf[i] == '/')
                AddBuf[i] = ' ';//��/�滻Ϊ�ո񣬱�������
        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*�жϸ���ַ�Ƿ�������*/
        if (CurDirName != "root"){//�������ַ����
            JumpPointer(Disk_Pointer, OriSqe, 0);//��ת��ԭ����λ��
            return "��ַ����";
        }
        if (AddStr.eof()){
            JumpPointer(Disk_Pointer, 101, 0);
            directory = "/root";
            return "/root";
        }
        /*ͨ���ӽ�㲻�Ͻ��ж�λ*/
        while (1){//�������ַ���AddressBuf�л�������
            AddStr >> CurDirName;//������һ��Ŀ¼
            int ChildBlockSqe = MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName);
            if (ChildBlockSqe){//����ҵ��˶�Ӧ�����ļ�
                if (FindDocType(Disk_Pointer, GetBlockSqe(Disk_Pointer)) != 0)
                    return "��ַ����";
                JumpPointer(Disk_Pointer, ChildBlockSqe, 0);//��ת����Ӧ��λ��
            }
            else{//���û��֪����Ӧ�����ļ�
                JumpPointer(Disk_Pointer, OriSqe, 0);//��ת��ԭ����λ��
                return "��ַ����";
            }
            if (AddStr.eof()){
                directory = OriAddBuf;
                return OriAddBuf;
            }//��������һ��Ŀ¼֮�󵽵�����ײ�������
        }
    }else if(AddBuf.length() == 1 && AddBuf[0] == '.'){
        return directory;
    }else if(AddBuf.length() == 2 && AddBuf[0] == '.' && AddBuf[1] == '.'){
        int index = 0;
        for(int i = 0; i < directory.length(); i++){
            if(directory[i] == '/') index = i;
        }
        directory.erase(index, directory.length() - index);
        return cdAddLocation(Disk_Pointer, directory);
    }else if(AddBuf[0] == '.' && AddBuf[1] == '/'){
        AddBuf.erase(0, 1);
        string directory1(directory);
        directory1.append(AddBuf);
        return cdAddLocation(Disk_Pointer, directory1);
    }else if(AddBuf[0] == '.' && AddBuf[1] == '.' && AddBuf[2] == '/'){
        cdAddLocation(Disk_Pointer, "..");
        AddBuf.erase(0, 2);
        string directory1(directory);
        directory1.append(AddBuf);
        return cdAddLocation(Disk_Pointer, directory1);
    }else if(AddBuf.length() > 0){
        string directory1(directory);
        directory1.append("/");
        directory1.append(AddBuf);
        return cdAddLocation(Disk_Pointer, directory1);
    }
    else
        return "��ַ����";
}

// dir
void ShowAllChildNodeName(fstream& Disk_Pointer, int BlockSqe, string& ReplyInfo){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char* ChildNodeName;
    int* ChildNodeAdd;
    int FileType; 

    JumpPointer(Disk_Pointer, BlockSqe, 112);
    ChildNodeAdd = Output32Bit(Disk_Pointer);
    FileType = FindDocType(Disk_Pointer, Add2BlockSqe(ChildNodeAdd));
    if (ChildNodeAdd[0] == 1){//�������ӽ�㱻ռ����
        int ChildNodeSqe = Add2BlockSqe(ChildNodeAdd);
        ChildNodeName = FindDocName(Disk_Pointer, ChildNodeSqe);
        ReplyInfo = ReplyInfo  + "\n" + string(ChildNodeName);
    }

    JumpPointer(Disk_Pointer, BlockSqe, 112+32);
    ChildNodeAdd = Output32Bit(Disk_Pointer);
    FileType = FindDocType(Disk_Pointer, Add2BlockSqe(ChildNodeAdd));
    if (ChildNodeAdd[0] == 1){//�������ӽ�㱻ռ����
        int ChildNodeSqe = Add2BlockSqe(ChildNodeAdd);
        ChildNodeName = FindDocName(Disk_Pointer, ChildNodeSqe);
        ReplyInfo = ReplyInfo  + "\n" + string(ChildNodeName);
    }

    JumpPointer(Disk_Pointer, BlockSqe, 112+32+32);
    ChildNodeAdd = Output32Bit(Disk_Pointer);
    FileType = FindDocType(Disk_Pointer, Add2BlockSqe(ChildNodeAdd));
    if (ChildNodeAdd[0] == 1){//�������ӽ�㱻ռ����
        int ChildNodeSqe = Add2BlockSqe(ChildNodeAdd);
        ChildNodeName = FindDocName(Disk_Pointer, ChildNodeSqe);
        ReplyInfo = ReplyInfo  + "\n" + string(ChildNodeName);
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
}

void ShowAllInfo(fstream& Disk_Pointer, int BlockSqe, string& ReplyInfo){
    char TempChar;
    int OriSqe = GetBlockSqe(Disk_Pointer);
    ReplyInfo = ReplyInfo + "\n" +"Ŀ¼����" +string(FindDocName(Disk_Pointer, BlockSqe));

    JumpPointer(Disk_Pointer, BlockSqe, 73);//��ת������Ա��������λ��
    ReplyInfo = ReplyInfo + "\n" + "����Ա�������ͣ�";
    for (int i = 0; i < 3; i++){//��ȡ����Ա��������
        Disk_Pointer >> TempChar;
        ReplyInfo += TempChar;
    }

    JumpPointer(Disk_Pointer, BlockSqe, 77);//��ת����ͨ�û���������λ��
    ReplyInfo = ReplyInfo + "\n" + "��ͨ�û��������ͣ�";
    for (int i = 0; i < 3; i++){//��ȡ��ͨ�û���������
        Disk_Pointer >> TempChar;
        ReplyInfo += TempChar;
    }

    JumpPointer(Disk_Pointer, BlockSqe, 80);//��ת�������λ��
    ReplyInfo = ReplyInfo + "\n" + "������ַ��";
    for (int i = 0; i < 32; i++){//��ȡ������ַ
        Disk_Pointer >> TempChar;
        ReplyInfo += TempChar;
    }

    JumpPointer(Disk_Pointer, BlockSqe, 112);//��ת���ӽ��λ��
    ReplyInfo = ReplyInfo + "\n" + "�ӽ���ַ��";
    for (int i = 0; i < 3; i++){//��ȡ�ӽ���ַ
        ReplyInfo += "\n";
        for (int j = 0; j < 32; j++){
            Disk_Pointer >> TempChar;
            ReplyInfo += TempChar;
        }
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
}

// ���ݵ�ַ�ƶ���ָ�룬������Ŀ¼����i�ڵ����
int dirAddLocation(fstream& Disk_Pointer, string AddBuf){
    string CurDirName;//������ʱ�洢��ǰλ��
    if (AddBuf.length() == 0)//�����ַΪ�գ����ص�ǰ���̿����
        return GetBlockSqe(Disk_Pointer);

    if (AddBuf[0] == '/'){
        JumpPointer(Disk_Pointer, 101, 0);//�Ӹ���㿪ʼ����
        /*����ָ��ķָ���/����Ԥ����*/
        for (int i = 0; i < AddBuf.length(); i++)//�ȴ����ַ�е�/����
            if (AddBuf[i] == '/')AddBuf[i] = ' ';//��/�滻Ϊ�ո񣬱�������

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*�жϸ���ַ�Ƿ�������*/
        if (CurDirName != "root")//�������ַ����
            return 0;
        /*ͨ���ӽ�㲻�Ͻ��ж�λ*/
        while (1){//�������ַ���AddressBuf�л�������
            AddStr >> CurDirName;//������һ��Ŀ¼
            int ChildBlockSqe = MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName);
            if (ChildBlockSqe)//����ҵ��˶�Ӧ�����ļ�
                JumpPointer(Disk_Pointer, ChildBlockSqe, 0);//��ת����Ӧ��λ��
            else//���û��֪����Ӧ�����ļ�
                return 0;
            if (AddStr.eof())//��������һ��Ŀ¼֮�󵽵�����ײ�������
                return GetBlockSqe(Disk_Pointer);
        }
    }else if(AddBuf.length() == 1 && AddBuf[0] == '.'){
        return GetBlockSqe(Disk_Pointer);
    }else if(AddBuf.length() == 2 && AddBuf[0] == '.' && AddBuf[1] == '.'){
        string directory1(directory);
        int index = 0;
        for(int i = 0; i < directory1.length(); i++){
            if(directory1[i] == '/') index = i;
        }
        directory1.erase(index, directory1.length() - index);
        return dirAddLocation(Disk_Pointer, directory1);
    }else if(AddBuf[0] == '.' && AddBuf[1] == '/'){
        AddBuf.erase(0, 1);
        string directory1(directory);
        directory1.append(AddBuf);
        return dirAddLocation(Disk_Pointer, directory1);
    }else if(AddBuf[0] == '.' && AddBuf[1] == '.' && AddBuf[2] == '/'){
        cdAddLocation(Disk_Pointer, "..");
        AddBuf.erase(0, 2);
        string directory1(directory);
        directory1.append(AddBuf);
        return dirAddLocation(Disk_Pointer, directory1);
    }else if(AddBuf.length() > 0){
        string directory1(directory);
        directory1.append("/");
        directory1.append(AddBuf);
        return dirAddLocation(Disk_Pointer, directory1);
    }
    else
        return 0;
}

// md
// �ҵ�·���µ����Ҫ����Ŀ¼�����֣�˳�����ָ��Ҳ�ƶ�
string mdAddLocation(fstream& Disk_Pointer, string AddBuf){
    string ErrorReturn = "";
    string CurDirName;//������ʱ�洢��ǰλ��
    if (AddBuf.length() == 0){//�����ַΪ�գ�����
        return ErrorReturn;    
    }
        

    /*�ж�������ַ�Ƿ�Ϊ���Ե�ַ������ǣ���ת����Ӧλ�ò�������Ҫ�������ļ�����������ǣ�ֱ�ӷ�����Ҫ�������ļ���*/
    if (AddBuf[0] == '/'){
        JumpPointer(Disk_Pointer, 101, 0);//�Ӹ���㿪ʼ����
        /*����ָ��ķָ���/����Ԥ����*/
        for (int i = 0; i < AddBuf.length(); i++)//�ȴ����ַ�е�/����
            if (AddBuf[i] == '/')
                AddBuf[i] = ' ';//��/�滻Ϊ�ո񣬱�������

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*�жϸ���ַ�Ƿ�������*/
        if (CurDirName != "root")//�������ַ����
            return ErrorReturn;

        /*ͨ���ӽ�㲻�Ͻ��ж�λ*/
        while (1){//�������ַ���AddressBuf�л�������
            AddStr >> CurDirName;//������һ��Ŀ¼
            if (AddStr.eof())//��������һ��Ŀ¼֮�󵽵�����ײ�����ô��ʱ��������־��Ƕ�Ӧ���ļ���
                break;
            int ChildBlockSqe = MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName);
            if (ChildBlockSqe){//����ҵ��˶�Ӧ�����ļ�
                if (FindDocType(Disk_Pointer, ChildBlockSqe) != 0)//������ļ�����Ŀ¼
                    return ErrorReturn;
                JumpPointer(Disk_Pointer, ChildBlockSqe, 0);//��ת����Ӧ��λ��
            }
            else//���û��֪����Ӧ�����ļ�
                return ErrorReturn;
        }
    }
    else{
        for (int i = 1; i < AddBuf.length(); i++)//�ж��ļ�������û��/����
            if (AddBuf[i] == '/')
                return ErrorReturn;
        CurDirName = AddBuf;
    }
    for (int i = 0; i < CurDirName.length(); i++)
        if (CurDirName[i] == '.')
            return ErrorReturn;
    return CurDirName;
}

// rd
// �鿴Ŀ¼i�ڵ����Ƿ�������ļ�������Ϊ0�� ����Ϊ1 
int IsDirEmpty(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    char TempChar;
    for (int i = 0; i < 3; i++){
        JumpPointer(Disk_Pointer, BlockSqe, 112+32*i);
        Disk_Pointer >> TempChar;
        if (TempChar == '1'){
            JumpPointer(Disk_Pointer, OriSqe, 0);
            return 0;
        }
    }
    return 1;
    JumpPointer(Disk_Pointer, OriSqe,0);
}

// ��ָ��ת�Ƶ�Ҫɾ���Ĵ��̿�����
int rdAddLocation(fstream& Disk_Pointer, string AddBuf){
    string CurDirName;//������ʱ�洢��ǰλ��
    if (AddBuf.length() == 0)//�����ַΪ�գ����ص�ǰ���̿����
        return GetBlockSqe(Disk_Pointer);

    if (AddBuf[0] == '/'){
        JumpPointer(Disk_Pointer, 101, 0);//�Ӹ���㿪ʼ����
        /*����ָ��ķָ���/����Ԥ����*/
        for (int i = 0; i < AddBuf.length(); i++)//�ȴ����ַ�е�/����
            if (AddBuf[i] == '/')
                AddBuf[i] = ' ';//��/�滻Ϊ�ո񣬱�������

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*�жϸ���ַ�Ƿ�������*/
        if (CurDirName != "root")//�������ַ����
            return 0;
        /*ͨ���ӽ�㲻�Ͻ��ж�λ*/
        while (1){//�������ַ���AddressBuf�л�������
            AddStr >> CurDirName;//������һ��Ŀ¼
            int ChildBlockSqe = MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName);
            if (ChildBlockSqe)//����ҵ��˶�Ӧ�����ļ�
                JumpPointer(Disk_Pointer, ChildBlockSqe, 0);//��ת����Ӧ��λ��
            else//���û���ҵ���Ӧ�����ļ�
                return 0;
            if (AddStr.eof()){//��������һ��Ŀ¼֮�󵽵�����ײ�������
                if (FindDocType(Disk_Pointer, GetBlockSqe(Disk_Pointer)) != 0)
                    return 0;
                return GetBlockSqe(Disk_Pointer);
            }
        }
    }
    else
        return 0;
}

void rdDir(fstream& Disk_Pointer, int BlockSqe){
    int FileType = FindDocType(Disk_Pointer, BlockSqe);
    if (FileType == 0){//�����Ӧ����Ŀ¼
    	// ���Ŀ¼��û���ļ� 
        if (IsDirEmpty(Disk_Pointer, BlockSqe)){
            /*�������Ķ�Ӧ�ӽ���ַ���*/
            int* ParNode = FindDocParNodeAdd(Disk_Pointer, BlockSqe);
            char* DocName = FindDocName(Disk_Pointer, BlockSqe);
            int** ParChildNode = FindDocChildNodeAdd(Disk_Pointer, Add2BlockSqe(ParNode));
            if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[0])))){
                JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112);
                for (int i = 0; i < 32; i++)
                    Disk_Pointer << 0;
            }
            else if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[1])))){
                JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112 + 32);
                for (int i = 0; i < 32; i++)
                    Disk_Pointer << 0;
            }
            else if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[2])))){
                JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112 + 64);
                for (int i = 0; i < 32; i++)
                    Disk_Pointer << 0;
            }

            /*��Ŀ¼i���������ɾ��*/
            JumpPointer(Disk_Pointer, BlockSqe, 0);
            BitMapChange(Disk_Pointer, GetBlockSqe(Disk_Pointer), 0);//��λͼ�ж�Ӧi����λ�����
            for (int i = 0; i < 1024; i++)
                Disk_Pointer << 0;
            return;
        }
        // ���Ŀ¼�����ļ� 
        else{
            int** TempIntArr = FindDocChildNodeAdd(Disk_Pointer, BlockSqe);
            /*ɾ�ӽ��*/
            if (TempIntArr[0][0] == 1)
                rdDir(Disk_Pointer, Add2BlockSqe(TempIntArr[0]));
            if (TempIntArr[1][0] == 1)
                rdDir(Disk_Pointer, Add2BlockSqe(TempIntArr[1]));
            if (TempIntArr[2][0] == 1)
                rdDir(Disk_Pointer, Add2BlockSqe(TempIntArr[2]));

            /*ɾ�Լ�*/
            if (IsDirEmpty(Disk_Pointer, BlockSqe))
                rdDir(Disk_Pointer, BlockSqe);
        }
    }
    else{//�����Ӧ�����ļ�
        int* DocContextAdd = FindDocContextNode(Disk_Pointer, BlockSqe);

        /*���ļ������������*/
        BitMapChange(Disk_Pointer, Add2BlockSqe(DocContextAdd),0);
        JumpPointer(Disk_Pointer, Add2BlockSqe(DocContextAdd), 0);
        for (int i = 0; i < 1024; i++)
            Disk_Pointer << 0;

        /*�������Ķ�Ӧ�ӽ���ַ���*/
        int* ParNode = FindDocParNodeAdd(Disk_Pointer, BlockSqe);
        char* DocName = FindDocName(Disk_Pointer, BlockSqe);
        int** ParChildNode = FindDocChildNodeAdd(Disk_Pointer, Add2BlockSqe(ParNode));

        if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[0])))){
            JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112);
            for (int i = 0; i < 32; i++)
                Disk_Pointer << 0;
        }
        else if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[1])))){
            JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112 + 32);
            for (int i = 0; i < 32; i++)
                Disk_Pointer << 0;
        }
        else if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[2])))){
            JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112 + 64);
            for (int i = 0; i < 32; i++)
                Disk_Pointer << 0;
        }

        /*���ļ�i�����������*/
        BitMapChange(Disk_Pointer, BlockSqe, 0);
        JumpPointer(Disk_Pointer, BlockSqe, 0);
        for (int i = 0; i < 240; i++)
            Disk_Pointer << 0;
        return;
    }
} 

// newfile
string FileNameTest(string FileName){
    string Right = "";
    if (FileName.length() == 0)
        return "��ַ���󣬴���ʧ��!";
    if (FileName.length() > 8)
        return "�ļ�������������ʧ�ܣ�";
    int CountPoint = 0;
    for (int i = 0; i < FileName.length(); i++){
        if (FileName[i] == '.')
            CountPoint++;
        if (FileName[i] == '*' || FileName[i] == '/')
            return "�ļ�����ʽ���󣬴���ʧ�ܣ�";
    }
    if (CountPoint != 1)
        return "�ļ�����ʽ���󣬴���ʧ�ܣ�";
    return Right;
}

string FileAccTest(string Acc){
    string Right = "";
    if (Acc.length() != 6)
        return "�������ͳ��ȴ��󣬴���ʧ��!";
    for (int i = 0; i < Acc.length(); i++)
        if (Acc[i] != '0'&& Acc[i] != '1')
            return "�������͸�ʽ���󣬴���ʧ��!";
    return Right;
}
// �ж�������ַ�Ƿ�Ϊ���Ե�ַ������ǣ���ת����Ӧλ�ò�������Ҫ�������ļ�����������ǣ�ֱ�ӷ�����Ҫ�������ļ���
string newfileAddLocation(fstream& Disk_Pointer, string AddBuf){
    string ErrorReturn = "";
    string CurDirName;//������ʱ�洢��ǰλ��
    if (AddBuf.length() == 0)//�����ַΪ�գ�����
        return ErrorReturn;

    /*�ж�������ַ�Ƿ�Ϊ���Ե�ַ������ǣ���ת����Ӧλ�ò�������Ҫ�������ļ�����������ǣ�ֱ�ӷ�����Ҫ�������ļ���*/
    if (AddBuf[0] == '/'){
        JumpPointer(Disk_Pointer, 101, 0);//�Ӹ���㿪ʼ����
        /*����ָ��ķָ���/����Ԥ����*/
        for (int i = 0; i < AddBuf.length(); i++)//�ȴ����ַ�е�/����
            if (AddBuf[i] == '/')
                AddBuf[i] = ' ';//��/�滻Ϊ�ո񣬱�������

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*�жϸ���ַ�Ƿ�������*/
        if (CurDirName != "root")//�������ַ����
            return ErrorReturn;

        /*ͨ���ӽ�㲻�Ͻ��ж�λ*/
        while (1){//�������ַ���AddressBuf�л�������
            AddStr >> CurDirName;//������һ��Ŀ¼
            if (AddStr.eof())//��������һ��Ŀ¼֮�󵽵�����ײ�����ô��ʱ��������־��Ƕ�Ӧ���ļ���
                break;
            int ChildBlockSqe = MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName);
            if (ChildBlockSqe){//����ҵ��˶�Ӧ�����ļ�
                if (FindDocType(Disk_Pointer, ChildBlockSqe) != 0)//������ļ�����Ŀ¼
                    return ErrorReturn;
                JumpPointer(Disk_Pointer, ChildBlockSqe, 0);//��ת����Ӧ��λ��
            }
            else//���û��֪����Ӧ�����ļ�
                return ErrorReturn;
        }
    }
    else
        for (int i = 1; i < AddBuf.length(); i++)//�ж��ļ�������û��/����
            if (AddBuf[i] == '/')
                return ErrorReturn;
    CurDirName = AddBuf;
    return CurDirName;
}

// cat
int catAddLocation(fstream& Disk_Pointer, string AddBuf){
    string CurDirName;//������ʱ�洢��ǰλ��
    if (AddBuf.length() == 0)//�����ַΪ�գ����ص�ǰ���̿����
        return 0;

    if (AddBuf[0] == '/'){
        JumpPointer(Disk_Pointer, 101, 0);//�Ӹ���㿪ʼ����
        /*����ָ��ķָ���/����Ԥ����*/
        for (int i = 0; i < AddBuf.length(); i++)//�ȴ����ַ�е�/����
            if (AddBuf[i] == '/')
                AddBuf[i] = ' ';//��/�滻Ϊ�ո񣬱�������

        istringstream AddStr(AddBuf);
        AddStr >> CurDirName;

        /*�жϸ���ַ�Ƿ�������*/
        if (CurDirName != "root")//�������ַ����
            return 0;
        /*ͨ���ӽ�㲻�Ͻ��ж�λ*/
        while (1)//�������ַ���AddressBuf�л�������
        {
            AddStr >> CurDirName;//������һ��Ŀ¼
            int ChildBlockSqe = MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName);
            if (ChildBlockSqe)//����ҵ��˶�Ӧ�����ļ�
                JumpPointer(Disk_Pointer, ChildBlockSqe, 0);//��ת����Ӧ��λ��
            else//���û��֪����Ӧ�����ļ�
                return 0;
            if (AddStr.eof()){//��������һ��Ŀ¼֮�󵽵�����ײ�������
                //������ļ���һ��Ŀ¼
                if (FindDocType(Disk_Pointer, GetBlockSqe(Disk_Pointer)) != 1){
                    return 0;
                }      
                return GetBlockSqe(Disk_Pointer);
            }
        }
    }
    else
        return 0;
}

string GetFileContext(fstream& Disk_Pointer, int BlockSqe){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    JumpPointer(Disk_Pointer, BlockSqe, 0);
    string FileContext = "";
    char TempChar;
    int* TempOutputIntArr;
    for (int i = 0; i < 128; i++){
        TempOutputIntArr = Output8Bit(Disk_Pointer);
        TempChar=EightInt2Char(TempOutputIntArr);
        if(TempChar != '0') FileContext += TempChar;
    }
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return FileContext;
}

// copy
string GetWindowsFileName(string path){
    for (int i = 0; i < path.length();i++)
        if (path[i] == '\\')
            path[i] = ' ';
    istringstream TempInputStr(path);
    string TempStr;
    while (!TempInputStr.eof())
        TempInputStr >> TempStr;
    return TempStr;
}

string CopyNewFile(fstream& Disk_Pointer,istringstream& Buf){
    int OriSqe = GetBlockSqe(Disk_Pointer);
    string newfileNameBuf;//�û��洢���������еĵ�ַ����
    string newfileAccBuf;
    string newfileContextBuf;
    Buf >> newfileNameBuf;
    Buf >> newfileAccBuf;
    Buf >> newfileContextBuf;

    string CurDirName = newfileAddLocation(Disk_Pointer, newfileNameBuf); // ����
    /*�ļ�����ʽ���*/
    string ReplyInfo = FileNameTest(CurDirName);
    if (ReplyInfo != "")
        return ReplyInfo;

    /*�������͸�ʽ���*/
    ReplyInfo = FileAccTest(newfileAccBuf);
    if (ReplyInfo != "")
        return ReplyInfo;

    int NewINodeBlockSqe = FindFreeAreaInINode(Disk_Pointer);
    int NewDocBlockSqe = FindFreeAreaInDoc(Disk_Pointer);

    /*�ж��ļ�ϵͳ�Ƿ��пռ�*/
    if (!NewINodeBlockSqe)//����ļ�ϵͳ����û�пռ䣬����ʧ��
        return "�ļ�ϵͳ���޿��пռ䣬����ʧ�ܣ�";
    if (!NewDocBlockSqe)//����ļ�ϵͳ����û�пռ䣬����ʧ��
        return "�ļ�ϵͳ���޿��пռ䣬����ʧ�ܣ�";

    /*�ж��������ļ����Ƿ����ӽ�����ļ����ظ�*/
    if (MatchDocName(Disk_Pointer, GetBlockSqe(Disk_Pointer), CurDirName))
        return "�ļ����ظ�������ʧ�ܣ�";

    /*�ж��ӽ���ǲ�������*/
    int FreeChildSqe = FindFreeChildNode(Disk_Pointer, GetBlockSqe(Disk_Pointer));
    if(FreeChildSqe){ // ����
        JumpPointer(Disk_Pointer, GetBlockSqe(Disk_Pointer), 112 + 32 * (FreeChildSqe - 1));//��ת����Ӧ���ӽ��
        int* INode = BlockSqe2Add(NewINodeBlockSqe);
        INode[0] = 1;//����λ��Ϊ1����ʾ���ӽ���Ѿ���ʹ��
        InputIntArr(Disk_Pointer, INode, 32);//����32λ�ӽ��
        JumpPointer(Disk_Pointer, GetBlockSqe(Disk_Pointer), 0);//���ؿ�ͷ
    }
    else
        return "��ǰ�ļ������ļ��Ѵ����ޣ�����ʧ�ܣ�";

    /*�����ļ����г�ʼ��*/
    InputDocName(Disk_Pointer, NewINodeBlockSqe, CurDirName);//�����ļ���
    InputParNode(Disk_Pointer, NewINodeBlockSqe, BlockSqe2Add(GetBlockSqe(Disk_Pointer)));//���븸����ַ
    InputAcc(Disk_Pointer, NewINodeBlockSqe, newfileAccBuf);//�����ļ���������
    InputFileContext(Disk_Pointer, NewINodeBlockSqe, NewDocBlockSqe, newfileContextBuf);//�����ļ�����
    InputFileType(Disk_Pointer, NewINodeBlockSqe, 1);//�����ļ�����
    BitMapChange(Disk_Pointer, NewINodeBlockSqe, 1);
    BitMapChange(Disk_Pointer, NewDocBlockSqe, 1);
    JumpPointer(Disk_Pointer, OriSqe, 0);
    return "�ļ������ɹ���";
}

// del
void delDoc(fstream& Disk_Pointer, int BlockSqe){
    int* DocContextAdd = FindDocContextNode(Disk_Pointer, BlockSqe);

    /*���ļ������������*/
    BitMapChange(Disk_Pointer, Add2BlockSqe(DocContextAdd), 0);
    JumpPointer(Disk_Pointer, Add2BlockSqe(DocContextAdd), 0);
    for (int i = 0; i < 1024; i++)
        Disk_Pointer << 0;

    /*�������Ķ�Ӧ�ӽ���ַ���*/
    int* ParNode = FindDocParNodeAdd(Disk_Pointer, BlockSqe);
    char* DocName = FindDocName(Disk_Pointer, BlockSqe);
    int** ParChildNode = FindDocChildNodeAdd(Disk_Pointer, Add2BlockSqe(ParNode));

    if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[0])))){
        JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112);
        for (int i = 0; i < 32; i++)
            Disk_Pointer << 0;
    }
    else if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[1])))){
        JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112 + 32);
        for (int i = 0; i < 32; i++)
            Disk_Pointer << 0;
    }
    else if (string(DocName) == string(FindDocName(Disk_Pointer, Add2BlockSqe(ParChildNode[2])))){
        JumpPointer(Disk_Pointer, Add2BlockSqe(ParNode), 112 + 64);
        for (int i = 0; i < 32; i++)
            Disk_Pointer << 0;
    }

    /*���ļ�i�����������*/
    BitMapChange(Disk_Pointer, BlockSqe, 0);
    JumpPointer(Disk_Pointer, BlockSqe, 0);
    for (int i = 0; i < 240; i++)
        Disk_Pointer << 0;
    return;
}

// check
void ConsistencyCheck(fstream& Disk_Pointer,int BlockSqe){
    JumpPointer(Disk_Pointer, BlockSqe, 0);
    int FileType = FindDocType(Disk_Pointer, BlockSqe);
    BitMapChange(Disk_Pointer, BlockSqe, 1);//�Ѹ��ļ���Ӧ��λͼ������Ϊ1
    if (FileType == 1){//�����һ���ļ�
        BitMapChange(Disk_Pointer, Add2BlockSqe(FindDocContextNode(Disk_Pointer, BlockSqe)), 1);//���ļ����ݶ�Ӧ��λͼ������Ϊ1
        return;
    }
    else{
        char TempChar;
        JumpPointer(Disk_Pointer, BlockSqe,112);
        Disk_Pointer >> TempChar;
        if (TempChar == '1'){
            JumpPointer(Disk_Pointer, BlockSqe, 112);
            int* ChildNodeAdd = Output32Bit(Disk_Pointer);
            ConsistencyCheck(Disk_Pointer, Add2BlockSqe(ChildNodeAdd));
        }

        JumpPointer(Disk_Pointer, BlockSqe, 144);
        Disk_Pointer >> TempChar;
        if (TempChar == '1'){
            JumpPointer(Disk_Pointer, BlockSqe, 144);
            int* ChildNodeAdd = Output32Bit(Disk_Pointer);
            ConsistencyCheck(Disk_Pointer, Add2BlockSqe(ChildNodeAdd));
        }

        JumpPointer(Disk_Pointer, BlockSqe, 176);
        Disk_Pointer >> TempChar;
        if (TempChar == '1'){
            JumpPointer(Disk_Pointer, BlockSqe, 176);
            int* ChildNodeAdd = Output32Bit(Disk_Pointer);
            ConsistencyCheck(Disk_Pointer, Add2BlockSqe(ChildNodeAdd));
        }
        return;
    }
}
