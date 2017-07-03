#ifndef __PARCEL_DATA_H__
#define __PARCEL_DATA_H__

namespace NetraDspMonitor
{

class ParcelData
{
public:
    static const int max_data_len = 1024;
    static const int max_int_len = 10;

private:
    char m_data[max_data_len];

    int m_rPointer;
    int m_wPointer;
    
public:
    ParcelData();
    ParcelData(const char * data, int len);
    ParcelData(const ParcelData&);
    ~ParcelData();
    
    void Put(int data);
    void Put(const char *data);

    bool Get(int * data);
    bool Get(char ** data);

    bool Empty() const;

    const char *Data() const;
    int Len() const;

    void ClearRead();
    void ClearWrite();

    ParcelData &operator=(const ParcelData &);
    
private:
    void GetStrCopy(char **str);
    void PutStr(const char *str);
    void CopyFrom(const ParcelData&);
    void InitPointer();
    

}; // ParcelData


}; // namespace NetraDspMonitor



#endif // __PARCEL_DATA_H__

