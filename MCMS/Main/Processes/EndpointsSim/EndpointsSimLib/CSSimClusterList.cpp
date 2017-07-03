/*
 * CSSimClusterList.cpp
 *
 *  Created on: May 25, 2010
 *      Author: drabkin
 */
#include <algorithm>
#include <strings.h>
#include <iomanip>
#include "CSSimClusterList.h"
#include "Trace.h"
#include "TraceStream.h"
#include "CSSimCluster.h"

CCSSimClusterList::CCSSimClusterList(void)
{ }

/*
 * Virtual
 */
CCSSimClusterList::~CCSSimClusterList(void)
{
    Clear();
}

/*
 * Virtual
 */
const char* CCSSimClusterList::NameOf(void) const
{
    return GetCompileType();
}

/*
 * Protected
 */
std::list<CCSSimCluster*>::const_iterator
CCSSimClusterList::FindElement(DWORD csID) const
{
    return
        std::find_if(
            m_list.begin(), m_list.end(),
            std::bind2nd(
                std::mem_fun(&CCSSimCluster::HasCSID), csID
            )
        );
}

CCSSimCluster* CCSSimClusterList::Get(DWORD csID) const
{
    std::list<CCSSimCluster*>::const_iterator it = FindElement(csID);
    PASSERTSTREAM_AND_RETURN_VALUE(
        it == m_list.end(),
        "Unable to find cluster CS["
            << csID
            <<"]",
        NULL);

    return *it;
}

int CCSSimClusterList::Add(DWORD csID, DWORD numPorts,
                           const COsQueue& creator, const COsQueue& eps)
{
    // check existence of a cluster with the same ID
    PASSERTSTREAM_AND_RETURN_VALUE(
        FindElement(csID) != m_list.end(),
        "Unable to add new cluster: CS["
            << csID
            << "] already exists",
        -1);

    // create new cluster and add it to the list
    CCSSimCluster* cluster =
        new CCSSimCluster(csID, numPorts, creator, eps);

    m_list.push_back(cluster);

    // it is not a stopper
    PASSERTMSG(m_list.size() > 4,
        "Number of CS clusters is more than four");

    return 0;
}

int CCSSimClusterList::Remove(DWORD csID)
{
    std::list<CCSSimCluster*>::iterator it =
        std::find_if(
            m_list.begin(), m_list.end(),
            std::bind2nd(
                std::mem_fun(&CCSSimCluster::HasCSID), csID
            )
        );

    PASSERTSTREAM_AND_RETURN_VALUE(
        it == m_list.end(),
        "Unable to remove cluster: CS["
            << csID
            << "] is not exist",
        -1);

    delete *it;
    m_list.erase(it);
    return 0;
}

void CCSSimClusterList::Clear(void)
{
    while(!m_list.empty())
    {
        delete m_list.back();
        m_list.pop_back();
    }
}

/*
 * the functor prints out CS cluster basic info
 */
class print_cs_info
{
public:
    print_cs_info(std::ostream& answer) :
        m_answer(answer)
    {
        m_answer << "CS ID | CS NUM PORT"
                 << std::endl
                 << "-------------------";
    }

    void operator()(const CCSSimCluster* cluster)
    {
        m_answer << std::endl
                 << std::setw(5)
                 << cluster->GetCSID()
                 << " | "
                 << std::setw(11)
                 << cluster->GetNumPorts();
    }

private:
    std::ostream& m_answer;
};

void CCSSimClusterList::PrintOut(std::ostream& out) const
{
    std::for_each(m_list.begin(), m_list.end(), print_cs_info(out));
}
