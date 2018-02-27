#include <ntddk.h>

#define DeviceName L"\\Device\\PBCKernelStackOverFlow"
#define SymbolicName L"\\DosDevices\\PBCKernelStackOverFlow"

#define BUFFER_SIZE 512

NTSTATUS DispatchIoControl(PDEVICE_OBJECT pDeviceObject,PIRP pIrp);

NTSTATUS sub_0123(PVOID UserBuffer,ULONG Size);

NTSTATUS DispatchCommand(PDEVICE_OBJECT pDeviceObject,PIRP pIrp)
{
	pIrp->IoStatus.Status = STATUS_SUCCESS;
	pIrp->IoStatus.Information = 0;
	IoCompleteRequest(pIrp,IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}


VOID DriverUnload(PDRIVER_OBJECT pDriverObject)
{
	UNICODE_STRING uSymbolicName = {0x00};
	RtlInitUnicodeString(&uSymbolicName,SymbolicName);
	IoDeleteSymbolicLink(&uSymbolicName);
	IoDeleteDevice(pDriverObject->DeviceObject);
	DbgPrint("PBC Kernel StackOverFlow:goodbye!");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT pDriverObject,PUNICODE_STRING pDriverPath)
{
	NTSTATUS status = STATUS_SUCCESS;
	UNICODE_STRING uDeviceName = {0x00};
	UNICODE_STRING uSymbolicName = {0x00};
	PDEVICE_OBJECT pDeviceObject = NULL;
	ULONG index;

	RtlInitUnicodeString(&uDeviceName,DeviceName);

	status = IoCreateDevice(pDriverObject,
		0,
		&uDeviceName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&pDeviceObject);
	if(!NT_SUCCESS(status))
	{
		DbgPrint(L"IoCreateDevice failed:%x\n",status);
		goto ret;
	}
	
	RtlInitUnicodeString(&uSymbolicName,SymbolicName);
	status = IoCreateSymbolicLink(&uSymbolicName,&uDeviceName);
	if(!NT_SUCCESS(status))
	{
		DbgPrint("IoCreateSymbolicLink faild:%x\n",status);
		IoDeleteDevice(pDeviceObject);
		goto ret;
	}
	
	for(index = 0; index < IRP_MJ_MAXIMUM_FUNCTION; index++)
	{
		pDriverObject->MajorFunction[index] = DispatchCommand;
	}
	
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DispatchIoControl;
	pDriverObject->DriverUnload = DriverUnload;


ret:
	return status;
}


NTSTATUS DispatchIoControl(PDEVICE_OBJECT pDeviceObject,PIRP pIrp)
{
	NTSTATUS status = STATUS_SUCCESS;
	PIO_STACK_LOCATION pIrpStack = NULL;
	PVOID Buffer = NULL;
	ULONG BufferSize = 0;

	pIrpStack = IoGetCurrentIrpStackLocation(pIrp);
	Buffer = pIrpStack->Parameters.DeviceIoControl.Type3InputBuffer;
	BufferSize = pIrpStack->Parameters.DeviceIoControl.InputBufferLength;
	
	status = sub_0123(Buffer,BufferSize);

	return status;

}


NTSTATUS sub_0123(PVOID UserBuffer,ULONG Size)
{
	NTSTATUS ntStatus = STATUS_SUCCESS;
	ULONG KernelBuffer[BUFFER_SIZE] = {0x00};

	//__try
	{
		ProbeForRead(UserBuffer,sizeof(KernelBuffer),(ULONG)__alignof(KernelBuffer));
		DbgPrint("UserBuffer address is 0x%x\n",UserBuffer);
		DbgPrint("UserBuffer size is 0x%x\n",Size);
		DbgPrint("KernelBuffer address is 0x%x\n",KernelBuffer);
		DbgPrint("KernelBuffer Size is 0x%x\n",sizeof(KernelBuffer));

		//指定长度为R3发过来的缓冲区长度
		RtlCopyMemory((PVOID)KernelBuffer,UserBuffer,Size);


	}
	/*
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		ntStatus = _exception_code;
		DbgPrint("Exception Code is %d\n",ntStatus);
	}
	*/
	return ntStatus;
	
}
