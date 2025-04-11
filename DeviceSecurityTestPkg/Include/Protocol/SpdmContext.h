// DeviceSecurityTestPkg/Include/Protocol/SpdmContext.h
#ifndef SPDM_CONTEXT_PROTOCOL_H_
#define SPDM_CONTEXT_PROTOCOL_H_

#include <Uefi.h>

#define SPDM_CONTEXT_PROTOCOL_GUID \
  { \
    0x12345678, 0x1234, 0x1234, {0x12, 0x34, 0x12, 0x34, 0x12, 0x34, 0x12, 0x34} \
  }

typedef struct {
  VOID *SpdmContext;
} SPDM_CONTEXT_PROTOCOL;

extern EFI_GUID gSpdmContextProtocolGuid;

#endif