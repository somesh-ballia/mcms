#!/usr/bin/python

import os
import sys
import smtplib
from email.MIMEText import MIMEText
from datetime import date
import pickle



## ----------------------------- Mail Sender -----------------------
class MailSender:
    def __init__(self,serverName):
        self.m_serverName = serverName
        self.m_mailSuffix = '@polycom.co.il'
        self.m_server = smtplib.SMTP(serverName)

    def SendMail(self,source,dest,subject,textMsg):
        msg = MIMEText(textMsg)
        msg['Subject'] = subject
        msg['From'] = source #'Me <me@example.com>'
        msg['To'] = dest #'You <you@example.com>'
        self.m_server.sendmail(source,dest + self.m_mailSuffix,
                                  msg.as_string())
        
    def VerifyUser(self,userAccount):
        print "Verifying user account: "+ userAccount + self.m_mailSuffix
        self.m_server.verify(userAccount)# + self.m_mailSuffix)
        
    def __del__(self):
        self.m_server.quit()
    
## ------------------------------------------------------------------

fileOwners={
'LogUtil.cpp':'No Owner'
,'ResourceProcess.cpp':'Ron L.'
,'ConnToCardManager.cpp':'Ron L.'
,'IPServiceResources.cpp':'Ron L.'
,'ConfStateMach.cpp':'Ron L.'
,'ConfResources.cpp':'Ron L.'
,'RsrcAlloc.cpp':'Ron L.'
,'ResourceMonitor.cpp':'Ron L.'
,'SystemResources.cpp':'Ron L.'
,'CRsrcDetailGet.cpp':'Ron L.'
,'ResourceManager.cpp':'Ron L.'
,'RsrvManager.cpp':'Ron L.'
,'CSApiRxSocketMngr.cpp':'Yehudit'
,'CSApiProcess.cpp':'Yehudit'
,'CSApiManager.cpp':'Yehudit'
,'CSApiMonitor.cpp':'Yehudit'
,'CSApiRxSocket.cpp':'Yehudit'
,'CSApiMplMcmsProtocolTracer.cpp':'Yehudit'
,'CSApiDispatcherTask.cpp':'Yehudit'
,'CSApiTxSocket.cpp':'Yehudit'
,'McmsDaemonMonitor.cpp':'Sagi'
,'WatchdogIF.cpp':'Sagi'
,'McmsDaemonManager.cpp':'Sagi'
,'McmsDaemonProcess.cpp':'Sagi'
,'McuStateGet.cpp':'Yehudit'
,'Authentication.cpp':'Yehudit'
,'SysConfigSet.cpp':'Yehudit'
,'McuMngrManager.cpp':'Yehudit'
,'IpInterfacesListGet.cpp':'Yehudit'
,'ActiveAlarmsListGet.cpp':'Yehudit'
,'McuMngrMonitor.cpp':'Yehudit'
,'McuTimeGet.cpp':'Yehudit'
,'SoftwareLocation.cpp':'Yehudit'
,'McuState.cpp':'Yehudit'
,'SystemTime.cpp':'Yehudit'
,'Licensing.cpp':'Yehudit'
,'LicensingGet.cpp':'Yehudit'
,'McuMngrProcess.cpp':'Yehudit'
,'SysConfigGet.cpp':'Yehudit'
,'FaultsMonitor.cpp':'Yehudit'
,'HlogList.cpp':'Yehudit'
,'FaultsManager.cpp':'Yehudit'
,'FaultsProcess.cpp':'Yehudit'
,'CommFaultsDBGet.cpp':'Yehudit'
,'GideonSimCardTask.cpp':'Dovev'
,'GideonSimCardApi.cpp':'Dovev'
,'GideonSimParties.cpp':'Dovev'
,'GideonSimLogicalUnit.cpp':'Dovev'
,'SimCardRxSocket.cpp':'Dovev'
,'GideonSimLoggerRxSocket.cpp':'Dovev'
,'GideonSimMonitor.cpp':'Dovev'
,'GideonSimProcess.cpp':'Dovev'
,'CommGideonSimSet.cpp':'Dovev'
,'SimGuiRxSocket.cpp':'Dovev'
,'GideonSimManager.cpp':'Dovev'
,'GideonSimCardAckStatusList.cpp':'Dovev'
,'SimGuiTxSocket.cpp':'Dovev'
,'GideonSimCardUnit.cpp':'Dovev'
,'SimCardTxSocket.cpp':'Dovev'
,'GideonSimConfig.cpp':'Dovev'
,'GideonSimLoggerTxSocket.cpp':'Dovev'
,'GideonSimLogicalModule.cpp':'Dovev'
,'CdrFullGet.cpp':'Yehudit'
,'CDRProcess.cpp':'Yehudit'
,'CDRManager.cpp':'Yehudit'
,'CDRMonitor.cpp':'Yehudit'
,'CdrListGet.cpp':'Yehudit'
,'CDRLog.cpp':'Yehudit'
,'CSInterface.cpp':'No Owner'
,'ApacheModuleManager.cpp':'Kobi'
,'ApacheModuleProcess.cpp':'Kobi'
,'XmlMiniParser.cpp':'Kobi'
,'FileListGet.cpp':'Kobi'
,'CreateRemoveDir.cpp':'Kobi'
,'ConnectionListGet.cpp':'Kobi'
,'ApacheModuleLib.cpp':'Kobi'
,'ApacheModuleMonitor.cpp':'Kobi'
,'Rename.cpp':'Kobi'
,'ApacheModuleEngine.cpp':'Kobi'
,'ConnectionList.cpp':'Kobi'
,'ZipWrapper.cpp':'Kobi'
,'HeaderPars.cpp':'Kobi'
,'ApacheModuleMain.cpp':'No Owner'
,'SwitchTask.cpp':'Yehudit'
,'MfaApi.cpp':'Yehudit'
,'MfaTask.cpp':'Yehudit'
,'MediaIpConfigVector.cpp':'Yehudit'
,'CardsProcess.cpp':'Yehudit'
,'HwMonitoring.cpp':'Yehudit'
,'CommCardDBGet.cpp':'Yehudit'
,'CardsDispatcherTask.cpp':'Yehudit'
,'MediaIpParamsVector.cpp':'Yehudit'
,'SwitchApi.cpp':'Yehudit'
,'CardsMonitor.cpp':'Yehudit'
,'IvrAddMusicSourceVector.cpp':'Yehudit'
,'CardsManager.cpp':'Yehudit'
,'CardMngrLoaded.cpp':'Yehudit'
,'DNSAgentMonitor.cpp':'Ori P.'
,'DNSAgentProcess.cpp':'Ori P.'
,'DNSAgentManager.cpp':'Ori P.'
,'SystemQuery.cpp':'Ori P.'
,'Main.cpp':'No Owner'
,'AuthenticationMonitor.cpp':'Kobi'
,'AuthenticationProcess.cpp':'Kobi'
,'AuthenticationManager.cpp':'Kobi'
,'ChangePassword.cpp':'Kobi'
,'OperListGet.cpp':'Kobi'
,'NewOperator.cpp':'Kobi'
,'MplApiTxSocket.cpp':'Yehudit'
,'MplApiRxSocket.cpp':'Yehudit'
,'MplApiDispatcherTask.cpp':'Yehudit'
,'MplApiSpecialCommandHandler.cpp':'Yehudit'
,'MplApiRxSocketMngr.cpp':'Yehudit'
,'MplApiProcess.cpp':'Yehudit'
,'MplApiManager.cpp':'Yehudit'
,'MplApiMonitor.cpp':'Yehudit'
,'McuCmdProcess.cpp':'Sagi'
,'CommConfService.cpp':'Yehudit'
,'CommCardService.cpp':'Yehudit'
,'CIPServiceListGet.cpp':'Yehudit'
,'CommMcmsService.cpp':'Yehudit'
,'CSMngrMonitor.cpp':'Yehudit'
,'CSMngrManager.cpp':'Yehudit'
,'WrappersCS.cpp':'Yehudit'
,'CommIPSListService.cpp':'Yehudit'
,'CommStartupService.cpp':'Yehudit'
,'CommCsModuleService.cpp':'Yehudit'
,'CommRsrcService.cpp':'Yehudit'
,'CommDnsAgentService.cpp':'Yehudit'
,'CommMcuService.cpp':'Yehudit'
,'CommProxyService.cpp':'Yehudit'
,'CIPServiceDel.cpp':'Yehudit'
,'CIPServiceFullListGet.cpp':'Yehudit'
,'CSMngrMplMcmsProtocolTracer.cpp':'Yehudit'
,'CommServiceService.cpp':'Yehudit'
,'CommService.cpp':'Yehudit'
,'CommGKService.cpp':'Yehudit'
,'CSMngrProcess.cpp':'Yehudit'
,'EpSimEndpointsList.cpp':'David'
,'EpGuiRxSocket.cpp':'David'
,'EndpointsSimProcess.cpp':'David'
,'EpSimProxyEndpointsTask.cpp':'David'
,'EpSimEndpointsTask.cpp':'David'
,'EpCsApiTxSocket.cpp':'David'
,'EpSimPSTNEndpointsTask.cpp':'David'
,'EndpointsSimManager.cpp':'David'
,'EpSimScriptsEndpointsTask.cpp':'David'
,'CommEndpointsSimSet.cpp':'David'
,'EndpointH323.cpp':'David'
,'EndpointSip.cpp':'David'
,'EpSimH323EpList.cpp':'David'
,'EpCsApiRxSocket.cpp':'David'
,'EpGuiTxSocket.cpp':'David'
,'EpSimH323EndpointsTask.cpp':'David'
,'EpSimCSEndpointsTask.cpp':'David'
,'EpSimGKEndpointsTask.cpp':'David'
,'EpSimGKRegistrationsList.cpp':'David'
,'EndpointsSimConfig.cpp':'David'
,'EpSimSipEpList.cpp':'David'
,'Endpoint.cpp':'David'
,'EpSimH323BehaviorList.cpp':'David'
,'EpSimBasicEpList.cpp':'David'
,'EpSimSipEndpointsTask.cpp':'David'
,'EpSimProxyConfList.cpp':'David'
,'EpSimBasicEndpointsTask.cpp':'David'
,'EpSimCapSetsList.cpp':'David'
,'EndpointsSimMonitor.cpp':'David'
,'BoardDetails.cpp':'David'
,'QAAPIProcess.cpp':'Sagi'
,'QAAPIRxSocket.cpp':'Sagi'
,'QAAPIManager.cpp':'Sagi'
,'QAAPITxSocket.cpp':'Sagi'
,'httpBldr.cpp':'Sagi'
,'HTTPPars.cpp':'Sagi'
,'QAAPIMonitor.cpp':'Sagi'
,'AppServerData.cpp':'Sagi'
,'HttpRcvr.cpp':'Sagi'
,'ConfiguratorManager.cpp':'Sagi'
,'ConfiguratorMonitor.cpp':'Sagi'
,'ConfiguratorProcess.cpp':'Sagi'
,'FileService.cpp':'No Owner'
,'LoggerMonitor.cpp':'No Owner'
,'LogFileManager.cpp':'No Owner'
,'EMATrace.cpp':'No Owner'
,'LoggerRxSocket.cpp':'No Owner'
,'ContainerClientLogger.cpp':'No Owner'
,'TraceTailer.cpp':'No Owner'
,'LoggerTxSocket.cpp':'No Owner'
,'LoggerProcess.cpp':'No Owner'
,'LoggerManager.cpp':'No Owner'
,'ZipCompressor.cpp':'No Owner'
,'GKProcess.cpp':'Uri A.'
,'GKGlobals.cpp':'Uri A.'
,'GKManagerApi.cpp':'Uri A.'
,'GKCall.cpp':'Uri A.'
,'GKService.cpp':'Uri A.'
,'GKToCsInterface.cpp':'Uri A.'
,'GKMonitor.cpp':'Uri A.'
,'GKManager.cpp':'Uri A.'
,'GkAlias.cpp':'Uri A.'
,'SingleToneTask.cpp':'Sagi'
,'TestClientMonitor.cpp':'Sagi'
,'MyRxTask.cpp':'Sagi'
,'TestClientProcess.cpp':'Sagi'
,'TestClientManager.cpp':'Sagi'
,'TestTask.cpp':'Sagi'
,'MyTxTask.cpp':'Sagi'
,'InstallerMonitor.cpp':'No Owner'
,'InstallerProcess.cpp':'No Owner'
,'InstallerManager.cpp':'No Owner'
,'DemoProcess.cpp':'Sagi'
,'DemoManager.cpp':'Sagi'
,'DemoMonitor.cpp':'Sagi'
,'ClientLoggerProcess.cpp':'Yehudit'
,'DHTable.cpp':'Udi'
,'DHTask.cpp':'Udi'
,'EncryptionKeyServerMonitor.cpp':'Udi'
,'EncryptionKeyServerProcess.cpp':'Udi'
,'EncryptionKeyServerManager.cpp':'Udi'
,'UnifiedComMode.cpp':'Romem'
,'AutoLayoutSet.cpp':'Ron L.'
,'ConfIpParameters.cpp':'Michael V.'
,'LayoutHandler.cpp':'Ron L.'
,'PartyApi.cpp':'Ron L.'
,'BridgePartyCntl.cpp':'Ron L.'
,'IpChannelDetails.cpp':'Michael V.'
,'VidSubImage.cpp':'Ron L.'
,'ConfAppMngrInitParams.cpp':'Ron L.'
,'DwordBitMask.cpp':'Ron L.'
,'SecondaryParameters.cpp':'Ron L.'
,'MoveParams.cpp':'Romem'
,'ReceptionSip.cpp':'Ori P.'
,'AttendedStruct.cpp':'Ron L.'
,'H323ImportPartyCntl.cpp':'Michael V.'
,'BridgePartyMediaParams.cpp':'Ron L.'
,'BridgePartyInitParams.cpp':'Ron L.'
,'ConfPartyProcess.cpp':'Ron L.'
,'H323Util.cpp':'Michael V.'
,'H323Caps.cpp':'Michael V.'
,'RsrvPartyAdd.cpp':'Ron L.'
,'AudioBridgeInitParams.cpp':'Ron L.'
,'AudioHardwareInterface.cpp':'Ron L.'
,'#ConfParty.cpp#':'Ron L.'
,'HardwareInterface.cpp':'Ron L.'
,'NetWorkChnlCntl.cpp':'Ron L.'
,'PartyMonitor.cpp':'Ron L.'
,'NetChnlCntl.cpp':'Ron L.'
,'H323Party.cpp':'Michael V.'
,'RsrvPartyDel.cpp':'Ron L.'
,'ConfPartyManager.cpp':'Ron L.'
,'IpPartyControl.cpp':'Michael V.'
,'H323PartyOut.cpp':'Michael V.'
,'IVRServiceList.cpp':'David'
,'H264Util.cpp':'Ron L.'
,'IPUtils.cpp':'Ron L.'
,'ConfPartyMplMcmsProtocolTracer.cpp':'Ron L.'
,'PartyInterface.cpp':'Ron L.'
,'H323DialOutSimulationParty.cpp':'Michael V.'
,'VideoLayoutDrv.cpp':'Ron L.'
,'BridgePartyMediaUniDirection.cpp':'Ron L.'
,'ConfAppPartiesList.cpp':'Ron L.'
,'FileListGet.cpp':'Udi'
,'BridgeInitParams.cpp':'Ron L.'
,'SIPPartyInSimulation.cpp':'Uri A.'
,'SIPPartyOUT.cpp':'Uri A.'
,'ReceptionList.cpp':'Udi'
,'H323DelPartyControl.cpp':'Michael V.'
,'DelVoicePartyCntl.cpp':'Ron L.'
,'BridgeMoveParams.cpp':'Ron L.'
,'CommConfDBGet.cpp':'Ori P.'
,'H323NetSetup.cpp':'Michael V.'
,'PartyConnection.cpp':'Ron L.'
,'AudioBridgePartyCntl.cpp':'Ron L.'
,'ReceptionH323.cpp':'Michael V.'
,'IVRStartIVR.cpp':'David'
,'SIPParty.cpp':'Uri A.'
,'PartyList.cpp':'Ron L.'
,'Bridge.cpp':'Ron L.'
,'VisualEffectsParamsDrv.cpp':'Ron L.'
,'SipCall.cpp':'Uri A.'
,'FECCBridge.cpp':'Romem'
,'ConfParty.cpp':'Ron L.'
,'Party.cpp':'Ron L.'
,'IVRDtmfColl.cpp':'David'
,'VideoBridge.cpp':'Ron L.'
,'IsdnParty.cpp':'Ron L.'
,'IVRFeatures.cpp':'David'
,'SipPartyControlImport.cpp':'Uri A.'
,'SIPPartyControlDelete.cpp':'Uri A.'
,'H323PartyIn.cpp':'Michael V.'
,'ConfAppInfo.cpp':'Ron L.'
,'ConfAppWaitEventsList.cpp':'Ron L.'
,'ConfAppFeatureObject.cpp':'Ron L.'
,'LectureModeParamsDrv.cpp':'Ron L.'
,'ConfPartyGlobals.cpp':'Ron L.'
,'MeetingRoomDBGet.cpp':'Udi'
,'IsdnNetSetup.cpp':'Ron L.'
,'CsInterface.cpp':'Ron L.'
,'Layout.cpp':'Ron L.'
,'SipScm.cpp':'Uri A.'
,'BridgePartyExportParams.cpp':'Ron L.'
,'CommResDBGet.cpp':'Ori P.'
,'SIPMsftCX.cpp':'Uri A.'
,'CommRes.cpp.ann':'Ori P.'
,'CommResDB.cpp':'Ori P.'
,'ConfPartySpecific.cpp':'Ron L.'
,'ConnectedVoicePartyCntl.cpp':'Ron L.'
,'IVRCntl.cpp':'David'
,'CapInfo.cpp':'Ron L.'
,'SipCaps.cpp':'Uri A.'
,'GlobalH323MonitoringFunc.cpp':'Michael V.'
,'NetCause.cpp':'Ron L.'
,'AudioBridge.cpp':'Ron L.'
,'IVRSlidesList.cpp':'David'
,'IpNetSetup.cpp':'Michael V.'
,'FECCBridgeInitParams.cpp':'Romem'
,'Interface.cpp':'Romem'
,'LobbyApi.cpp':'Ron L.'
,'ConfPartyDispatcherTask.cpp':'Ron L.'
,'RsrcParams.cpp':'Ron L.'
,'H323Control.cpp':'Michael V.'
,'CommRes.cpp':'Ron L.'
,'H323GkStatus.cpp':'Michael V.'
,'BridgeInterface.cpp':'Ron L.'
,'SipPartyOutSimulation.cpp':'Uri A.'
,'ReceptionIVR.cpp':'David'
,'Lobby.cpp':'Udi'
,'CapClass.cpp':'Ron L.'
,'H323ExportPartyCntl.cpp':'Michael V.'
,'AudioBridgeInterface.cpp':'Ron L.'
,'GetMRSpecific.cpp':'Udi'
,'SipNetSetup.cpp':'Uri A.'
,'H323PartyInSimulation.cpp':'Michael V.'
,'IpServiceListManager.cpp':'Michael V.'
,'CUpdateInfo.cpp':'Udi'
,'encrAuth.cpp':'Udi'
,'BridgePartyAudioInOut.cpp':'Ron L.'
,'SIPInternals.cpp':'Uri A.'
,'Conf1.cpp':'Ron L.'
,'GetProfileSpecific.cpp':'Udi'
,'CommConf.cpp':'Ori P.'
,'VideoBridgeLectureModeParams.cpp':'Ron L.'
,'IVRServiceAdd.cpp':'David'
,'SIPConfPack.cpp':'Uri A.'
,'ConfPartyMonitor.cpp':'Ron L.'
,'ConfPartyRoutingTable.cpp':'Ron L.'
,'IVRLanguageAdd.cpp':'David'
,'IvrApiCommandCreator.cpp':'David'
,'Image.cpp':'Ron L.'
,'VideoBridgePartyCntl.cpp':'Ron L.'
,'H323AddPartyControl.cpp':'Michael V.'
,'CommResDBAction.cpp':'Ori P.'
,'VideoInterface.cpp':'Ron L.'
,'H323Scm.cpp':'Michael V.'
,'SIPPartyControl.cpp':'Uri A.'
,'StructTmDrv.cpp':'Ron L.'
,'H323ChangeModePartyControl.cpp':'Michael V.'
,'ConfAppMngrInterface.cpp':'Ron L.'
,'ConfAction.cpp':'Ron L.'
,'IVRServiceSetDefault.cpp':'David'
,'BridgePartyDisconnectParams.cpp':'Ron L.'
,'IVRServiceListGet.cpp':'David'
,'CommConfSpecific.cpp':'Ori P.'
,'NetSetup.cpp':'Ori P.'
,'H323PartyControl.cpp':'Michael V.'
,'RsrcDesc.cpp':'Ron L.'
,'EncryptionKey.cpp':'Udi'
,'SIPPartyIN.cpp':'Uri A.'
,'BridgePartyAudioParams.cpp':'Ron L.'
,'RsrvPartyAction.cpp':'Ron L.'
,'BridgePartyVideoOut.cpp':'Ron L.'
,'AddVoicePartyCntl.cpp':'Ron L.'
,'VideoBridgeInitParams.cpp':'Ron L.'
,'CommModeInfo.cpp':'Ron L.'
,'ConfAppMngr.cpp':'Ron L.'
,'CommConfDB.cpp':'Ori P.'
,'IVRServiceDel.cpp':'David'
,'VoicePartyCntl.cpp':'Ron L.'
,'H323StrCap.cpp':'Michael V.'
,'NetInterface.cpp':'Ron L.'
,'PartyCntl.cpp':'Ron L.'
,'RsrvParty.cppold':'Ron L.'
,'SipRefer.cpp':'Uri A.'
,'SipPartyControlExport.cpp':'Uri A.'
,'IVRService.cpp':'David'
,'Reception.cpp':'Udi'
,'IVRManager.cpp':'David'
,'NetCallingParty.cpp':'Udi'
,'CallAndChannelInfo.cpp':'Udi'
,'SIPPartyControlChangeMode.cpp':'Uri A.'
,'ConfAppActiveEventsList.cpp':'Ron L.'
,'ConfApi.cpp':'Ron L.'
,'SIPPartyControlAdd.cpp':'Uri A.'
,'BridgePartyVideoIn.cpp':'Ron L.'
,'VideoLayoutPartyDrv.cpp':'Ron L.'
,'FECCBridgePartyInitParams.cpp':'Romem'
,'SIPControl.cpp':'Uri A.'
,'PartyRsrcDesc.cpp':'Ori P.'
,'VideoBridgeInterface.cpp':'Ron L.'
,'Conf.cpp':'Ron L.'
,'BridgePartyVideoParams.cpp':'Ron L.'
,'IsdnVoiceParty.cpp':'Ron L.'
,'CommResShort.cpp':'Ron L.'
,'TerminalListManager.cpp':'Ron L.'
,'NetCalledParty.cpp':'Ron L.'
,'IpParty.cpp':'Michael V.'
,'BridgePartyList.cpp':'Ron L.'
,'ConfPartyManagerLocalApi.cpp':'Ron L.'
,'Observer.cpp':'Ori P.'
,'IsdnVoicePartyOut.cpp':'Ron L.'
,'TestMain.cpp':'No Owner'
,'SipProxyMonitor.cpp':'Ori P.'
,'SipProxyApi.cpp':'Ori P.'
,'SipProxyManager.cpp':'Ori P.'
,'SipProxyConfCntl.cpp':'Ori P.'
,'SipProxyProcess.cpp':'Ori P.'
,'SystemFunctions.cpp':'No Owner'
,'OsSocketClient.cpp':'No Owner'
,'OsSocketListener.cpp':'No Owner'
,'OsQueue.cpp':'No Owner'
,'Semaphore.cpp':'No Owner'
,'OsSocketConnected.cpp':'No Owner'
,'OsTask.cpp':'No Owner'
,'OsFileIF.cpp':'No Owner'
,'Trace.cpp':'No Owner'
,'TraceClass.cpp':'No Owner'
,'StatusStringConverter.cpp.keep':'No Owner'
,'RequestHandler.cpp':'No Owner'
,'TimerDesc.cpp':'No Owner'
,'TaskApi.cpp':'No Owner'
,'SlotNumConvertor.cpp':'No Owner'
,'ManagerTask.cpp':'No Owner'
,'ErrorHandlerTask.cpp':'No Owner'
,'Timer.cpp':'No Owner'
,'SocketTxTask.cpp':'No Owner'
,'SocketApi.cpp':'No Owner'
,'SocketTask.cpp':'No Owner'
,'ManagerApi.cpp':'No Owner'
,'OpcodeStringConverter.cpp':'No Owner'
,'StartupConditionContainer.cpp':'No Owner'
,'StatusStringConverter.cpp':'No Owner'
,'StateMachine.cpp':'No Owner'
,'MonitorTask.cpp':'No Owner'
,'ProcessBase.cpp':'No Owner'
,'HlogApi.cpp':'No Owner'
,'ListenSocket.cpp':'No Owner'
,'SysConfig.cpp':'No Owner'
,'SysConfigEma.cpp':'No Owner'
,'SingleToneApi.cpp':'No Owner'
,'ListenSocketApi.cpp':'No Owner'
,'SysConfigBase.cpp':'No Owner'
,'DispatcherTask.cpp':'No Owner'
,'SystemState.cpp':'No Owner'
,'SocketRxTask.cpp':'No Owner'
,'ClientLoggerTask.cpp':'No Owner'
,'TaskApp.cpp':'No Owner'
,'WatchDogTask.cpp':'No Owner'
,'ClientSocket.cpp':'No Owner'
,'AlarmableTask.cpp':'No Owner'
,'zlibengn.cpp':'No Owner'
,'psosxml.cpp':'No Owner'
,'StrArray.cpp':'No Owner'
,'MockCMplMcmsProtocol.cpp':'No Owner'
,'PairOfSockets.cpp':'No Owner'
,'CDRShort.cpp':'No Owner'
,'CDRLogApi.cpp':'No Owner'
,'SIPProxyIpParameters.cpp':'No Owner'
,'StructTm.cpp':'No Owner'
,'CommResAdd.cpp':'No Owner'
,'ConfPartyManagerApi.cpp':'No Owner'
,'TransactionsFactory.cpp':'No Owner'
,'WrappersCards.cpp':'No Owner'
,'IpService.cpp':'No Owner'
,'WrappersSipProxy.cpp':'No Owner'
,'IpConfiguration.cpp':'No Owner'
,'OperMask.cpp':'No Owner'
,'OperEvent.cpp':'No Owner'
,'WrappersGK.cpp':'No Owner'
,'Native.cpp':'No Owner'
,'MplMcmsProtocolTracer.cpp':'No Owner'
,'LectureModeParams.cpp':'No Owner'
,'IpInterface.cpp':'No Owner'
,'NetChannConn.cpp':'No Owner'
,'SipUtils.cpp':'No Owner'
,'FaultsContainer.cpp':'No Owner'
,'WrappersResource.cpp':'No Owner'
,'IpParameters.cpp':'No Owner'
,'RsrvParty.cpp':'No Owner'
,'MediaIpConfig.cpp':'No Owner'
,'HlogElement.cpp':'No Owner'
,'CDRUtils.cpp':'No Owner'
,'IVRPlayMessage.cpp':'No Owner'
,'MediaIpParameters.cpp':'No Owner'
,'CDRDetal.cpp':'No Owner'
,'ConfStart.cpp':'No Owner'
,'RsrvManagerApi.cpp':'No Owner'
,'ObjString.cpp':'No Owner'
,'FilterTrace.cpp':'No Owner'
,'H221Str.cpp':'No Owner'
,'SerializeObject.cpp':'No Owner'
,'Versions.cpp':'No Owner'
,'VideoLayout.cpp':'No Owner'
,'ConfContactInfo.cpp':'No Owner'
,'SharedMemory.cpp':'No Owner'
,'Operator.cpp':'No Owner'
,'OperCfg.cpp':'No Owner'
,'IpV6.cpp':'No Owner'
,'FileList.cpp':'No Owner'
,'ConfigManagerApi.cpp':'No Owner'
,'SystemTick.cpp':'No Owner'
,'LogInConfirm.cpp':'No Owner'
,'BilParty.cpp':'No Owner'
,'DummyEntry.cpp':'No Owner'
,'NetworkParameters.cpp':'No Owner'
,'KeyCode.cpp':'No Owner'
,'WrappersConfParty.cpp':'No Owner'
,'MplMcmsProtocol.cpp':'No Owner'
,'IfConfig.cpp':'No Owner'
,'FilterTraceContainer.cpp':'No Owner'
,'TraceStream.cpp':'No Owner'
,'WrappersCSBase.cpp':'No Owner'
,'VisualEffectsParams.cpp':'No Owner'
,'GKManagerUtils.cpp':'No Owner'
,'SipProxyManagerApi.cpp':'No Owner'
,'VideoCellLayout.cpp':'No Owner'
,'IpPortRange.cpp':'No Owner'
,'TranEntry.cpp':'No Owner'
,'FaultDesc.cpp':'No Owner'
,'WrappersCommon.cpp':'No Owner'
,'UserDefinedInfo.cpp':'No Owner'
,'IVRAvMsgStruct.cpp':'No Owner'
,'TerminalCommand.cpp':'No Owner'
,'StartupCondDependTree.cpp':'No Owner'
,'LogInRequest.cpp':'No Owner'
,'IPServiceDynamicList.cpp':'No Owner'
,'Request.cpp':'No Owner'
,'DnsConfiguration.cpp':'No Owner'
,'WrappersDnsAgent.cpp':'No Owner'
,'IpRouter.cpp':'No Owner'
,'IpV4.cpp':'No Owner'
,'H323Alias.cpp':'No Owner'
,'Segment.cpp':'No Owner'
,'InitCommonStrings.cpp':'No Owner'
,'CallPart.cpp':'No Owner'
,'McuMemory.cpp':'No Owner'
,'IpCommonUtilTrace.cpp':'No Owner'
,'BigDiv.cpp':'No Owner'
,'CdrApiClasses.cpp':'No Owner'
,'MessageHeader.cpp':'No Owner'
,'FilterByLevel.cpp':'No Owner'
,'DynIPSProperties.cpp':'No Owner'
,'CommResApi.cpp':'No Owner'
,'PartyIdTaskApi.cpp':'No Owner'
,'NStream.cpp':'No Owner'
,'OperatorList.cpp':'No Owner'
,'PObject.cpp':'No Owner'
,'StringsMaps.cpp':'No Owner'
,'EnumsToStrings.cpp':'No Owner'
,'AcStartupIndFormats.cpp':'No Owner'
,'SipSdpAndHdrsFormat.cpp':'No Owner'
,'BinBuilder.cpp':'No Owner'
,'SipCsIndFormat.cpp':'No Owner'
,'CsHeaderFormat.cpp':'No Owner'
,'H323CommonParamsFormat.cpp':'No Owner'
,'AcServiceIndFormats.cpp':'No Owner'
,'H323CsReqFormat.cpp':'No Owner'
,'XmlPrintLog.cpp':'No Owner'
,'SipCsReqFormat.cpp':'No Owner'
,'NonStandardFormat.cpp':'No Owner'
,'GkCommonParamsFormat.cpp':'No Owner'
,'GkIfMsgsPhFormat.cpp':'No Owner'
,'GkCsIndFormat.cpp':'No Owner'
,'FastIndexTable.cpp':'No Owner'
,'Parser.cpp':'No Owner'
,'XmlBuilder.cpp':'No Owner'
,'H323CsIndFormat.cpp':'No Owner'
,'H460_1Format.cpp':'No Owner'
,'AcServiceReqFormats.cpp':'No Owner'
,'ConferenceMCMFormat.cpp':'No Owner'
,'CommonFormats.cpp':'No Owner'
,'CallTaskMsgPhFormat.cpp':'No Owner'
,'CapabilitiesFormat.cpp':'No Owner'
,'Convertor.cpp':'No Owner'
,'GkCsReqFormat.cpp':'No Owner'
,'RoundTripDelayFormat.cpp':'No Owner'
,'AcStartupReqFormats.cpp':'No Owner'
,'CapH263AnnexesFormat.cpp':'No Owner'
}




        
## ---------------------- Parse Report File ------------------------

class ParserReport:
    def __init__(self,fileName):
        # now open a file for reading
        # replace filename with the path to the file you created in pickletest.py
        #unpicklefile = open('/home/udibb/Carmel/ClearCase/udibb_newRoot_MCMS-Carmel-V1.0/vob/MCMS/Main/AssertFilesOwners', 'r')
        # now load the list that we pickled into a new object
        # close the file, just for safety
        #unpicklefile.close()
        self.m_assertFileOwnerTable = dict()
        self.m_fileName = fileName
        self.m_userPrintInd = dict()
        self.m_ownerToMsgTable = dict()
        self.m_userMails = {'Udi':'udib'
                            ,"Haggai":'haggai.gelernter'
                            ,"Ron L.":'ron.lazarovich'
                            ,"Ori P.":'ori.pugatzky'
                            ,"Romem":'romem.shach'
                            ,"Yoella":'yoella.bourshan'
                            ,"Kobi":'kobi.ginon'
                            ,"Sagi":'sagi.amiel'
                            ,"Dovev":'dovev.liberman'
                            ,"Shmulik Y.":'Shmulik.Yudkovich'
                            ,"Uri A.":'uri.avni'
                            ,"Keren G.":'keren.gratziany'
                            ,"Inga":'inga.lavi'
                            ,"Eran D.":'eran.decker'
                            ,"Michael V.":'michael.volovik'
                            ,"Olga S. ":'olga.shchukin'
                            ,"David":'david.rabkin'
                            ,"Yehudit":'judith.shuva-maman'
                            ,"Carmit":'carmit.lavi'
                            }
        
    def GetDataPerOwner(self):
        try:
            reportFd = open( self.m_fileName)
        except IOError:
            print "Error:Problem opening file: " + self.m_fileName
            return
        
        line = reportFd.readline() 
        while line:
            if ( line .find('Exception List Statistics') != -1):
                for x in range(6):#Reading The first 5 empty lines
                    reportFd.readline()
                self.ReadAssertedFilesOwners(reportFd)
            if ( line .find('owners for failed scripts') != -1):
                reportFd.readline() #Reading The first empty line
                self.ReadScriptsOwners(reportFd) #Read all the owners failed scripts data
            
            if (line .find('owners for core files') != -1):
                line = reportFd.readline() #Reading The first empty line
                self.ReadCoresOwners(reportFd) 

            line = reportFd.readline()

        reportFd.close()

    def AddWelcomeMessage(self,userName,table):
        table[userName] = 'Hi '+ userName + ',\n\nPlease check the following rejects we received on the  night report\n\n'
        #Setting the user data he already recivied
        self.m_userPrintInd[userName] = dict()
        self.m_userPrintInd[userName][0] = False
        self.m_userPrintInd[userName][1] = False
        self.m_userPrintInd[userName][2] = False
        
    def ReadAssertedFilesOwners(self,reportFd):
        print "Reading the owners of the aseerted files"
        line = reportFd.readline()
        while line:
            if (line.isspace()):
                return
            for x in range(len(line)):
                if line[x].isalpha():
                    line = line[x:]
                    arr = line.split(':')
                    if(len(arr) < 2):
                        return
                    fileName = arr[0]
                    fileLine = arr[1]
                    owner = ''
                    if (fileOwners.has_key(fileName)):
                        owner = fileOwners[fileName]
                    if (owner == '' or 'No Owner' == owner):
                            owner = 'Eran D.' 
                    
                    #print "!!!!!!! fileName = " + fileName + " ,line = "+ fileLine+ ",owner="+owner
                    #Add the message to the user
                    if (self.m_ownerToMsgTable.has_key(owner) == False ):
                        self.AddWelcomeMessage(owner ,self.m_ownerToMsgTable)

                    #First msg
                    if (self.m_userPrintInd[owner][0] == False):
                        self.m_ownerToMsgTable[owner] += '\n\nWe Found the following asserted files under your name:\n\n'

                    self.m_userPrintInd[owner][0] = True    
                    self.m_ownerToMsgTable[owner]+=line
                    break
            reportFd.readline() #Read the space line
            line = reportFd.readline() 
                    
    def ReadScriptsOwners(self,reportFd):
        print "Found Failed Scripts owner"
        line = reportFd.readline()
        while line:
            if (line.isspace()):
                return
            line = line.lstrip()
            #read all '!' if exists
            if line[0] == '!' :
                line = line[4:]
            userName = line.split(' ')[0]
            line = line[len(userName):]
            if (line[2] == '.' ):
                userName+= line[0:3]
                line = line[3:]
            line = line.lstrip() #Remove spaces fro begining
            scriptName=line.split(' ')[0]
            #print "User: " + userName + " has a failed script: " + scriptName
            if (self.m_ownerToMsgTable.has_key(userName) == False ):
                self.AddWelcomeMessage(userName,self.m_ownerToMsgTable)
                
            #First msg
            if (self.m_userPrintInd[userName][1] == False):
                self.m_ownerToMsgTable[userName] += '\n\nWe Found the following failed scripts under your name:\n\n'
            self.m_userPrintInd[userName][1] = True
            
            self.m_ownerToMsgTable[userName] += line 
            line = reportFd.readline()
                
    def ReadCoresOwners(self,reportFd):
        print "Found Core dumps owners"
        line = reportFd.readline()
        while line:
            if (line.isspace() or line .find('Today\'s Log') != -1 ):
                return
            line=line.lstrip() #Remove spaces from begining
            userName = line.split(' ')[0]
            line = line[len(userName):]
            if (line[2] == '.' ):
                userName+= line[0:3]
                line = line[3:]
                
            line = line.lstrip() #Remove spaces from the begining
            if (self.m_ownerToMsgTable.has_key(userName) == False ):
                self.AddWelcomeMessage(userName,self.m_ownerToMsgTable)
                
            #First msg
            if (self.m_userPrintInd[userName][2] == False):
                self.m_ownerToMsgTable[userName] += '\nWe found the following Core dumps under your name:\n\n'
            self.m_userPrintInd[userName][2] = True
            
            self.m_ownerToMsgTable[userName]+= line
            line = reportFd.readline()

   
    def SendMailToOwners(self):
        # sender = MailSender('mail.polycom.co.il')
        # sender = MailSender('isrexch01.israel.polycom.com')
        sender = MailSender('ISRMailprd01.polycom.com')

        for user,msg in self.m_ownerToMsgTable.iteritems():
            #Get the user mail according to the table
            if self.m_userMails.has_key(user):
                msg += '\nThanks,\nCarmel Team'
                #Send The mail to the owners
                print "Sending mail to : " + self.m_userMails[user] + "\n\n" + msg + "\n\n"
                sender.SendMail('Carmel Night Test',self.m_userMails[user],'Check out the following issues',msg)
            else:
                sender.SendMail('Carmel Night Test','david.rabkin','Can not find user',"Can not find the user=" + user)
        
## ------------------------------------------------------------------
        
    
if __name__ == '__main__':
    #Make sure that the day is not Friday(4) and not Saturday(5)
    
    if len(sys.argv) != 2:
        sys.exit("Usage: SendMailToOwners NightShortReportFile")
        
    parser = ParserReport(sys.argv[1])
    parser.GetDataPerOwner()
    parser.SendMailToOwners()
    
