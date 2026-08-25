// entry.cpp

#include "hypervisor.h"
#include "./common/logging.h"

#include <ntddk.h>
#include <wdf.h>

extern "C"
VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
	UNREFERENCED_PARAMETER(DriverObject);
	hypervisor::shutdown();
	log::printf("Driver unloaded\n");
}

NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
	UNREFERENCED_PARAMETER(RegistryPath);

	log::printf("Driver Entry\n");

	DriverObject->DriverUnload = DriverUnload;

	if (!hypervisor::initialize())
	{
		log::error("Hypervisor initialization failed\n");

		return STATUS_UNSUCCESSFUL;
	}

	return STATUS_SUCCESS;
}