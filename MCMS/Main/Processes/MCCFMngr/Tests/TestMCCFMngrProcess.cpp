#include "TestMCCFMngrProcess.h"

///////////////////////////////////////////////////////////////////////////
#include "MccfPackagesRegistrar.h" // available MCCF packages

#include "MccfMsgFactory.h"        // MCCF transport messages
#include "MccfContext.h"           // MCCF transport protocol context

#include "DialogPrepare.h"

#include "MccfRxSocket.h"

///////////////////////////////////////////////////////////////////////////
#include "SystemFunctions.h"

///////////////////////////////////////////////////////////////////////////
#include "Trace.h"
#include "TraceStream.h"

//////////////////////////////////////////////////////////////////////
CPPUNIT_TEST_SUITE_REGISTRATION(CTestMCCFMngrProcess);

//////////////////////////////////////////////////////////////////////
void CTestMCCFMngrProcess::setUp()
{ CMccfPackagesRegistrar::RegisterAllPackages(); }

//////////////////////////////////////////////////////////////////////
void CTestMCCFMngrProcess::tearDown()
{}

//////////////////////////////////////////////////////////////////////
void CTestMCCFMngrProcess::testConstructor()
{
	FTRACEINTO;
	CPPUNIT_ASSERT_MESSAGE(PRETTY_FUNCTION, CProcessBase::GetProcess());
}

void CTestMCCFMngrProcess::testApiBaseObject()
{
	FTRACEINTO;
	CPPUNIT_ASSERT_MESSAGE(PRETTY_FUNCTION, CProcessBase::GetProcess());

#define VERIFY_TYPE(type, v) CPPUNIT_ASSERT(static_cast<TypeTraitsValueTagEnum>(ApiTypeHelper<type>::tag) == v);
#define VERIFY_SIZE(type, v) { ApiTypeHelper<type>::ValueType x; CPPUNIT_ASSERT((ApiTypeHelper<type>::CurrentBinarySize(x)) == v); }
#define VERIFY_SIZE_INIT(type, i, v) { ApiTypeHelper<type>::ValueType x(i); CPPUNIT_ASSERT((ApiTypeHelper<type>::CurrentBinarySize(x)) == v); }

	VERIFY_TYPE(bool, ttv_generic_type);

	VERIFY_TYPE(int, ttv_generic_type);
	VERIFY_TYPE(long long, ttv_generic_type);

	VERIFY_TYPE(byte, ttv_generic_type);
	VERIFY_TYPE(unsigned, ttv_generic_type);
	VERIFY_TYPE(unsigned long long, ttv_generic_type);

	VERIFY_TYPE(std::string, ttv_std_string);
	VERIFY_TYPE(const char*, ttv_const_char_pointer);

	VERIFY_TYPE(string_opt, ttv_api_type);

	VERIFY_TYPE(uint_opt, ttv_api_type);
	VERIFY_TYPE(ulong_opt, ttv_api_type);

	VERIFY_TYPE(ApiBaseObjectPtr, ttv_api_base_object_ptr);
	VERIFY_TYPE(ApiBaseObject, ttv_api_base_object_derived);
	VERIFY_TYPE(MscIvr, ttv_api_base_object_derived);

	VERIFY_SIZE_INIT(int, 7, sizeof(int));
	VERIFY_SIZE_INIT(int_opt, 7, sizeof(bool) + sizeof(int));
	VERIFY_SIZE_INIT(std::string, "abc", sizeof(size_t) + 3);
	VERIFY_SIZE_INIT(string_opt, "abc", sizeof(bool) + sizeof(size_t) + 3);

	VERIFY_SIZE(MscIvr, 3);

#undef VERIFY_TYPE
#undef VERIFY_SIZE
#undef VERIFY_SIZE_INIT
}

//////////////////////////////////////////////////////////////////////
void CTestMCCFMngrProcess::testSYNC()
{
	const char syncMsg[] =
		"CFW prxtid-mccf-stack%0 SYNC\0"
		"Packages: polycom-msc-conf/1.0\0"
		"Dialog-id: ccid-1323193616035-6330219227949700096\0"
		"Keep-alive: 120";

	CMccfMsgFactory& f = CMccfMsgFactory::instance();
	MccfErrorCodesEnum status = mec_OK;

	CMccfRxSocket rx;
	CMccfContext context(rx);

	HMccfMessage h(f.CreateHeader(context, &syncMsg[0], sizeof(syncMsg), status));
	CPPUNIT_ASSERT(HMccfMessage::manager().size() == 1);

	h.close();
	CPPUNIT_ASSERT(HMccfMessage::manager().size() == 0);
}

//////////////////////////////////////////////////////////////////////
void CTestMCCFMngrProcess::testIvrMessage()
{
	{
		const char xml_buf[] =
			"<mscivr version=\"1.0\" xmlns=\"urn:ietf:params:xml:ns:msc-ivr\" desclang=\"some description\">"
				"<dialogprepare>"
					"<dialog>"
						"<prompt>"
							"<media loc=\"https://10.47.17.233:8443/media/proximo-7.0.0.98914/prompts/adhoc-video-6.0.0.100271M-ddiaz/Conference_NID.wav\"/>"
							"<media loc=\"https://10.47.17.233:8443/media/proximo-7.0.0.98914/prompts/adhoc-video-6.0.0.100271M-ddiaz/Conference_2.wav\"/>"
						"</prompt>"
					"</dialog>"
				"</dialogprepare>"
			"</mscivr>";

		MscIvr msg;
		const size_t size = msg.CurrentBinarySize();

		msg.ReadFromXmlStream(xml_buf, sizeof(xml_buf));
		CPPUNIT_ASSERT(msg.m_pResponseType.Contains(DialogPrepare::classType()));

		size_t size_full = msg.CurrentBinarySize();
		CPPUNIT_ASSERT(size_full > size);

		CSegment seg;
		seg << msg;

		MscIvr m_seg;
		seg >> m_seg;

		CPPUNIT_ASSERT(m_seg.m_pResponseType.Contains(DialogPrepare::classType()));
		CPPUNIT_ASSERT(m_seg == msg);

		std::cout << msg;

		m_seg.Clear();
		CPPUNIT_ASSERT(size == m_seg.CurrentBinarySize());
	}

	{
		const char xml_buf[] =
			"<mscivr version=\"1.0\" xmlns=\"urn:ietf:params:xml:ns:msc-ivr\" desclang=\"some description\">"
				"<dialogstart connectionid=\"20709600:rmx2k_3332000075-3144-RMX-0000000059-0082509846\">"
					"<dialog>"
						"<prompt>"
							"<media loc=\"https://10.226.131.20:8443/media/proximo-7.0.2.121777/prompts/adhoc-video-6.0.2.121777/General_Polycom_Slide.jpg\" type=\"image/jpeg\"/>"
						"</prompt>"
					"</dialog>"
				"</dialogstart>"
			"</mscivr>";

		MscIvr orig;
		orig.ReadFromXmlStream(xml_buf, sizeof(xml_buf));

		MscIvr* copy1 = new MscIvr;
		copy1->ReadFromXmlStream(xml_buf, sizeof(xml_buf));

		CPPUNIT_ASSERT(orig == *copy1);

		MscIvr* copy2 = copy1->NewCopy();
		CPPUNIT_ASSERT(*copy2 == *copy1);

		delete copy2;
		copy2 = NULL;

		CPPUNIT_ASSERT(orig == *copy1);

		delete copy1;
		copy1 = NULL;
	}
}

//////////////////////////////////////////////////////////////////////
