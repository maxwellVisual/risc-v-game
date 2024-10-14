#pragma once
/* memory.c */
#ifdef CONFIG_INTERNAL_MEMORY
#   ifdef CONFIG_DEFAULT_MEMORY_FILE_SIZE
#       error "aexactly one between CONFIG_INTERNAL_MEMORY and CONFIG_DEFAULT_MEMORY_FILE_SIZE must be set"
#   endif
#else
#   ifndef CONFIG_DEFAULT_MEMORY_FILE_SIZE
#       error "exactly one between CONFIG_INTERNAL_MEMORY and CONFIG_DEFAULT_MEMORY_FILE_SIZE must be set"
#   endif
#endif

/* risc-v.c */
#ifdef CONFIG_USE_RV64
#   define ___CONFIG_BITS "64"
#else
#   define ___CONFIG_BITS "32"
#endif
#ifdef CONFIG_USE_RV_E
#   define ___CONFIG_INST_TYPE "E"
#else
#   define ___CONFIG_INST_TYPE "I"
#endif
#define ___MSG_FORMAT(bits, type) "RV"#bits#type
#define ___NAME ___MSG_FORMAT(___CONFIG_BITS, ___CONFIG_INST_TYPE)
// #pragma message("using #__MSG_FORMAT(___CONFIG_BITS, ___CONFIG_INST_TYPE)")
#ifdef CONFIG_USE_RV_EXTENSION_ZIFENCEI
// #   pragma message("with "#___NAME" Zifencei Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_ZICSR
// #   pragma message("with "#___NAME" Zicsr Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_M
// #   pragma message("with "#RV#__CONFIG_BITS"M Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_A
// #   pragma message("with "#RV#__CONFIG_BITS"A Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_F
// #   pragma message("with "#RV#__CONFIG_BITS"F Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_D
// #   pragma message("with "#RV#__CONFIG_BITS"D Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_Q
// #   pragma message("with "#RV#__CONFIG_BITS"Q Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_ZFH
// #   pragma message("with "#RV#__CONFIG_BITS"Zfh Standard Extension")
#endif
#ifdef CONFIG_USE_RV_EXTENSION_ZAWRS
// #   pragma message("with "#RV#__CONFIG_BITS"Zawrs Standard Extension")
#endif

#undef ___CONFIG_INST_TYPE
#undef ___CONFIG_BITS
#undef ___NAME
#undef ___MSG_FORMAT
