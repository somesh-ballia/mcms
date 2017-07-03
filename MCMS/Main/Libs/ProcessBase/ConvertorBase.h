// ConverterBase.h

#ifndef CCONVERTORBASE_H_
#define CCONVERTORBASE_H_

#include <map>
#include <string>
#include <iostream>
#include <utility>

#include "Trace.h"
#include "PObject.h"
#include "TraceStream.h"

// Replace brand names in a strins, depend on a product type.
std::string RebrandString(const std::string& str);

template <typename TKey, typename TVal>
class CConvertorBase : public    CPObject,
                       protected std::map<TKey, TVal>,
                       private   CNonCopyable
{
  CLASS_TYPE_1(CConvertorBase, CPObject)

 protected:
  CConvertorBase() : m_default() {}
  void Add(const TKey& key, const TVal& val)
  {
    std::pair<typename std::map<TKey, TVal>::iterator, bool> res =
      insert(std::make_pair(key, val));

    PASSERTSTREAM(!res.second, "Key " << key << " already exists.");
  }

  const TVal& Get(const TKey& key) const
  {
    typename std::map<TKey, TVal>::const_iterator it = this->find(key);
    if (this->end() != it)
      return it->second;

    std::ostringstream buf;
    buf << "#" << key;
    m_unknown = buf.str();

    return m_default;
  }

  const std::string& GetStr(const TKey& key) const
  {
    const std::string& str = Get(key);
    if (!str.empty())
      return str;

    std::ostringstream buf;
    buf << "#" << key;
    m_unknown = buf.str();

    return m_unknown;
  }

  const std::string& Unknown() const {return m_unknown;}

 private:
  const TVal          m_default;
  mutable std::string m_unknown;
}; 

#endif
