/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/******************************************************************************
 *
 *  The original Work has been changed by NXP Semiconductors.
 *
 *  Copyright (C) 2015-2018 NXP Semiconductors
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/
#pragma once
#include "NfcJniUtil.h"

namespace android {
extern jmethodID gCachedNfcManagerNotifyNdefMessageListeners;
extern jmethodID gCachedNfcManagerNotifyTransactionListeners;
extern jmethodID gCachedNfcManagerNotifyConnectivityListeners;
extern jmethodID gCachedNfcManagerNotifyEmvcoMultiCardDetectedListeners;
extern jmethodID gCachedNfcManagerNotifyLlcpLinkActivation;
extern jmethodID gCachedNfcManagerNotifyLlcpLinkDeactivated;
extern jmethodID gCachedNfcManagerNotifyLlcpFirstPacketReceived;
extern jmethodID gCachedNfcManagerNotifySeFieldActivated;
extern jmethodID gCachedNfcManagerNotifySeFieldDeactivated;
extern jmethodID gCachedNfcManagerNotifySeListenActivated;
extern jmethodID gCachedNfcManagerNotifySeListenDeactivated;
extern jmethodID gCachedNfcManagerNotifyUiccStatusEvent;
/*
 * host-based card emulation
 */
extern jmethodID gCachedNfcManagerNotifyHostEmuActivated;
extern jmethodID gCachedNfcManagerNotifyHostEmuData;
#if (NXP_EXTNS == TRUE)
#if (NXP_NFCC_HCE_F == TRUE)
extern jmethodID gCachedNfcManagerNotifyT3tConfigure;
extern jmethodID gCachedNfcManagerNotifyReRoutingEntry;
#endif
extern jmethodID gCachedNfcManagerNotifyDownloadInProgress;
extern jmethodID gCachedNfcManagerNotifyFwDwnldRequested;
#endif
extern jmethodID gCachedNfcManagerNotifyHostEmuDeactivated;
extern jmethodID gCachedNfcManagerNotifyAidRoutingTableFull;

extern const char* gNativeP2pDeviceClassName;
extern const char* gNativeLlcpServiceSocketClassName;
extern const char* gNativeLlcpConnectionlessSocketClassName;
extern const char* gNativeLlcpSocketClassName;
extern const char* gNativeNfcTagClassName;
extern const char* gNativeNfcManagerClassName;
extern const char* gNativeNfcSecureElementClassName;
#if (NXP_EXTNS == TRUE)
extern const char* gNativeNfcAlaClassName;
extern const char* gNativeNfcMposManagerClassName;
#endif
}  // namespace android
