#include "util.h"

#include <efilib.h>

const CHAR16* TmpStr(CHAR8 *src, int length) {
	static CHAR16 arr[4][16];
	static int j;
	CHAR16* dest = arr[j = (j+1) % 4];
	int i;
	for (i = 0; i < length && i < 16-1 && src[i]; ++i) {
		dest[i] = src[i];
	}
	dest[i] = 0;
	return dest;
}

UINTN NullPrint(IN CONST CHAR16 *fmt, ...) {
	return 0;
}

const CHAR16* TrimLeft(const CHAR16* s) {
	// Skip white-space and BOM.
	while (s[0] == L'\xfeff' || s[0] == ' ' || s[0] == '\t') {
		++s;
	}
	return s;
}

const CHAR16* StrStr(const CHAR16* haystack, const CHAR16* needle) {
	int len = StrLen(needle);
	while (haystack && haystack[0]) {
		if (StrnCmp(haystack, needle, len) == 0) {
			return haystack;
		}
		++haystack;
	}
	return 0;
}

const CHAR16* StrStrAfter(const CHAR16* haystack, const CHAR16* needle) {
	return (haystack = StrStr(haystack, needle)) ? haystack + StrLen(needle) : 0;
}

UINT64 Random_a, Random_b;

UINT64 Random(void) {
	// Implemented after xoroshiro128plus.c
	if (!Random_a && !Random_b) {
		RandomSeedAuto();
	}
	UINT64 a = Random_a, b = Random_b, r = a + b;
	b ^= a;
	Random_a = rotl(a, 55) ^ b ^ (b << 14);
	Random_b = rotl(b, 36);
	return r;
}

void RandomSeed(UINT64 a, UINT64 b) {
	Random_a = a;
	Random_b = b;
}

void RandomSeedAuto(void) {
	EFI_TIME t;
	RT->GetTime(&t, 0);
	UINT64 a, b = ((((((UINT64) t.Second * 100 + t.Minute) * 100 + t.Hour) * 100 + t.Day) * 100 + t.Month) * 10000 + t.Year) * 300000 + t.Nanosecond;
	BS->GetNextMonotonicCount(&a);
	RandomSeed(a, b), Random(), Random();
}

void WaitKey(void) {
	uefi_call_wrapper(ST->ConIn->Reset,2,ST->ConIn, FALSE);
	WaitForSingleEvent(ST->ConIn->WaitForKey, 0);
}

EFI_INPUT_KEY ReadKey(void) {
	WaitKey();
	EFI_INPUT_KEY key = {0};
	uefi_call_wrapper(ST->ConIn->ReadKeyStroke,2,ST->ConIn, &key);
	return key;
}

void* LoadFileWithPadding(EFI_FILE_HANDLE dir, const CHAR16* path, UINTN* size_ptr, UINTN padding) {
	EFI_STATUS e;
	EFI_FILE_HANDLE handle;


	Print(L"LoadFileWithPadding: path        : %s\n", path );
	Print(L"LoadFileWithPadding: dir file handle : %0x\n", dir );





//	e = dir->Open(dir, &handle, (CHAR16*) path, EFI_FILE_MODE_READ, 0);
	e = uefi_call_wrapper(dir->Open,5, dir, &handle, (CHAR16*) path, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(e)) {
		Print(L"LoadFileWithPadding: Open failed, error = %0x\n", e );
		if (e == EFI_SUCCESS) Print(L" The file was opened.\n");
		if (e == EFI_NOT_FOUND ) Print(L" The specified file could not be found on the device.\n");
		if (e == EFI_NO_MEDIA ) Print(L" The device has no medium.\n");
		if (e == EFI_MEDIA_CHANGED ) Print(L" The device has a different medium in it or the medium is no longer supported.\n");
		if (e == EFI_DEVICE_ERROR ) Print(L" The device reported an error.\n");
		if (e == EFI_VOLUME_CORRUPTED ) Print(L" The file system structures are corrupted.\n");
		if (e == EFI_WRITE_PROTECTED ) Print(L" An attempt was made to create a file, or open a file for write when the media is write-protected.\n");
		if (e == EFI_ACCESS_DENIED ) Print(L" The service denied access to the file.\n");
		if (e == EFI_OUT_OF_RESOURCES ) Print(L" Not enough resources were available to open the file.\n");
		if (e == EFI_VOLUME_FULL ) Print(L" The volume is full.\n");
		Print (L" EFI_SUCCESS,EFI_NOT_FOUND,EFI_NO_MEDIA,EFI_MEDIA_CHANGED,EFI_DEVICE_ERROR,EFI_VOLUME_CORRUPTED,EFI_WRITE_PROTECTED,EFI_ACCESS_DENIED,EFI_OUT_OF_RESOURCES,EFI_VOLUME_FULL = %0x, %0x,%0x,%0x,%0x,%0x,%0x,%0x,%0x,%0x\n",EFI_SUCCESS,EFI_NOT_FOUND,EFI_NO_MEDIA,EFI_MEDIA_CHANGED,EFI_DEVICE_ERROR,EFI_VOLUME_CORRUPTED,EFI_WRITE_PROTECTED,EFI_ACCESS_DENIED,EFI_OUT_OF_RESOURCES,EFI_VOLUME_FULL);
		return 0;
	}
	Print(L" Getting file info...\n");
	EFI_FILE_INFO *info = LibFileInfo(handle);
	UINTN size = info->FileSize;
	Print(L" EFI_FILE_INFO: Size=%u\n",info->Size);
        Print(L" EFI_FILE_INFO: FileSize=%u\n",info->FileSize);
        Print(L" EFI_FILE_INFO: PhysicalSize=%u\n",info->PhysicalSize);
        Print(L" EFI_FILE_INFO: CreateTime=%0x\n",info->CreateTime);
        Print(L" EFI_FILE_INFO: LastAccessTime=%0x\n",info->LastAccessTime);
        Print(L" EFI_FILE_INFO: ModificationTime=%0x\n",info->ModificationTime);
        Print(L" EFI_FILE_INFO: Attribute=%u\n",info->Attribute);
        Print(L" EFI_FILE_INFO: FileName=%s\n",info->FileName);

	FreePool(info);

	void* data = NULL;
//	e = BS->AllocatePool(EfiBootServicesData, size + padding, &data);
	e = uefi_call_wrapper(BS->AllocatePool,3,EfiBootServicesData, size + padding, &data);
	if (EFI_ERROR(e)) {
                Print(L"LoadFileWithPadding: AllocatePool failed, error = %0x\n", e );
		handle->Close(handle);
		return 0;
	}
        for (int i = 0; i < (size + padding); i++) {
                *((char*)data + i) = 0;
        }

	e = uefi_call_wrapper(handle->Read,3,handle, &size, data);
	if ( e == EFI_SUCCESS ) {
		Print(L" The data was read.\n");
		Print(L" size = %u\n",size);
//		Print(L" data = %s\n",data);
	} else {
		Print (L" Read status = %0x\n",e);
		if ( e == EFI_NO_MEDIA ) Print(L"  The device has no medium.\n");
		if ( e == EFI_DEVICE_ERROR ) Print(L"  The device reported an error, or an attempt was made to read from a deleted file, or on entry, the current file position is beyond the end of the file.\n");
		if ( e == EFI_VOLUME_CORRUPTED ) Print(L"  The file system structures are corrupted.\n");
		if ( e == EFI_BUFFER_TOO_SMALL ) Print(L"  The BufferSize is too small to read the current directory entry. BufferSize has been updated with the size needed to complete the request.\n");
		return 0;
	}
	for (int i = 0; i < padding; ++i) {
		*((char*)data + size + i) = 0;
	}
	Print (L" Data =\n");
	for (int i=0; i < 5; i++) {
		Print(L" %0x",*((char*)data+i) );
	}
//	for (int i=3; i < (size +2); i++) {
//		Print(L" %0x",*((char*)data+i) );
//		Print(L"%c",*((char*)data+i) );
//		if (!(i%30)) Print(L"\n");
//	}
	Print(L"\n\n");
        for (int i=size-1; i < size+padding-1; i++) {
                Print(L" %0x",*((char*)data+i) );
        }
        Print(L"\n\n");
//	handle->Close(handle);
	e = uefi_call_wrapper(handle->Close,1,handle);
	Print(L" file closed, status = %0x\n",e);
	if (EFI_ERROR(e)) {
                Print(L"LoadFileWithPadding: Close failed, error = %0x\n", e );
		FreePool(data);
		return 0;
	}
	if (size_ptr) {
		*size_ptr = size;
	}
	Print(L" returning *data\n");
	return data;
}
