

#if !defined(_VALGRIND_SEQ_H__)
#define _VALGRIND_SEQ_H__


#define VG_USERREQ_SKIN_BASE(a,b) \
   ((unsigned int)(((a)&0xff) << 24 | ((b)&0xff) << 16))
#define VG_IS_SKIN_USERREQ(a, b, v) \
   (VG_USERREQ_SKIN_BASE(a,b) == ((v) & 0xffff0000))



typedef
   enum { 
      VG_USERREQ__MAKE_NOACCESS = VG_USERREQ_SKIN_BASE('M','C'),
      VG_USERREQ__MAKE_WRITABLE,
      VG_USERREQ__MAKE_READABLE,
      VG_USERREQ__DISCARD,
      VG_USERREQ__CHECK_WRITABLE,
      VG_USERREQ__CHECK_READABLE,
      VG_USERREQ__DO_LEAK_CHECK,
      VG_USERREQ__COUNT_LEAKS,

      /* These two have been moved into core, because they are useful for
         any tool that tracks heap blocks.  Hence the suffix.  But they're
         still here for backwards compatibility, although Valgrind will
         abort with an explanatory message if you use them. */
      VG_USERREQ__MALLOCLIKE_BLOCK__OLD_DO_NOT_USE,
      VG_USERREQ__FREELIKE_BLOCK__OLD_DO_NOT_USE,

      VG_USERREQ__GET_VBITS,
      VG_USERREQ__SET_VBITS,

      VG_USERREQ__CREATE_BLOCK,

      /* This is just for memcheck's internal use - don't use it */
      _VG_USERREQ__MEMCHECK_GET_RECORD_OVERLAP = VG_USERREQ_SKIN_BASE('M','C')+256
   } Vg_MemCheckClientRequest;


#define VALGRIND_MAGIC_SEQUENCE(				\
        _zzq_rlval, _zzq_default, _zzq_request,			\
        _zzq_arg1, _zzq_arg2, _zzq_arg3, _zzq_arg4)		\
								\
  { unsigned int _zzq_args[5];					\
    _zzq_args[0] = (unsigned int)(_zzq_request);		\
    _zzq_args[1] = (unsigned int)(_zzq_arg1);			\
    _zzq_args[2] = (unsigned int)(_zzq_arg2);			\
    _zzq_args[3] = (unsigned int)(_zzq_arg3);			\
    _zzq_args[4] = (unsigned int)(_zzq_arg4);			\
    asm volatile("roll $29, %%eax ; roll $3, %%eax\n\t"		\
                 "rorl $27, %%eax ; rorl $5, %%eax\n\t"		\
                 "roll $13, %%eax ; roll $19, %%eax"		\
                 : "=d" (_zzq_rlval)				\
                 : "a" (&_zzq_args[0]), "0" (_zzq_default)	\
                 : "cc", "memory"				\
                );						\
  }

#define VALGRIND_DO_LEAK_CHECK                                     \
   {unsigned int _qzz_res;                                         \
    VALGRIND_MAGIC_SEQUENCE(_qzz_res, 0,                           \
                            VG_USERREQ__DO_LEAK_CHECK,             \
                            0, 0, 0, 0);                           \
   }




#endif
