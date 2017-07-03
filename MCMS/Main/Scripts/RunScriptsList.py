#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh

import os


from McmsConnection import *


def WriteAndPrint(file,text):
	file.write(text)
	file.write("\n")
	print text
	

if __name__ == '__main__':
	
	c = McmsConnection()

	# check if "tests file" parameter receive
	if len(sys.argv) == 1:
		print "tests xml file is missing"
		exit(11)
		
	xml_test_file=sys.argv[1]	
	print "tests xml file is: " + xml_test_file
	print

	# check if "tests file" is in .xml format
	is_xml = str(xml_test_file).find(".xml")
	if is_xml == -1:
		print "tests file is not .xml file"
		exit(12)
	
	c.LoadXmlFile(xml_test_file)
	test_list = c.loadedXml.getElementsByTagName("TEST_RUN")
	if len(test_list) == 0:
		print "tests file not containing tests"
		exit(13)

	results_folder = ""
	if len(sys.argv) > 2:
		results_folder = sys.argv[2]
		if os.path.lexists(results_folder):
			print "results_folder: " + results_folder
		else:
			print "results_folder " + results_folder + "doe's not exists, using TestResults instead"
			results_folder = "TestResults"
			
	print

	XmlFileName = "ScriptsListLogFile"
	xml_words_list = xml_test_file.split('/')
	print xml_words_list
	for word in xml_words_list:
		if word.find(".xml"):
			XmlFileName = word.split('.')[0]
			print XmlFileName

	ScriptsListLogFileName = results_folder + '/' +  XmlFileName +".log"
	ListLogFile = open(ScriptsListLogFileName,'w')


	system_procecess_list =["Authentication", "McuMngr", "Cards", "Resource", "MplApi", "CSApi", "CSMngr", "GideonSim", 
           "EndpointsSim", "ConfParty", "QAAPI", "CDR", "SipProxy","Ice", "DNSAgent", "Faults", "Gatekeeper",
           "Logger", "Configurator", "EncryptionKeyServer", "CertMngr"]

	running_commands_list = []
	results_list = []

	for test in test_list:
		## 1) test name
		WriteAndPrint(ListLogFile,"=========================================================================================================")
		test_name = test.getElementsByTagName("TEST_NAME")[0].firstChild.data
		WriteAndPrint(ListLogFile,"test name is: " + test_name)
		WriteAndPrint(ListLogFile,"---------------------------------------------------------------------------------------------------------")
		## 2) test configuration files
		##    pars all configuration files from xml and export them
		is_test_cofiguration = test.getElementsByTagName("TEST_CONFIGURATION").length
		if is_test_cofiguration:
			WriteAndPrint(ListLogFile,"using cofiguration files:")
			private_mpl_sim = ""
			private_mpl_sim = test.getElementsByTagName("TEST_CONFIGURATION").item(0).getElementsByTagName("MPL_SIM_FILE")[0].firstChild.data
			if private_mpl_sim != "":
				if private_mpl_sim == "Auto":
					WriteAndPrint(ListLogFile,"using existing MPL_SIM_FILE")
				else:
					WriteAndPrint(ListLogFile,"using MPL_SIM_FILE: " + private_mpl_sim)
					os.putenv("MPL_SIM_FILE",private_mpl_sim)

			private_license = ""	
			private_license = test.getElementsByTagName("TEST_CONFIGURATION").item(0).getElementsByTagName("LICENSE_FILE")[0].firstChild.data
			if private_license != "":
				if private_license == "Auto":
					WriteAndPrint(ListLogFile,"using existing LICENSE_FILE")
				else:
					WriteAndPrint("using LICENSE_FILE: " + private_license)
					os.putenv("LICENSE_FILE",private_license)

			private_resource_setting = ""
			private_resource_setting = test.getElementsByTagName("TEST_CONFIGURATION").item(0).getElementsByTagName("RESOURCE_SETTING_FILE")[0].firstChild.data
			if private_resource_setting != "":
				if private_resource_setting == "Auto":
					WriteAndPrint(ListLogFile,"using existing RESOURCE_SETTING_FILE")
				else:
					WriteAndPrint(ListLogFile,"using RESOURCE_SETTING_FILE: " + private_resource_setting)
					os.putenv("RESOURCE_SETTING_FILE",private_resource_setting)

			private_system_cards_mode = ""
			private_system_cards_mode = test.getElementsByTagName("TEST_CONFIGURATION").item(0).getElementsByTagName("SYSTEM_CARDS_MODE_FILE")[0].firstChild.data
			if private_system_cards_mode != "":
				if private_system_cards_mode == "Auto":
					WriteAndPrint(ListLogFile,"using existing SYSTEM_CARDS_MODE_FILE")
				else:
					WriteAndPrint(ListLogFile,"using SYSTEM_CARDS_MODE_FILE: " + private_system_cards_mode)
					os.putenv("SYSTEM_CARDS_MODE_FILE",private_system_cards_mode)

			private_ip_service = ""
			private_ip_service = test.getElementsByTagName("TEST_CONFIGURATION").item(0).getElementsByTagName("USE_ALT_IP_SERVICE")[0].firstChild.data
			if private_ip_service != "":
				if private_ip_service == "Auto":
					WriteAndPrint(ListLogFile,"using existing IP_SERVICE_FILE")
				else:
					WriteAndPrint(ListLogFile,"using USE_ALT_IP_SERVICE: " + private_ip_service)
					os.putenv("USE_ALT_IP_SERVICE",private_ip_service)

		else:
			WriteAndPrint(ListLogFile,"no specific configuration files - using default")

		WriteAndPrint(ListLogFile,"---------------------------------------------------------------------------------------------------------")			

		test_valgrind_list = []
		is_test_valgrind = test.getElementsByTagName("TEST_VALGRIND_LIST").length
		if is_test_valgrind:
			WriteAndPrint(ListLogFile,"processses that will run under valgrind for all scripts:")
			test_valgrind_processes_list = test.getElementsByTagName("TEST_VALGRIND_LIST").item(0).getElementsByTagName("TEST_VALGRIND_PROCESS")
			if len(test_valgrind_processes_list) == 0:
				WriteAndPrint(ListLogFile,"empty TEST_VALGRIND_LIST")
			for test_process in test_valgrind_processes_list:
				test_process_name = test_process.firstChild.data
				if test_process_name == "NoValgrind":
					WriteAndPrint(ListLogFile,"test_process_name NoValgrind - do nothing")
				elif test_process_name == "All":
					WriteAndPrint(ListLogFile,"test_process_name All - add all processes")
					for system_process in system_procecess_list:
						test_valgrind_list.append(system_process)
				else:				
					WriteAndPrint(ListLogFile,"test process under valgrind: " + test_process_name)
					test_valgrind_list.append(test_process_name)
			if len(test_valgrind_list)!=0:
				WriteAndPrint(ListLogFile,"")
				WriteAndPrint(ListLogFile,"processes list:")
				for process in test_valgrind_list:
					WriteAndPrint(ListLogFile,process)

		else:
			WriteAndPrint(ListLogFile,"processs that will run under valgrind for all scripts - empty")
		
		WriteAndPrint(ListLogFile,"---------------------------------------------------------------------------------------------------------")		
		## 4) pars scripts
		##    pars scripts names, and valgrind processes per script, add the exexcuting commands to running_commands_list
		WriteAndPrint(ListLogFile,"scripts list:")
		scripts_list = test.getElementsByTagName("SCRIPT_RUN")
		for script in scripts_list:
			script_name = script.getElementsByTagName("SCRIPT_NAME")[0].firstChild.data
			script_run_name = script_name +'.py'
			WriteAndPrint(ListLogFile,"script name: " + script_name)
			valgrind_processes_list = script.getElementsByTagName("VALGRIND_PROCESS")
			if len(valgrind_processes_list) == 0:
				WriteAndPrint(ListLogFile,"running command: " + script_run_name)
				running_commands_list.append(script_run_name)
			for process in valgrind_processes_list:
				process_name = process.firstChild.data
				if process_name == "NoValgrind":
					WriteAndPrint(ListLogFile,"running command: " + script_run_name)
					running_commands_list.append(script_run_name)
				elif process_name == "All":
					WriteAndPrint(ListLogFile,"process_name == All")
					for system_process in system_procecess_list:
						script_velgrind_run_name = script_run_name + ' ' + system_process						
						WriteAndPrint(ListLogFile,"running command: " + script_velgrind_run_name)
						if script_velgrind_run_name not in running_commands_list:
							running_commands_list.append(script_velgrind_run_name)
				else:				
					WriteAndPrint(ListLogFile,"process under valgrind: " + process_name)
					script_velgrind_run_name = script_run_name + ' ' + process_name
					WriteAndPrint(ListLogFile,"running command: " + script_velgrind_run_name)
					if script_velgrind_run_name not in running_commands_list:
						running_commands_list.append(script_velgrind_run_name)

			if len(test_valgrind_list) > 0:		
				for all_test_process in test_valgrind_list:
					WriteAndPrint(ListLogFile,"test process under valgrind: " + all_test_process)
					script_valgrind_run_name = script_run_name + ' ' + all_test_process
					WriteAndPrint(ListLogFile,"running command: " + script_valgrind_run_name)
					if script_valgrind_run_name not in running_commands_list:
						running_commands_list.append(script_valgrind_run_name)
			WriteAndPrint(ListLogFile,"")


	WriteAndPrint(ListLogFile,"---------------------------------------------------------------------------------------------------------")		
	WriteAndPrint(ListLogFile,"running commands list:")
	for command in running_commands_list:
		WriteAndPrint(ListLogFile,command)
	WriteAndPrint(ListLogFile,"---------------------------------------------------------------------------------------------------------")
	WriteAndPrint(ListLogFile,"running scripts:")
	for command in running_commands_list:
		WriteAndPrint(ListLogFile,"running script: " + command)
		cmd_line = "Scripts/RunTestFromList.sh " + "Scripts/" + command + " " + results_folder
		script_status = os.system(cmd_line)
		script_status = script_status/256
		result_line = command + ": "
		if script_status == 0:
			result_line = result_line + "passed"
		elif script_status == 102:
			result_line = result_line + "startup failed"
		elif script_status == 101:
			result_line = result_line + "failed"
		else:
			result_line = result_line + "failed with status " + str(script_status)
		results_list.append(result_line)
	WriteAndPrint(ListLogFile,"---------------------------------------------------------------------------------------------------------")
	WriteAndPrint(ListLogFile,"printing results:")
	for result in results_list:
		WriteAndPrint(ListLogFile,result)
	WriteAndPrint(ListLogFile,"=========================================================================================================")
	ListLogFile.close()
