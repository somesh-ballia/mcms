#!/mcms/python/bin/python
import os, string
import sys
import subprocess

from CsSimulationConfig import *
from CsSimulationConfig.SippTestClasses import *
from CsSimulationConfig.SippTestClasses.SippTestUtils import *

from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_1 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_2 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_3 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_4 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_5 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_6 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_7 import *
from CsSimulationConfig.SippTestClasses.TruncateSipDomainTest_8 import *
from CsSimulationConfig.SippTestClasses.SdesNegotiatedParamsTest_1 import *


sippTestUtils = SippTestUtils()

#### CALL YOUR TEST CLASSES HERE -->-->-->

TruncateSipDomainTest_1(sippTestUtils, "Truncate SIP domain from site name #1","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_2(sippTestUtils, "Truncate SIP domain from site name #2","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_3(sippTestUtils, "Truncate SIP domain from site name #3","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_4(sippTestUtils, "Truncate SIP domain from site name #4","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_5(sippTestUtils, "Truncate SIP domain from site name #5","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_6(sippTestUtils, "Truncate SIP domain from site name #6","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_7(sippTestUtils, "Truncate SIP domain from site name #7","Arbel.Moshe@polycom.co.il")
TruncateSipDomainTest_8(sippTestUtils, "Truncate SIP domain from site name #8","Arbel.Moshe@polycom.co.il")
SdesNegotiatedParamsTest_1(sippTestUtils, "SdesNegotiatedParamsTest_1","Arbel.Moshe@polycom.co.il")
