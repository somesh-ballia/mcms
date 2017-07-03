#ifndef __AVC_SVC_CAP_STRUCT_H__
#define __AVC_SVC_CAP_STRUCT_H__

struct AvcSvcCap
{
    int cpPortCount;
    int supportAvcCp;
    int supportAvcVsw;
    int supportSvc;
    int supportMixedCp;
    int supportMixedVsw;
    int supportCascadeAvc;
    int supportCascadeSvc;
    int supportSrtpAvc;
    int supportSrtpSvc;
    int supportTip;
    int supportAvcCifPlus;
    int supportItp;
    int supportAudioOnlyConf;
    int supportHighProfileContent;
    int maxLineRate;
    int licenseMode;
};

#endif

