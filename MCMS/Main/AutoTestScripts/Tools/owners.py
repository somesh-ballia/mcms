#! /usr/bin/env python



import pickle

import os, fnmatch
import re

from time import gmtime, strftime

filename='testing.txt'
reportname='short_report.txt'
ScriptOwners={
#__BEGIN_TESTS__
"SetPartyPivateLayout":"Amir.Kaplan",
"GW_1":"Amir.Kaplan",
"GW_2":"Amir.Kaplan",
"ResourceMPMPlusResCapacity":"Amir.Kaplan",
"ResourceMPMPlusSwitchModesDynamicly":"Amir.Kaplan",
"ResourceISDNDirectDialIn":"Amir.Kaplan",
"ResourceMPMPlusPartiallyAssembly20":"Amir.Kaplan",
"ResourceMPMPlusPartiallyAssembly40":"Amir.Kaplan",
"ResourceMPMPlusAllocationPriorities":"Amir.Kaplan",
"ResourceMPMPlusPartyTypes":"Amir.Kaplan",
"ResourceMPMAutoMode":"Amir.Kaplan",
"ResourceMPMAuto_ResMinParticipants":"Amir.Kaplan",
"ResourceRepeatedPatterns":"Amir.Kaplan",
"ResourceMPMPlusAuto_ResMinParticipants":"Amir.Kaplan",
"ResourceMPMPlusFixed_ResMinParticipants":"Amir.Kaplan",
"ResourceHotSwap":"Amir.Kaplan",
"ResourceHotSwapAA_MPMPlus":"Amir.Kaplan",
"ResourceHotSwapAA_MPM":"Amir.Kaplan", 
"ResourceMPMPlusAuto_ResMinParticipants":"Amir.Kaplan",
"MAX_CP_RESOLUTIONFlagInMPMPlusBased":"Amir.Kaplan",
"ResourceMPMPlusAutoMode":"Amir.Kaplan",
"ResourceMPMPlusFixedMode":"Amir.Kaplan", 
"ResourceMPMPlusAdvanced":"Amir.Kaplan", 
"ResourceMPMPlusAdvanced2":"Amir.Kaplan",
"Resource2000RepeatedRes":"Amir.Kaplan",
"ResourceNoScheduler":"Amir.Kaplan",
"ResourceReadresDB":"Amir.Kaplan",
"ResourceRepeatedRes":"Amir.Kaplan",
"ResourceReservation":"Amir.Kaplan",
"AddRemoveConfTemplateNew":"Amir.Kaplan",
"AddRemoveResNew":"Amir.Kaplan",
"H264_H263_Content":"Shira.Shafrir",
"HD":"Amir.Kaplan",
"Hd1080Vsw":"Amir.Kaplan",
"MixVideoModesMpmPlusBased":"Amir.Kaplan",
"MixVideoProtocolsMpmPlusBased":"Amir.Kaplan",
"RcrsDeficiencySD30MpmPlusBased":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxMixParticipantsTest":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxVideoParticipantsTest":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxVideoPartiesInConf":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxVoipParticipantsTest":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxVoipPartiesInConf":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxConfrencesTest":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxEQsTest":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxProfilesTest":"Amir.Kaplan",
"RmxMpmPlus_ConfParty_MaxSipFactoriesTest":"Amir.Kaplan",
"DecisionMatrixConfsInMPMPlusBased":"Amir.Kaplan",
"LprModeChange":"Shira.Shafrir",
"LprModeChange1":"Shira.Shafrir",
"ISDN_DirectDialIn":"Amir.Kaplan",
"ISDN_DialInDialOut":"Amir.Kaplan",
"ISDN_MultiRate":"Amir.Kaplan",
"ISDN_MaxCapacity":"Amir.Kaplan",
"ISDN_ReconnectParty":"Amir.Kaplan",
"ISDN_ConfVideoLayout":"Amir.Kaplan",
"ISDN_DialOut":"Amir.Kaplan",
"ISDN_MultyTypeConf":"Amir.Kaplan",
"ISDN_Move":"Amir.Kaplan",
"ISDNConf":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxConferenceTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxEQsTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxMixParticipantsTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxProfilesTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxSipFactoriesTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxVideoParticipantsTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxVideoPartiesInConf":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxVoipParticipantsTest":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxVoipPartiesInConf":"Amir.Kaplan",
"RmxMpmRx_ConfParty_MaxMRsTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxConferenceTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxEQsTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxMixParticipantsTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxProfilesTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxSipFactoriesTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxVideoParticipantsTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxVoipParticipantsTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxVoipPartiesInConf":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxMRsTest":"Amir.Kaplan",
"RmxMpmx_ConfParty_MaxVideoPartiesInConf":"Amir.Kaplan",
"CreateCertificate":"Koby.Ginon",
"TLSCommunicationMpmPlus":"Koby.Ginon",
"TLSCommunicationMpm":"Koby.Ginon",
"ExternalDB_TLSSimulation":"Kobi.Ginon",
"SysConfigSetWrongEunm":"avid.Rabkin",
"SysConfigSetWrongNumberRange":"Kobi.Ginon",
"SysConfigSetWrongVal":"Kobi.Ginon",
"SysConfigSetMain":"Kobi.Ginon",
"HD_force_party_level":"Amir.Kaplan",
"TestAuditFull":"Kobi.Ginon",
"HotSwapUtils":"Kobi.Ginon",      
"AutoRealVoip":"Amir.Kaplan",
"AutoRealVideo":"Amir.Kaplan",      
"AddDeleteIpService":"Kobi.Ginon",
"AddRemoveProfile":"Amir.Kaplan",
"AddRemoveMrNew":"Amir.Kaplan",
"AddRemoveOperator":"Kobi.Ginon",       
"VideoConfWith40Participants":"Amir.Kaplan",
"Add20ConferenceNew":"Amir.Kaplan",      
"GetCardsMonitor":"Kobi.Ginon",      
"GetOperatorsMonitor":"Kobi.Ginon",      
"Create5ConfWith3Participants":"Amir.Kaplan",       
"CreateCPConfWith4DialInParticipants":"Amir.Kaplan",
"ReconnectParty":"Amir.Kaplan",      
"ConfVideoLayout":"Amir.Kaplan",
"AwakeMrByUndef":"Amir.Kaplan", 
"CreateConfWith3SipParticipants":"Shira.Shafrir",  
"AutoLayout":"Amir.Kaplan",      
"PersonalLayout":"Amir.Kaplan",      
"SameLayout":"Amir.Kaplan",       
"PresentationMode":"Amir.Kaplan",
"SpeakerChange":"Amir.Kaplan",
"EncryConf":"Amir.Kaplan",        
"AddDeleteNewIvr":"Amir.Kaplan",
"AddDeleteEqService":"Amir.Kaplan",      
"ResourceFullCapacity":"Amir.Kaplan",
"Resource2Undef3RealParties":"Amir.Kaplan",      
"FECCTokenTest":"Amir.Kaplan",
"SocketDisconnectConnect":"Kobi.Ginon",
"SimMfaComponentFatal":"Kobi.Ginon",   
"AddConfMinParticipantsOverLimit":"Amir.Kaplan",        
"AddRemoveConfWrongNumericID":"Amir.Kaplan",      
"CreateAdHoc3BlastUndefParties":"Amir.Kaplan", 
"SipDialInToMR":"Shira.Shafrir",
"ResourceConfsFullCapacity":"Amir.Kaplan", 
"UpdateParty":"Amir.Kaplan", 
"ResourceMultMoveUndef":"Amir.Kaplan",
"ResourceDetailParty":"Amir.Kaplan",        
"Capacity40UndefinedDialIn":"Amir.Kaplan",
"VideoConfWith20Participants":"Amir.Kaplan",
"UndefinedDialIn":"Amir.Kaplan",
"ForceVideo":"Amir.Kaplan",
"MuteVideo":"Amir.Kaplan",
"LectureModes":"Amir.Kaplan",
"VisualEffects":"Amir.Kaplan",
"ShortSocketDisconnectConnect":"Kobi.Ginon",
"AddRemoveConfWrongMonitorID":"Amir.Kaplan",     
"IPPartyMonitoring":"Shira.Shafrir",
"EncryDialIn":"Amir.Kaplan",
"ResourceUnitDetail":"Amir.Kaplan",
"ResourceDisable":"Amir.Kaplan",
"EncrypValidity":"Amir.Kaplan",
"LobbyRejectDialIn":"Amir.Kaplan",
"AddRemovePartyConfMissing":"Amir.Kaplan",
"CheckMultiTypesOfCalls":"Amir.Kaplan",
"CheckTypesOfDialInCalls":"Shira.Shafrir",
"AwakeMRWithDefinedDialIn":"Amir.Kaplan",
"AwakeMRWithDefinedSipDialIn":"Amir.Kaplan",
"AwakeMrByUndefViaEntryQ":"Amir.Kaplan",
"CS_CardSocketDisconnect":"Kobi.Ginon",
"IsActiveAlarmEmpty":"Kobi.Ginon",
#"LongConnDisc":"Amir.Kaplan",
"MFA_CardSocketDisconnectReconnect":"Kobi.Ginon",
"MfaComponentFatal":"Kobi.Ginon",
"PLC":"Amir.Kaplan",
"SM_test":"Kobi.Ginon",
"ShortNoConnectionWithCS_Card":"Kobi.Ginon",
"ShortNoConnectionWithMFA_Card":"Kobi.Ginon",
"TestActionTag":"Kobi.Ginon",
"TU10UserChangePwToOtherUser":"Kobi.Ginon",
"TU11ConnectUserWithWrongPassword":"Kobi.Ginon",
"TU1CheckAddedUsersInListAndConnectThem":"Kobi.Ginon",
"TU2UserAddUser":"Kobi.Ginon",
"TU3ExceedUsersListsMaxNum":"Kobi.Ginon",
"TU4ReconnectDeletedUser":"Kobi.Ginon",
"TU5_UserDelUser":"Kobi.Ginon",
"TU6DelLastAdmin":"Kobi.Ginon",
"TU7ReconnectUserWithOldPwAfterChangePw":"Kobi.Ginon",
"TU8ReconnectUserWithNewPwAfterChangePw":"Kobi.Ginon",
"TU9Add2SameNameUsers":"Kobi.Ginon",
"UndefSipInToEQ":"Amir.Kaplan",
"TestAutoExtend":"Amir.Kaplan",
"TestConfExist":"Amir.Kaplan",
"TestDefaultRestore":"Amir.Kaplan",
"AddDelete38IvrServices":"Amir.Kaplan",
"DefaultIvrServices":"Amir.Kaplan",
"IvrChangeToChairAndChangePWs":"Amir.Kaplan",
"IvrEntranceFeatures":"Amir.Kaplan",
"UsersPermission":"Kobi.Ginon",
"Ntp_Del_SystemTimeFile":"Kobi.Ginon",
"ExternalDB_Simulation":"Kobi.Ginon",
"TU12CheckAddedUsersLongNamesAndWeirdChars":"Kobi.Ginon",
"TU13Users_CaseSensitive":"Kobi.Ginon",
"TU14CheckUsersDB-AfterReset":"Kobi.Ginon",
"ExternalDB":"Kobi.Ginon",
"addH323PartyWithH263Cap":"Shira.Shafrir",
"ExternalDbOperations":"Kobi.Ginon",
"ContentHighestCommon":"Amir.Kaplan",
"ContentPresentation":"Amir.Kaplan",
"RCVR1_Test_manager_process_recovery_policy":"Kobi.Ginon",
"RCVR2_Test_Manager_task_recovery_policy":"Kobi.Ginon",
"RCVR3_Test_delay_between_failures_in_process_recovery_policy":"Kobi.Ginon",
"RCVR4_Test_delay_between_failures_in_task_recovery_policy":"Kobi.Ginon",
"AddUpdateDeleteIpService":"Kobi.Ginon",
"TestSysCfg":"Kobi.Ginon",
"CSStartupWithoutCSTest":"Kobi.Ginon", 
"CSStartupWithoutMFATest":"Kobi.Ginon", 
"RCVR1_1_Test_manager_process_recovery_policy":"Kobi.Ginon",
"RCVR1_2_Test_Manager_task_recovery_policy":"Kobi.Ginon",
"RCVR1_3_Test_delay_between_failures_in_process_recovery_policy":"Kobi.Ginon",
"RCVR5_Run_last_to_del_RCVR_core_dumps":"Kobi.Ginon",
"RCVR1_4_Test_delay_between_failures_in_task_recovery_policy":"Kobi.Ginon",
"RCVR5_Run_last_to_del_RCVR_c_o_r_e_dumps":"Kobi.Ginon",
"TestPreventFaultFlooding":"Kobi.Ginon",
"TestCDRGetLastId":"Kobi.Ginon",
"TestCDRMonitoring":"Kobi.Ginon",
"ShortNoConnectionWithSwitch_Card":"Kobi.Ginon",
"SDDecisionMatrixConfs":"Amir.Kaplan",
"SDFullCapaity":"Amir.Kaplan",
"SDMixVideoModes":"Amir.Kaplan",
"SDMixVideoProtocols":"Amir.Kaplan",
"SDResourcesTest":"Amir.Kaplan",
"SDMotionConfsUndefDialInFullCapaity":"Amir.Kaplan",
"SDSharpnesConfsUndefDialInFullCapaity":"Amir.Kaplan",
"TestSyncMsgMechanism":"Kobi.Ginon",
"UsersListCorrupted":"Kobi.Ginon",
"UsersListDefaultUserAlert":"Kobi.Ginon",
"CheckUnicodeDisplayName":"Amir.Kaplan",
"TestCDRDisplayName":"Kobi.Ginon",
"TestUnicodeCutUtf8Strings":"Kobi.Ginon",
"TestUnicodeSnmp":"Kobi.Ginon",
"TestUnicodeExAsciiFile":"Kobi.Ginon",
"TestUnicodeUnknownCharsets":"Kobi.Ginon",
"PSTNConf":"Amir.Kaplan",
"PSTN_Move":"Amir.Kaplan",
"Move":"Amir.Kaplan",
"RtmIsdnConfig_AddPhoneRange":"Kobi.Ginon",
"RtmIsdnConfig_AddService":"Kobi.Ginon",
"RtmIsdnConfig_SetDefaultService":"Kobi.Ginon",
"RtmIsdnConfig_SpanAttach":"Kobi.Ginon",
"RtmIsdnConfig_SpanDetach":"Kobi.Ginon",
"RtmIsdnConfig_UpdateService":"Kobi.Ginon",
"RtmIsdnConfig_DelPhoneRange":"Kobi.Ginon",
"RtmIsdnConfig_DelService":"Kobi.Ginon",
"Test_Snmp_3_redundant_community":"Kobi.Ginon",
"Test_Snmp_4_check_cotent":"Kobi.Ginon",
"Test_Snmp_7_Security":"Kobi.Ginon",
"Test_Snmp_6_disable_Trap":"Kobi.Ginon",
"Test_Snmp_11_Community__Name":"Kobi.Ginon",
"Test_Snmp_5_disable_Snmp":"Kobi.Ginon",
"Test_Snmp_10_GK_Address__Snmp":"Kobi.Ginon",
"Test_Snmp_8_Active_Alarm_Traps":"Kobi.Ginon",
"Test_Snmp_9_Active_Alarm_Traps_v2":"Kobi.Ginon",
"Test_Snmp_13_Cold__Startup":"Kobi.Ginon",
"Test_Snmp_12_Snmpd__recovery":"Kobi.Ginon",
"Test_Snmp_14_RMX__Interfaces":"Kobi.Ginon",
"Unidirection":"Shira.Shafrir",
"ConfWithDifferentSARParticipants":"Amir.Kaplan",
"UsersListNoFile":"Amir.Kaplan",
"CheckGarbageCollector":"Koby.Ginon",
"Faults_Test_1_full_faults_only":"Kobi.Ginon",
"Faults_Test_2_is_both_faults_exist_in_the_2_lists":"Kobi.Ginon",
"Faults_Test_4_is_old_short_faults_exist":"Kobi.Ginon",
"Faults_Test_5_cyclic_faults_after_startup":"Kobi.Ginon",
"HDCPConfs":"Amir.Kaplan",
"HD_connect_disconnect":"Amir.Kaplan",
"HD_force":"Amir.Kaplan",
"HD_lecture_mode":"Amir.Kaplan",
"HD_speaker_change":"Amir.Kaplan",
"HD_switch_in_switch":"Amir.Kaplan",
"TestAddAlert":"Kobi.Ginon",
"TestNeatShutdown":"Kobi.Ginon",
"TestResetHistory":"Kobi.Ginon",
"PSTN_1":"Kobi.Ginon",
"PSTN_2":"Kobi.Ginon",
"PSTN_3":"Kobi.Ginon",
"PSTN_4":"Kobi.Ginon",
"PSTN_5":"Kobi.Ginon",
"PSTN_6":"Kobi.Ginon",
"Cop4CIF16X9ChangeSpeaker":"Amir.Kaplan",
"Cop4CIF16x9ChangeConfLayout":"Amir.Kaplan",
"Cop4CIF4x3ChangeConfLayout":"Amir.Kaplan",
"Cop4CIF4x3ChangeSpeaker":"Amir.Kaplan",
"CopHD1080ChangeConfLayout":"Amir.Kaplan",
"CopHD1080ChangeSpeaker":"Amir.Kaplan",
"CopHD720ChangeConfLayout":"Amir.Kaplan",
"CopHD720ChangeSpeaker":"Amir.Kaplan",
"CopHD1080LectureMode":"Amir.Kaplan",
"MxN_AllProfiles":"Amir.Kaplan",
"UndefSipInToEQ":"Amir.Kaplan",
"EncryDialIn":"Amir.Kaplan",
"EncrypValidity":"Amir.Kaplan",
"AwakeMRWithDefinedDialIn":"Amir.Kaplan",
"AwakeMRWithDefinedSipDialIn":"Amir.Kaplan",
"AwakeMrByUndef":"Amir.Kaplan",
"UndefinedSIPDialIn":"Shira.Shafrir",
"EncryConf":"Amir.Kaplan",
"AwakeMrByUndefViaEntryQ":"Amir.Kaplan",
"ConnectPartiesToVSWConfViaEQ":"Amir.Kaplan",
"TestMIBFile":"Kobi.Ginon",
"AutoRealVideoDialInMixedV4V6":"Shira.Shafrir",
"AutoRealVideoV6":"Shira.Shafrir",
"MixAutoRealVideo":"Shira.Shafrir",
"SipAutoRealVideo":"Shira.Shafrir",
"SipAutoRealVoip":"Shira.Shafrir",
"ChangeExchangeModuleCfg":"Amir.Kaplan",
"AutoScan":"Amir.Kaplan",
"ResourceSharedRsrcList":"Amir.Kaplan",
"ResourceVideoPreview":"Amir.Kaplan",
"AddSipBfcpDialIn":"Amir.Kaplan",
"FooBar":"Kobi.Ginon",
"FooBar2":"Kobi.Ginon",
"FooBar3":"Kobi.Ginon",
"ResourceResolutionSlider":"Amir.Kaplan",
"UpdateProfile":"Amir.Kaplan",
"CheckLayoutInRoomSwitchMode":"Amir.Kaplan",
"AddDeleteFactoryWithSipParty":"Amir.Kaplan",
"H264_H263_Content":"Shira.Shafrir",
"LprModeChange1":"Shira.Shafrir",
"Unidirection":"Shira.Shafrir",
"SipMuteAudioDirection":"Shira.Shafrir",
"SipMuteAudioinCpConf":"Shira.Shafrir",
"SipMuteAudio":"Shira.Shafrir",
"SipMuteAudioInactive":"Shira.Shafrir",
"SipMuteAudioPort":"Shira.Shafrir",  
"SipMuteVideoPort":"Shira.Shafrir",
"InstallTest":"Kobi.Ginon",
"BackupAndRestore":"Kobi.Ginon",
"TestBigCDRFileRetrieve":"Kobi.Ginon",
"TestMaxOpenHttpSockets":"Kobi.Ginon",
"AllUnicodeTests":"Kobi.Ginon",
"DowngradeToV7_8":"Kobi.Ginon",
"TestExchangeModuleCfg":"Kobi.Ginon",
"SoftMcuStartup":"Ori.Pugatzky",
"SoftMcuMcuTypesEdge":"Ori.Pugatzky",
"SoftMcuMcuTypesMFW":"Ori.Pugatzky",
"TestRestApi":"Ori.Pugatzky",
"TestRestSnmpMfw":"Ori.Pugatzky",
"cs_simple_unit_tests":"Shira.Shafrir",
"TestLdapModule":"Kobi.Ginon",
"TestCdrService":"Kobi.Ginon",
"CheckLprCpConf":"Shira.Shafrir",
#__END_TESTS__
}
# each string of test should be started from quote and finished by comma

ProcessOwners={
"Auditor":"Kobi.Ginon",
"ApacheModule":"Kobi.Ginon",
"Authentication":"Kobi.Ginon", 
"Cards":"Kobi.Ginon",
"CDR":"Kobi.Ginon", 
"ClientLogger":"Kobi.Ginon", 
"Configurator":"Kobi.Ginon",
"ConfParty":"Shira.Shafrir",
"CSApi":"Kobi.Ginon",
"CSMngr":"Kobi.Ginon",
"Demo":"Kobi.Ginon",
"DNSAgent":"Kobi.Ginon",
"EncryptionKeyServer":"Kobi.Ginon",
"EndpointsSim":"Kobi.Ginon",
"Faults":"Kobi.Ginon",
"Gatekeeper":"Shira.Shafrir",
"GideonSim":"Kobi.Ginon",
"Logger":"Kobi.Ginon",
"McmsDaemon":"Kobi.Ginon",
"McuCmd":"Kobi.Ginon",
"McuMngr":"Kobi.Ginon",
"MplApi":"Kobi.Ginon",
"QAAPI":"Amir.Kaplan",
"Resource":"Amir.Kaplan",
"SipProxy":"Shira.Shafrir", 
"TestClient":"Kobi.Ginon",
"CheckForcedPartiesOnConfMrTmplRsrv":"Kobi.Ginon",
}

#-----------------------------------------------------
def GetList():
 try:
	        file=open(filename, 'r')
	 	try:
			d1 = pickle.load(file)
			print "d1 = "
			print d1
		except EOFError:
	        	pass
	        file.close()
		
 except IOError:
	        pass
#-----------------------------------------------------

def WriteList():
	file=open(filename, 'w')
	pickle.dump(d, file)
	file.close()
	print "d = "
	print d

#-----------------------------------------------------
#-----------------------------------------------------
# main #####
#GetList()
repfile=open(reportname, 'r')
which_scripts = 0 # 0: didnt start script list, 1 - no valgrind scripts, 2 - valigrind scripts
first_line = 1
all_success = 1
for line in repfile:

	if "Functionality only - no valgrind" in line:
		which_scripts = 1
		first_line = 1			
	else:
		if "Failed Scripts when running under valgrind" in line:
			which_scripts = 2	
			first_line = 1		
	if which_scripts == 0:
		continue		 
	findScriptInLine = re.search('(\S*)', line)
 	if findScriptInLine:
		scriptInLine = findScriptInLine.group(1)
	else:
		#print "not findScriptInLine"
		continue
	#print "scriptInLine:" + scriptInLine + " Line " + line
	for script in ScriptOwners:
		if len(line) > len(script)+1:
			#print "script ... " + script + " compare to " + scriptInLine
			if (script==scriptInLine): 
						if ("---" in line) and ("error" in line):
							if (first_line):
								if which_scripts == 1: 
									print "* owners for failed scripts - no valgrind:"
								else:
									print "* owners for failed scripts - under valgrind:"
								first_line=0	
								all_success = 0
							print "!!  %-30s" %str(ScriptOwners[script]),
							print  line.strip()
							continue
						if ("---" in line) and ("error" not in line):
							if (first_line):
								if which_scripts == 1: 
									print "* owners for failed scripts - no valgrind:"
								else:
									print "* owners for failed scripts - under valgrind:"
								first_line=0	
								all_success = 0
							print "    %-30s" %str(ScriptOwners[script]),
							print  line.strip()
							continue
							
						if ("OK" not in line) and (".core" not in line):
					#for process in ProcessOwners:
					#	if process+" " in line:
					#		print "    %-30s" %str(ProcessOwners[process]),
					#		print  line.strip()
					#		continue
							if (first_line):
								if which_scripts == 1: 
									print "* owners for failed scripts -  no valgrind:"
								else:
									print "* owners for failed scripts - under valgrind:"
								first_line=0	
								all_success = 0
							print "    %-30s" %str(ScriptOwners[script]),
							print  line.strip()
							continue

if (all_success):
	#we didn't find any failure !!
	print "^ SUCCESS! clean run, no failed tests!"

repfile.close()
repfile=open(reportname, 'r')
first_line = 1
for line in repfile:
	if (".core." in line):
		for process in ProcessOwners:
			if process+".core" in line:
				if (first_line): 
					print "# owners for core files:"
					first_line=0
				print "    %-10s" %str(ProcessOwners[process]),
				print  line.strip()
               			continue

repfile.close()

current_day=strftime("%A", gmtime())
first_line=1

for path, subdirs, files in os.walk(current_day):
	subdirs.sort()
	for d in subdirs:
		if d not in ScriptOwners:
			if (first_line):
				print "# scripts without an owner:"
				first_line=0	
			print d
			#print "no owner for "+d
	break
