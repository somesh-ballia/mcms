#ifndef __PRODUCT_TYPE_H__
#define __PRODUCT_TYPE_H__

enum eProductType
{
  eProductTypeUnknown     = -1, // use this for uninitialized data members

 // RMX family 0-20
  
  eProductTypeRMX2000		= 0,
  eProductTypeRMX4000		= 3,
  eProductTypeRMX1500		= 2,
  
  // Signaling gateway family 21-40 
  
  eProductTypeNPG2000               = 21,

  // Call Generator 31-40

  eProductTypeCallGenerator     = 31,

  //SoftMcu 41-50
  eProductTypeSoftMCU           = 41,
  eProductTypeSoftMCUMfw		= 42,
  eProductTypeGesher            = 43,
  eProductTypeNinja             = 44,
  eProductTypeEdgeAxis          = 45,

  //SoftMcu CG
  eProductTypeCallGeneratorSoftMCU = 46,
};

enum eProductFamily
  {
    eProductFamilyUnknown = -1,
    eProductFamilyMGC     =  0,
    eProductFamilyRMX     =  1,
    eProductFamilyNPG     =  2,
    eProductFamilyCallGenerator = 3,
    eProductFamilySoftMcu = 4,
  };

const char* ProductTypeToString(eProductType productType);
const char* ProductFamilyToString(eProductFamily productFamily);
const char* ProductTypeToRPCSString(eProductType type);

eProductType StringToProductType(const char * pn_string);

eProductFamily ProductTypeToProductFamily(eProductType productType);



#endif 
