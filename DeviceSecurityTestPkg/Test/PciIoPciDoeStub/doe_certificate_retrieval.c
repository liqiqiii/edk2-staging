#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/SpdmDeviceInterface.h>
#include <Protocol/PciIo.h>
#include <IndustryStandard/Pci.h>
#include <IndustryStandard/SpdmDoe.h>

// External declarations from PciIoPciDoeStub.c
extern VOID *mSpdmContext;

EFI_STATUS
PerformDoeDiscoveryAndGetCertificate (
  VOID
  )
{
  EFI_STATUS                  Status;
  UINT32                      TransportMessageSize;
  VOID                        *TransportMessage;
  PCI_DOE_DISCOVERY_REQUEST   DoeRequest;
  UINT8                       *CertChainBuffer;
  UINTN                       CertChainSize;
  
  DEBUG ((DEBUG_INFO, "Starting DOE discovery and certificate retrieval\n"));

  // Step 3: Perform DOE Discovery
  ZeroMem (&DoeRequest, sizeof(DoeRequest));
  DoeRequest.Index = 0;

  Status = libspdm_pci_doe_encode_discovery(
             sizeof(DoeRequest),
             &DoeRequest,
             &TransportMessageSize,
             &TransportMessage
             );
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to encode DOE discovery message: %r\n", Status));
    return Status;
  }

  Status = libspdm_send_message(
             mSpdmContext,
             TransportMessageSize,
             TransportMessage
             );
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to send DOE discovery message: %r\n", Status));
    return Status;
  }

  // Step 4: Get Certificate Chain
  // Allocate buffer for certificate chain
  CertChainSize = MAX_SPDM_CERT_CHAIN_SIZE;
  CertChainBuffer = AllocateZeroPool(CertChainSize);
  if (CertChainBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = libspdm_get_certificate(
             mSpdmContext,
             0,  // slot_id
             &CertChainSize,
             CertChainBuffer
             );
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to get certificate chain: %r\n", Status));
    FreePool(CertChainBuffer);
    return Status;
  }

  // Step 5: Complete Connection Flow
  Status = libspdm_get_version(mSpdmContext);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to get version: %r\n", Status));
    goto Exit;
  }

  Status = libspdm_get_capabilities(mSpdmContext);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to get capabilities: %r\n", Status));
    goto Exit;
  }

  Status = libspdm_negotiate_algorithms(mSpdmContext);
  if (EFI_ERROR(Status)) {
    DEBUG ((DEBUG_ERROR, "Failed to negotiate algorithms: %r\n", Status));
    goto Exit;
  }

  DEBUG ((DEBUG_INFO, "Successfully completed DOE certificate retrieval\n"));

Exit:
  if (CertChainBuffer != NULL) {
    FreePool(CertChainBuffer);
  }
  return Status;
}