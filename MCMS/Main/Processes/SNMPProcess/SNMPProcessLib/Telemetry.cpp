// Telemetry.cpp

#include "Telemetry.h"

#include "PrettyTable.h"
#include "TraceStream.h"
#include "ProcessBase.h"

// Timers opcodes.
#define STATISTICS_TIMER                200
#define STATISTICS_TIMER_TIME_OUT_VALUE 10*SECOND
#define SLIDER_SIZE                     60/(STATISTICS_TIMER_TIME_OUT_VALUE/SECOND)

PBEGIN_MESSAGE_MAP(CTelemetry)
  ONEVENT(STATISTICS_TIMER, ANYCASE, CTelemetry::OnTimerCalculate)
PEND_MESSAGE_MAP(CTelemetry, CStateMachine);

CTelemetry::CTelemetry() : m_slider(SLIDER_SIZE)
{}

bool CTelemetry::EnableSlider(bool enable)
{
  bool ret = IsValidTimer(STATISTICS_TIMER);
  PASSERTMSG_AND_RETURN_VALUE(ret && enable,
      "Multiple enables of Telemetry Slider",
      true);
  PASSERTMSG_AND_RETURN_VALUE(!ret && !enable,
      "There is no slider to disable",
      false);

  if (enable)
  {
    StartTimer(STATISTICS_TIMER, STATISTICS_TIMER_TIME_OUT_VALUE);
  }
  else
  {
    DeleteTimer(STATISTICS_TIMER);

    // Resets data.
    m_slider = std::deque<std::map<eTelemetryType, CTelemetryValue> >(SLIDER_SIZE);
  }

  return ret;
}

void CTelemetry::Update(const CTelemetryValue& value)
{
  eTelemetryType type = value.GetType();
  const CTelemetryTraits& traits = CTelemetryTraits::Get(type);

  // Update m_prolonger.
  if (traits.is_prolonged)
  {
    // Verifies that a new element is really new.
    std::pair<std::map<eTelemetryType, CTelemetryValue>::iterator, bool> res =
        m_prolonger.insert(std::make_pair(type, value));

    // Assigns new value only on previously existed element.
    if (!res.second)
      res.first->second.Update(value);

    return;
  }

  std::deque<std::map<eTelemetryType, CTelemetryValue> >::iterator ii;
  for (ii = m_slider.begin(); ii != m_slider.end(); ii++)
  {
    // Verifies that a new element is really new.
    std::pair<std::map<eTelemetryType, CTelemetryValue>::iterator, bool> res =
        ii->insert(std::make_pair(type, value));

    // Continues on a new parameter.
    if (res.second)
      continue;

    res.first->second.Update(value);
  }
}

// For example: 10 seconds a timer.
void CTelemetry::OnTimerCalculate()
{
  CProcessBase* proc = CProcessBase::GetProcess();
  PASSERT_AND_RETURN(NULL == proc);

  // Removes first element and pops next element to the top.
  m_slider.pop_front();

  // Adds new element to the tail.
  m_slider.push_back(std::map<eTelemetryType, CTelemetryValue>());

  // Updates dynamic values, possible values are disabled(1), ok(2), failed(3).
  unsigned int state = (eMcuState_Normal == proc->GetSystemState()) ? 2 : 3;
  Update(CTelemetryValue(eTT_SystemStatus, state));

  StartTimer(STATISTICS_TIMER, STATISTICS_TIMER_TIME_OUT_VALUE);
}

unsigned int CTelemetry::Sum(eTelemetryType t1, eTelemetryType t2) const
{
  CTelemetryValue v1 = Get(t1);
  CTelemetryValue v2 = Get(t2);

  size_t dummy;
  unsigned int sum;
  if (!v1.IsEmpty() && !v2.IsEmpty())
    sum = *reinterpret_cast<unsigned int*>(v1.GetPtr(dummy)) +
          *reinterpret_cast<unsigned int*>(v2.GetPtr(dummy));
  else
    sum = 0u;

  return sum;
}

// Static.
unsigned int CTelemetry::CalculateRatio(unsigned int v, unsigned int t)
{
  if (0 == t)
    return 0;
  
  unsigned int r = (v*100)/t;
  if ((v*100)%t > 5)
    r++;

  return r;
}

unsigned int CTelemetry::Ratio(eTelemetryType t1, eTelemetryType t2) const
{
  CTelemetryValue v1 = Get(t1);
  CTelemetryValue v2 = Get(t2);

  if (v1.IsEmpty() || v2.IsEmpty())
    return 0;

  size_t dummy;
  unsigned int v = *reinterpret_cast<unsigned int*>(v1.GetPtr(dummy));
  unsigned int t = *reinterpret_cast<unsigned int*>(v2.GetPtr(dummy));
  return CalculateRatio(v, t);
}

unsigned int CTelemetry::RatioOnSum(eTelemetryType t1, eTelemetryType t2) const
{
  CTelemetryValue v1 = Get(t1);
  CTelemetryValue v2 = Get(t2);

  if (v1.IsEmpty() || v2.IsEmpty())
    return 0;

  size_t dummy;
  unsigned int v = *reinterpret_cast<unsigned int*>(v1.GetPtr(dummy));
  unsigned int t = v +
                   *reinterpret_cast<unsigned int*>(v2.GetPtr(dummy));
  return CalculateRatio(v, t);
}

CTelemetryValue CTelemetry::Get(eTelemetryType type) const
{
  PASSERTSTREAM_AND_RETURN_VALUE(type <= eTT_Unknown || type >= eTT_Max,
      "Unable to process with telemetry type " << TelemetaryTypeToStr(type),
      CTelemetryValue());

  if (CTelemetryTraits::Get(type).is_calculated)
  {
    switch (type)
    {
      case eTT_NumPortsUsed:
        return CTelemetryValue(eTT_NumPortsUsed,
                               Sum(eTT_NumVideoPortsUsed, eTT_NumVoicePortsUsed));

      case eTT_NumVideoPortsUsedPercentage:
        return CTelemetryValue(eTT_NumVideoPortsUsedPercentage,
                               Ratio(eTT_NumVideoPortsUsed, eTT_NumVideoPorts));

      case eTT_NumVoicePortsUsedPercentage:
        return CTelemetryValue(eTT_NumVoicePortsUsedPercentage,
                               Ratio(eTT_NumVoicePortsUsed, eTT_NumVoicePorts));

      case eTT_NewCalls:
        return CTelemetryValue(eTT_NewCalls,
                               Sum(eTT_SuccessfulNewCalls, eTT_FailedNewCalls));

      case eTT_EndCalls:
        return CTelemetryValue(eTT_EndCalls,
                               Sum(eTT_SuccessfulEndCalls, eTT_FailedEndCalls));

      case eTT_RatioSuccessfulNewCalls:
        return CTelemetryValue(eTT_RatioSuccessfulNewCalls,
                               RatioOnSum(eTT_SuccessfulNewCalls, eTT_FailedNewCalls));


      case eTT_RatioSuccessfulEndCalls:
        return CTelemetryValue(eTT_RatioSuccessfulEndCalls,
                               RatioOnSum(eTT_SuccessfulEndCalls, eTT_FailedEndCalls));

      case eTT_RatioPortsUsed:
      {
        CTelemetryValue used   = Get(eTT_NumPortsUsed);
        CTelemetryValue total1 = Get(eTT_NumVideoPorts);
        CTelemetryValue total2 = Get(eTT_NumVoicePorts);

        size_t dummy;
        unsigned int ratio;
        if (!used.IsEmpty() && !total1.IsEmpty() && !total2.IsEmpty())
        {
          unsigned int v = *reinterpret_cast<unsigned int*>(used.GetPtr(dummy));
          unsigned int t = *reinterpret_cast<unsigned int*>(total1.GetPtr(dummy)) +
                           *reinterpret_cast<unsigned int*>(total2.GetPtr(dummy));
          ratio = CalculateRatio(v, t);
        }
        else
        {
          ratio = 0;
        }

        return CTelemetryValue(eTT_RatioPortsUsed, ratio);
      }

      default:
        PASSERTSTREAM(true, "Illegal type " << TelemetaryTypeToStr(type));
    }

    return CTelemetryValue();
  }

  std::map<eTelemetryType, CTelemetryValue>::const_iterator it;
  if (CTelemetryTraits::Get(type).is_prolonged)
  {
    it = m_prolonger.find(type);
    if (it != m_prolonger.end())
      return it->second;

    CSegment dvalue = CTelemetryTraits::Get(type).dvalue;
    return CTelemetryValue(dvalue);
  }

  // Looks for data for last 10 seconds.
  it = m_slider.front().find(type);
  if (it != m_slider.front().end())
    return it->second;

  CSegment dvalue = CTelemetryTraits::Get(type).dvalue;
  return CTelemetryValue(dvalue);
}

// Virtual, private.
const char* CTelemetry::NameOf() const
{
  return GetCompileType();
}

// Virtual, private.
void CTelemetry::Dump(std::ostream& out) const
{
  CPrettyTable<const char*, const char*, const char*, const char*> 
    tbl("name", "type", "action", "value");

  eTelemetryType ii = eTT_Unknown;
  for (++ii; ii < eTT_Max; ++ii)
  {
    std::ostringstream buf;
    buf << Get(ii);
    tbl.Add(TelemetaryTypeToStr(ii),
            TelemetryTypeBasicToStr(CTelemetryTraits::Get(ii).btype),
            TelemetryActionToStr(CTelemetryTraits::Get(ii).action),
            buf.str().c_str());
  }

  tbl.SetCaption("snmp db");
  tbl.Sort(0);
  tbl.Dump(out);
}
