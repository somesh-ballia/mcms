#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <map>
#include <string>

#include "dsp_monitor_getter.h"
#include "shared_memory_man.h"

#include "dsp_monitor.h"
#include "dsp_element.h"
#include "string_buffer_proxy.h"

using std::string;
using std::map;

static unsigned char s_dummyProtection[512*1024];

static map<string, NetraDspMonitor::SharedAddressFormat> * s_name_map = 0;

namespace NetraDspMonitor
{
const char *dsp_monitor_get_name_suffix_by_id(char *temp, int temp_len, int id)
{
    (void)s_dummyProtection;
    if (id == -1)
        snprintf(temp, temp_len, "__");
    else
        snprintf(temp, temp_len, "%02d", id);
    return temp;
}

static void name_map_init()
{
    map<string, NetraDspMonitor::SharedAddressFormat> & netra_dsp_monitor_name_map = *s_name_map;
    for (int i = 0; i < DSPMONITOR_MAX_BOARD_LIST_LEN; i++)
    {
        char temp[10];
        string suffix_slot = string(dsp_monitor_get_name_suffix_by_id(temp, 10, i));
        
        for (int j = 0; j < DSPMONITOR_MAX_DSP_LIST_LEN; j++)
        {
            string suffix_dsp = suffix_slot + string(dsp_monitor_get_name_suffix_by_id(temp, 10, j));
            netra_dsp_monitor_name_map[string(NDM_TEMPERATURE) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_TEMPE_LEN, NDM_TEMPE_DEFAULT, 0), NDM_TEMPE_LEN);
            netra_dsp_monitor_name_map[string(NDM_HARDWARE_VERSION) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_HARDV_LEN, NDM_HARDV_DEFAULT, 0), NDM_HARDV_LEN);
            netra_dsp_monitor_name_map[string(NDM_SOFTWARE_VERISON) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_SOFTV_LEN, NDM_SOFTV_DEFAULT, 0), NDM_SOFTV_LEN);
            netra_dsp_monitor_name_map[string(NDM_CPU_USAGE) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_CPUUSAGE_LEN, NDM_CPUUSAGE_DEFAULT, 0), NDM_CPUUSAGE_LEN);
            netra_dsp_monitor_name_map[string(NDM_MEMORY_USAGE) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_MEMORYUSAGE_LEN, NDM_MEMORYUSAGE_DEFAULT, 0), NDM_MEMORYUSAGE_LEN);
        
            netra_dsp_monitor_name_map[string(NDM_IS_OCCUPIED) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_ISOCD_LEN, NDM_ISOCD_DEFAULT, 0), NDM_ISOCD_LEN);
            netra_dsp_monitor_name_map[string(NDM_IS_FAULTY) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_ISFTY_LEN, NDM_ISFTY_DEFAULT, 0), NDM_ISFTY_LEN);
            netra_dsp_monitor_name_map[string(NDM_PERCENT_OCCUPIED) + suffix_dsp] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_PEROC_LEN, NDM_PEROC_DEFAULT, 0), NDM_PEROC_LEN);

            for (int k = 0; k < DSPMONITOR_MAX_PORT_LIST_LEN; k++)
            {
                string suffix_port = suffix_dsp + string(dsp_monitor_get_name_suffix_by_id(temp, 10, k));
                netra_dsp_monitor_name_map[string(NDM_CONF_ID) + suffix_port] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_CONFID_LEN, NDM_CONFID_DEFAULT, 0), NDM_CONFID_LEN);
                netra_dsp_monitor_name_map[string(NDM_PARTY_ID) + suffix_port] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_PARTYID_LEN, NDM_PARTYID_DEFAULT, 0), NDM_PARTYID_LEN);
                netra_dsp_monitor_name_map[string(NDM_IS_ACTIVE) + suffix_port] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_ISACT_LEN, NDM_ISACT_DEFAULT, 0), NDM_ISACT_LEN);
                netra_dsp_monitor_name_map[string(NDM_PERCENT_OCCUPIED) + suffix_port] = SharedAddressFormat(g_sha_mem_man.Allocate(NDM_PEROC_LEN, NDM_PEROC_DEFAULT, 0), NDM_PEROC_LEN);
            }
        }
    }
    g_sha_mem_man.AllocateComplete(0);
}
}

static int FindNetraDspMonitorElementInfo(string const & dm_name, int & offset, int & max_size)
{
    typedef map<string, NetraDspMonitor::SharedAddressFormat>::const_iterator const_ele_iterator;
    const_ele_iterator pos = s_name_map->find(dm_name);
    if (s_name_map->end()==pos)
    {
        return false;
    }
    else
    {
        NetraDspMonitor::SharedAddressFormat const & addrFormat = (*pos).second;
        offset = addrFormat.offset;
        max_size = addrFormat.maxSize;
        return true;
    }
}

static void DumpNetraDspMonitorElementInfos()
{
#define DM_NO_DEBUG
#ifndef DM_NO_DEBUG
    typedef map<string, NetraDspMonitor::SharedAddressFormat>::const_iterator const_ele_iterator;
    const_ele_iterator const end = s_name_map->end();
    const_ele_iterator it = s_name_map->begin();
    for (; it!=end; ++it)
    {
        string const & key = (*it).first;
        NetraDspMonitor::SharedAddressFormat const & val = (*it).second;
        TRACE_INFO("KEY=" << key << ", " << val.offset << ", " << val.maxSize);
    }
#endif
}

int InitNetraDspMonitor()
{
    assert(0==s_name_map);
    s_name_map = new map<string, NetraDspMonitor::SharedAddressFormat>();
    NetraDspMonitor::name_map_init();
    DumpNetraDspMonitorElementInfos();
    return 0;
}

void CleanupNetraDspMonitor()
{
    if (0==s_name_map)
    {
        return;
    }

    delete s_name_map;
    s_name_map = 0;
}

namespace NetraDspMonitor
{
StringBufferProxy<1024> dsp_monitor_name_translation(const char * dm_name)
{
    StringBufferProxy<1024> temp;
    string const dmName(dm_name);
    int offset = 0;
    int maxSize = 0;
    int const found = FindNetraDspMonitorElementInfo(dm_name, offset, maxSize);
    if (!found)
    {
        //TRACE_ERROR("HelloInvalidName: "<<dm_name<<"!");
        DumpNetraDspMonitorElementInfos();
        NDMStrSafeCopy(temp.GetBuf(), INVALID_NAME, temp.GetBufSize());
    }
    else
    {
        g_sha_mem_man.WriteShmAddress(temp.GetBuf(), temp.GetBufSize(), offset, maxSize, 0);
    }
    return temp;
}

//static char temp_dsp_name[1024];

static StringBufferProxy<1024> compose_dsp_name(const char *name, DspEleId id)
{
    StringBufferProxy<1024> proxy;
    char * temp_dsp_name = proxy.GetBuf();
    
    char temp[10] = {0};
    NDMStrSafeCopy(temp_dsp_name, name, proxy.GetBufSize());
    strncat(temp_dsp_name, dsp_monitor_get_name_suffix_by_id(temp, sizeof(temp), id.slotId), sizeof(temp) - 1);
    if (id.unitId != -1)
        strncat(temp_dsp_name, dsp_monitor_get_name_suffix_by_id(temp, sizeof(temp), id.unitId), sizeof(temp) - 1);
    if (id.portId != -1)
        strncat(temp_dsp_name, dsp_monitor_get_name_suffix_by_id(temp, sizeof(temp), id.portId), sizeof(temp) - 1);
    return proxy;
}

DspElement::DspElement(const char *name, DspEleId id)
    : BaseElement(dsp_monitor_name_translation(compose_dsp_name(name, id)))
    , m_id(id)
{
}

DspElement::~DspElement()
{
}

int DspElement::GetId(DspEleId& id)
{
    id = m_id;
    return 0;
}

void DspElement::Marshing()
{
    BaseElement::Marshing();
}

void DspElement::Unmarshing()
{
    BaseElement::Unmarshing();
}

class NetraDspMonitorObj
{
public:
    NetraDspMonitorObj()
    {
        InitNetraDspMonitor();
    }
    ~NetraDspMonitorObj()
    {
        CleanupNetraDspMonitor();
    }
};

static NetraDspMonitorObj netra_dsp_monitor_obj;

}; // namespace NetraDspMonitor

