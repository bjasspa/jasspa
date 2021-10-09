/* -*- C -*- ****************************************************************
 *
 * Copyright (c) 2020 Maxinity Software Ltd (www.maxinity.co.uk).
 * 
 * All Rights Reserved.
 * 
 * This  document  may  not, in  whole  or in  part, be  copied,  photocopied,
 * reproduced,  translated,  or  reduced to any  electronic  medium or machine
 * readable form without prior written consent from Maxinity Software Ltd.
 *
 ****************************************************************************/

#define meSSL_DEBUG 1
#define meSSL_USE_DLL 1
#define meSSLBUFSIZ 4096
#if meSSL_USE_DLL
#define MESSLFunc(f) sslF_##f
#else
#define MESSLFunc(f) f
#endif
#define MESSL_STRINGQUT(str) #str
#define MESSL_STRINGIFY(str) MESSL_STRINGQUT(str)

#include "messl.h"
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#include <openssl/conf.h>
#include <openssl/engine.h>

#ifdef _WIN32
#define meSslLibGetFunc    GetProcAddress
#define meSslGetError      GetLastError
#define meSocketGetError   WSAGetLastError
#define meSocketClose      closesocket
#define meBadSocket        INVALID_SOCKET
#define snprintf           sprintf_s          
#else
#include <unistd.h>
#include <dlfcn.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#define meSslLibGetFunc    dlsym
#define meSslGetError()    (errno)
#define meSocketGetError() (errno)
#define meSocketClose      close
#define meBadSocket        -1
#endif

#if meSSL_USE_DLL

typedef BIO *(*meSSLF_BIO_new)(const BIO_METHOD *type);
typedef int (*meSSLF_BIO_free)(BIO *a);
typedef uint64_t (*meSSLF_BIO_number_written)(BIO *bio);
typedef int (*meSSLF_BIO_read)(BIO *b, void *data, int dlen);
typedef const BIO_METHOD *(*meSSLF_BIO_s_mem)(void);
typedef void (*meSSLF_CRYPTO_free)(void *ptr, const char *file, int line);
typedef void (*meSSLF_ERR_clear_error)(void);
typedef char *(*meSSLF_ERR_error_string)(unsigned long e, char *buf);
typedef unsigned long (*meSSLF_ERR_get_error)(void);
typedef int (*meSSLF_OPENSSL_sk_num)(const OPENSSL_STACK *);
typedef void *(*meSSLF_OPENSSL_sk_value)(const OPENSSL_STACK *, int);
typedef void (*meSSLF_OPENSSL_sk_pop_free)(OPENSSL_STACK *st, void (*func) (void *));
typedef const char *(*meSSLF_RAND_file_name)(char *file, size_t num);
typedef int (*meSSLF_RAND_load_file)(const char *file, long max_bytes);
typedef int (*meSSLF_RAND_status)(void);
typedef const char *(*meSSLF_SSL_CIPHER_get_name)(const SSL_CIPHER *c);
typedef long (*meSSLF_SSL_CTX_ctrl)(SSL_CTX *ctx, int cmd, long larg, void *parg);
typedef void (*meSSLF_SSL_CTX_free)(SSL_CTX *);
typedef X509_STORE *(*meSSLF_SSL_CTX_get_cert_store)(const SSL_CTX *);
typedef int (*meSSLF_SSL_CTX_load_verify_locations)(SSL_CTX *ctx, const char *CAfile, const char *CApath);
typedef SSL_CTX *(*meSSLF_SSL_CTX_new)(const SSL_METHOD *meth);
typedef int (*meSSLF_SSL_CTX_set1_param)(SSL_CTX *ctx, X509_VERIFY_PARAM *vpm);
typedef int (*meSSLF_SSL_CTX_set_default_verify_paths)(SSL_CTX *ctx);
typedef void (*meSSLF_SSL_CTX_set_verify)(SSL_CTX *ctx, int mode, SSL_verify_cb callback);
typedef int (*meSSLF_SSL_connect)(SSL *ssl);
typedef long (*meSSLF_SSL_ctrl)(SSL *ssl, int cmd, long larg, void *parg);
typedef void (*meSSLF_SSL_free)(SSL *ssl);
typedef const SSL_CIPHER *(*meSSLF_SSL_get_current_cipher)(const SSL *s);
typedef int (*meSSLF_SSL_get_error)(const SSL *s, int ret_code);
typedef X509 *(*meSSLF_SSL_get_peer_certificate)(const SSL *s);
typedef long (*meSSLF_SSL_get_verify_result)(const SSL *ssl);
typedef const char *(*meSSLF_SSL_get_version)(const SSL *s);
typedef SSL *(*meSSLF_SSL_new)(SSL_CTX *ctx);
typedef int (*meSSLF_SSL_read)(SSL *ssl, void *buf, int num);
typedef int (*meSSLF_SSL_set_fd)(SSL *s, int fd);
typedef int (*meSSLF_SSL_shutdown)(SSL *s);
typedef int (*meSSLF_SSL_write)(SSL *ssl, const void *buf, int num);
typedef const SSL_METHOD *(*meSSLF_TLS_client_method)(void);
typedef int (*meSSLF_X509_NAME_print_ex)(BIO *out, const X509_NAME *nm, int indent, unsigned long flags);
typedef int (*meSSLF_X509_STORE_add_cert)(X509_STORE *ctx, X509 *x);
typedef void (*meSSLF_X509_VERIFY_PARAM_free)(X509_VERIFY_PARAM *param);
typedef X509_VERIFY_PARAM *(*meSSLF_X509_VERIFY_PARAM_new)(void);
typedef int (*meSSLF_X509_VERIFY_PARAM_set_flags)(X509_VERIFY_PARAM *param, unsigned long flags);
typedef void *(*meSSLF_X509_get_ext_d2i)(const X509 *x, int nid, int *crit, int *idx);
typedef X509_NAME *(*meSSLF_X509_get_issuer_name)(const X509 *a);
typedef X509_NAME *(*meSSLF_X509_get_subject_name)(const X509 *a);
typedef const char *(*meSSLF_X509_verify_cert_error_string)(long n);
typedef ASN1_OCTET_STRING *(*meSSLF_a2i_IPADDRESS)(const char *ipasc);
typedef ASN1_STRING *(*meSSLF_X509_NAME_ENTRY_get_data)(const X509_NAME_ENTRY *ne);
typedef X509_NAME_ENTRY *(*meSSLF_X509_NAME_get_entry)(const X509_NAME *name, int loc);
typedef int (*meSSLF_X509_NAME_get_index_by_NID)(X509_NAME *name, int nid, int lastpos);
typedef int (*meSSLF_ASN1_STRING_length)(const ASN1_STRING *x);
typedef char *(*meSSLF_X509_NAME_oneline)(const X509_NAME *a, char *buf, int size);
typedef int (*meSSLF_X509_NAME_print_ex_fp)(FILE *fp, const X509_NAME *nm, int indent, unsigned long flags);
typedef int (*meSSLF_ASN1_STRING_cmp)(const ASN1_STRING *a, const ASN1_STRING *b);
typedef int (*meSSLF_ASN1_STRING_to_UTF8)(unsigned char **out, const ASN1_STRING *in);
typedef int (*meSSLF_CONF_modules_load_file)(const char *filename, const char *appname, unsigned long flags);
typedef void (*meSSLF_ENGINE_load_builtin_engines)(void);
typedef int (*meSSLF_OPENSSL_init_ssl)(uint64_t opts, const OPENSSL_INIT_SETTINGS *settings);
typedef void (*meSSLF_OPENSSL_load_builtin_modules)(void);
typedef int (*meSSLF_RAND_poll)(void);
typedef const SSL_METHOD *(*meSSLF_TLS_client_method)(void);
typedef long (*meSSLF_SSL_get_verify_result)(const SSL *ssl);
typedef void (*meSSLF_X509_free)(X509 *a);
typedef void (*meSSLF_GENERAL_NAME_free)(GENERAL_NAME *a);
typedef void (*meSSLF_CRYPTO_free)(void *ptr, const char *file, int line);
typedef void (*meSSLF_ASN1_OCTET_STRING_free)(ASN1_OCTET_STRING *a);
typedef X509 *(*meSSLF_d2i_X509)(X509 **a, const unsigned char **in, long len);
typedef int (*meSSLF_X509_NAME_get_text_by_NID)(X509_NAME *name, int nid, char *buf, int len);

meSSLF_ASN1_OCTET_STRING_free sslF_ASN1_OCTET_STRING_free;
meSSLF_ASN1_STRING_cmp sslF_ASN1_STRING_cmp;
meSSLF_ASN1_STRING_length sslF_ASN1_STRING_length;
meSSLF_ASN1_STRING_to_UTF8 sslF_ASN1_STRING_to_UTF8;
meSSLF_BIO_free sslF_BIO_free;
meSSLF_BIO_new sslF_BIO_new;
meSSLF_BIO_number_written sslF_BIO_number_written;
meSSLF_BIO_read sslF_BIO_read;
meSSLF_BIO_s_mem sslF_BIO_s_mem;
meSSLF_CONF_modules_load_file sslF_CONF_modules_load_file;
meSSLF_CRYPTO_free sslF_CRYPTO_free;
meSSLF_CRYPTO_free sslF_CRYPTO_free;
meSSLF_ENGINE_load_builtin_engines sslF_ENGINE_load_builtin_engines;
meSSLF_ERR_clear_error sslF_ERR_clear_error;
meSSLF_ERR_error_string sslF_ERR_error_string;
meSSLF_ERR_get_error sslF_ERR_get_error;
meSSLF_GENERAL_NAME_free sslF_GENERAL_NAME_free;
meSSLF_OPENSSL_init_ssl sslF_OPENSSL_init_ssl;
meSSLF_OPENSSL_load_builtin_modules sslF_OPENSSL_load_builtin_modules;
meSSLF_OPENSSL_sk_num sslF_OPENSSL_sk_num;
meSSLF_OPENSSL_sk_value sslF_OPENSSL_sk_value;
meSSLF_OPENSSL_sk_pop_free sslF_OPENSSL_sk_pop_free;
meSSLF_RAND_file_name sslF_RAND_file_name;
meSSLF_RAND_load_file sslF_RAND_load_file;
meSSLF_RAND_poll sslF_RAND_poll;
meSSLF_RAND_status sslF_RAND_status;
meSSLF_SSL_CIPHER_get_name sslF_SSL_CIPHER_get_name;
meSSLF_SSL_CTX_ctrl sslF_SSL_CTX_ctrl;
meSSLF_SSL_CTX_free sslF_SSL_CTX_free;
meSSLF_SSL_CTX_get_cert_store sslF_SSL_CTX_get_cert_store;
meSSLF_SSL_CTX_load_verify_locations sslF_SSL_CTX_load_verify_locations;
meSSLF_SSL_CTX_new sslF_SSL_CTX_new;
meSSLF_SSL_CTX_set1_param sslF_SSL_CTX_set1_param;
meSSLF_SSL_CTX_set_default_verify_paths sslF_SSL_CTX_set_default_verify_paths;
meSSLF_SSL_CTX_set_verify sslF_SSL_CTX_set_verify;
meSSLF_SSL_connect sslF_SSL_connect;
meSSLF_SSL_ctrl sslF_SSL_ctrl;
meSSLF_SSL_free sslF_SSL_free;
meSSLF_SSL_get_current_cipher sslF_SSL_get_current_cipher;
meSSLF_SSL_get_error sslF_SSL_get_error;
meSSLF_SSL_get_peer_certificate sslF_SSL_get_peer_certificate;
meSSLF_SSL_get_verify_result sslF_SSL_get_verify_result;
meSSLF_SSL_get_verify_result sslF_SSL_get_verify_result;
meSSLF_SSL_get_version sslF_SSL_get_version;
meSSLF_SSL_new sslF_SSL_new;
meSSLF_SSL_read sslF_SSL_read;
meSSLF_SSL_set_fd sslF_SSL_set_fd;
meSSLF_SSL_shutdown sslF_SSL_shutdown;
meSSLF_SSL_write sslF_SSL_write;
meSSLF_TLS_client_method sslF_TLS_client_method;
meSSLF_TLS_client_method sslF_TLS_client_method;
meSSLF_X509_NAME_ENTRY_get_data sslF_X509_NAME_ENTRY_get_data;
meSSLF_X509_NAME_get_entry sslF_X509_NAME_get_entry;
meSSLF_X509_NAME_get_index_by_NID sslF_X509_NAME_get_index_by_NID;
meSSLF_X509_NAME_oneline sslF_X509_NAME_oneline;
meSSLF_X509_NAME_print_ex sslF_X509_NAME_print_ex;
meSSLF_X509_NAME_print_ex_fp sslF_X509_NAME_print_ex_fp;
meSSLF_X509_STORE_add_cert sslF_X509_STORE_add_cert;
meSSLF_X509_VERIFY_PARAM_free sslF_X509_VERIFY_PARAM_free;
meSSLF_X509_VERIFY_PARAM_new sslF_X509_VERIFY_PARAM_new;
meSSLF_X509_VERIFY_PARAM_set_flags sslF_X509_VERIFY_PARAM_set_flags;
meSSLF_X509_free sslF_X509_free;
meSSLF_X509_get_ext_d2i sslF_X509_get_ext_d2i;
meSSLF_X509_get_issuer_name sslF_X509_get_issuer_name;
meSSLF_X509_get_subject_name sslF_X509_get_subject_name;
meSSLF_X509_verify_cert_error_string sslF_X509_verify_cert_error_string;
meSSLF_a2i_IPADDRESS sslF_a2i_IPADDRESS;
meSSLF_d2i_X509 sslF_d2i_X509;
meSSLF_X509_NAME_get_text_by_NID sslF_X509_NAME_get_text_by_NID;

static ossl_unused ossl_inline int sslF_sk_GENERAL_NAME_num(const STACK_OF(GENERAL_NAME) *sk)
{
    return MESSLFunc(OPENSSL_sk_num)((const OPENSSL_STACK *)sk);
}
static ossl_unused ossl_inline GENERAL_NAME *sslF_sk_GENERAL_NAME_value(const STACK_OF(GENERAL_NAME) *sk, int idx)
{
    return (GENERAL_NAME *) MESSLFunc(OPENSSL_sk_value)((const OPENSSL_STACK *)sk, idx);
}
static ossl_unused ossl_inline void sslF_sk_GENERAL_NAME_pop_free(STACK_OF(GENERAL_NAME) *sk, sk_GENERAL_NAME_freefunc freefunc)
{
    MESSLFunc(OPENSSL_sk_pop_free)((OPENSSL_STACK *)sk, (OPENSSL_sk_freefunc)freefunc);
}
#endif

static SSL_CTX *meSslCtx=NULL;
static meUByte *proxyHost=NULL;
static int proxyPort=0;
static meInt logFlags=0;
static meSslLogger logFunc;
static void *logData;

static void
strBase64Encode3(meUByte *dd, meUByte c1, meUByte c2, meUByte c3)
{
    static meUByte base64Table[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" ;
    dd[0] = base64Table[(c1 >> 2)] ;
    dd[1] = base64Table[((c1 << 4) & 0x30) | ((c2 >> 4) & 0x0f)] ;
    dd[2] = base64Table[((c2 << 2) & 0x3c) | ((c3 >> 6) & 0x03)] ;
    dd[3] = base64Table[(c3 & 0x3f)] ;
}

static meUByte *
strBase64Encode(meUByte *dd, meUByte *ss)
{
    meUByte c1, c2, c3 ;

    while((c1=*ss++) != '\0')
    {
        if((c2=*ss++) == '\0')
        {
            c3 = '\0' ;
            strBase64Encode3(dd,c1,c2,c3) ;
            dd += 2 ;
            *dd++ = '=' ;
            *dd++ = '=' ;
            break ;
        }
        else if((c3=*ss++) == '\0')
        {
            strBase64Encode3(dd,c1,c2,c3) ;
            dd += 3 ;
            *dd++ = '=' ;
            break ;
        }
        strBase64Encode3(dd,c1,c2,c3) ;
        dd += 4 ;
    }
    *dd = '\0' ;
    return dd ;
}

static void
meSslGetSSLErrors(meUByte *buff)
{
    if(logFunc != NULL)
    {
        unsigned long err;
        int bi = 0;
        while((err = MESSLFunc(ERR_get_error)()) != 0)
            bi += snprintf((char *) buff+bi,meSSL_BUFF_SIZE - bi,"\n  OpenSSL: %s",MESSLFunc(ERR_error_string)(err,NULL));
        logFunc(buff,logData);
    }
    else
        MESSLFunc(ERR_clear_error)();
}

static int
meSslInitPrng(meUByte *buff)
{
    const char *random_file;
    
    if(MESSLFunc(RAND_status)())
        return 0;
    
    /* Get the random file name using RAND_file_name. */
    buff[0] = '\0';
    if(((random_file = MESSLFunc(RAND_file_name)((char *) buff,meSSL_BUFF_SIZE)) != NULL) && (random_file[0] != '\0'))
    {
        /* Seed at most 16k (apparently arbitrary value borrowed from
           curl) from random file. */
        MESSLFunc(RAND_load_file)(random_file,16384);
        if(MESSLFunc(RAND_status)())
            return 0;
    }
    MESSLFunc(RAND_poll)();
    if(MESSLFunc(RAND_status)())
        return 0;
    if(logFunc != NULL)
    {
        snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Could not seed PRNG");
        logFunc(buff,logData);
    }
    return -1;
}

static int
mSslIsValidIp4Address(const char *str, const char *end)
{
    int saw_digit = 0;
    int octets = 0;
    int val = 0;
    
    while (str < end)
    {
        int ch = *str++;
        
        if (ch >= '0' && ch <= '9')
        {
            val = val * 10 + (ch - '0');
            
            if (val > 255)
                return 0;
            if (!saw_digit)
            {
                if (++octets > 4)
                    return 0;
                saw_digit = 1;
            }
        }
        else if (ch == '.' && saw_digit)
        {
            if (octets == 4)
                return 0;
            val = 0;
            saw_digit = 0;
        }
        else
            return 0;
    }
    if (octets < 4)
        return 0;
    
    return 1;
}

#ifdef ENABLE_IPV6
static int
mSslIsValidIp6Address (const char *str, const char *end)
{
    /* Use lower-case for these to avoid clash with system headers.  */
    enum {
        ns_inaddrsz  = 4,
        ns_in6addrsz = 16,
        ns_int16sz   = 2
    };
    
    const char *curtok;
    int tp;
    const char *colonp;
    int saw_xdigit;
    unsigned int val;
    
    tp = 0;
    colonp = NULL;
    
    if (str == end)
        return 0;
    
    /* Leading :: requires some special handling. */
    if (*str == ':')
    {
        ++str;
        if (str == end || *str != ':')
            return 0;
    }
    
    curtok = str;
    saw_xdigit = 0;
    val = 0;
    
    while (str < end)
    {
        int ch = *str++;
        
        /* if ch is a number, add it to val. */
        if (c_isxdigit (ch))
        {
            val <<= 4;
            val |= XDIGIT_TO_NUM (ch);
            if (val > 0xffff)
                return 0;
            saw_xdigit = 1;
            continue;
        }
        
        /* if ch is a colon ... */
        if (ch == ':')
        {
            curtok = str;
            if (!saw_xdigit)
            {
                if (colonp != NULL)
                    return 0;
                colonp = str + tp;
                continue;
            }
            else if (str == end)
                return 0;
            if (tp > ns_in6addrsz - ns_int16sz)
                return 0;
            tp += ns_int16sz;
            saw_xdigit = 0;
            val = 0;
            continue;
        }
        
        /* if ch is a dot ... */
        if (ch == '.' && (tp <= ns_in6addrsz - ns_inaddrsz)
            && mSslIsValidIp4Address(curtok, end) == 1)
        {
            tp += ns_inaddrsz;
            saw_xdigit = 0;
            break;
        }
        
        return 0;
    }
    
    if (saw_xdigit)
    {
        if (tp > ns_in6addrsz - ns_int16sz)
            return 0;
        tp += ns_int16sz;
    }
    
    if (colonp != NULL)
    {
        if (tp == ns_in6addrsz)
            return 0;
        tp = ns_in6addrsz;
    }
    
    if (tp != ns_in6addrsz)
        return 0;
    
    return 1;
}
#endif

int
mSslIsValidIpAddress(const char *name)
{
    const char *endp;
    
    endp = name + strlen(name);
    if(mSslIsValidIp4Address(name,endp))
        return 1;
#ifdef ENABLE_IPV6
    if(mSslIsValidIp6Address (name, endp))
        return 1;
#endif
    return 0;
}
static void
meSsslHostnameGetSni(const char *hostname, char *outBuff)
{
    size_t len = strlen(hostname);
    memcpy(outBuff,hostname,len);

  /* Remove trailing dot(s) to fix #47408.
   * Regarding RFC 6066 (SNI): The hostname is represented as a byte
   * string using ASCII encoding without a trailing dot. */
    while((len > 0) && (outBuff[len-1] == '.'))
        len--;
    outBuff[len] = 0;
}


#ifdef _WIN32
void
meSslInitSystemCaCert(SSL_CTX *sslCtx, char *rbuff)
{
    HCERTSTORE hStore;
    PCCERT_CONTEXT cCntx = NULL;
    const unsigned char *ce;
    X509 *x509;
    X509_STORE *store;
    char buf[128];
    int ii=1;
    
    if((store = MESSLFunc(SSL_CTX_get_cert_store)(sslCtx)) == NULL)
    {
        if(logFlags & meSSL_LOG_WARNING)
            logFunc((meUByte *) "OpenSSL: Failed to get SSL_CTX cert store",logData);
        return;
    }
    
    do {
        // The Root store holds the main root CAs and the CA store holds any intermediate CAs - bail out if we can't open the Root
        if((hStore = CertOpenSystemStore(0,(ii) ? "Root":"CA")) == 0)
        {
            if(logFlags & meSSL_LOG_WARNING)
                logFunc((meUByte *) "OpenSSL: Failed to open Root system cert store",logData);
            return;
        }

        while((cCntx = CertEnumCertificatesInStore(hStore,cCntx)) != NULL)
        {
            //uncomment the line below if you want to see the certificates as pop ups
            //CryptUIDlgViewContext(CERT_STORE_CERTIFICATE_CONTEXT,cCntx,NULL,NULL,0,NULL);
            ce = cCntx->pbCertEncoded;
            x509 = MESSLFunc(d2i_X509)(NULL,&ce,cCntx->cbCertEncoded);
            if (x509)
            {
                MESSLFunc(X509_NAME_oneline)(MESSLFunc(X509_get_subject_name)(x509),buf,sizeof(buf));
                if(MESSLFunc(X509_STORE_add_cert)(store,x509) == 1)
                {
                    if(logFlags & meSSL_LOG_VERBOSE)
                    {
                        snprintf(rbuff,meSSL_BUFF_SIZE,"OpenSSL: Loaded & added Windows Root certificate for: subject='%s'",buf);
                        logFunc(rbuff,logData);
                    }
                }
                else if(logFlags & meSSL_LOG_WARNING)
                {
                    snprintf(rbuff,meSSL_BUFF_SIZE,"OpenSSL: Loaded but failed to added Windows Root certificate for: subject='%s'",buf);
                    logFunc(rbuff,logData);
                }
                MESSLFunc(X509_free)(x509);
            }
            else if(logFlags & meSSL_LOG_WARNING)
            {
                snprintf(rbuff,meSSL_BUFF_SIZE,"OpenSSL: Failed to generate x509 for Windows Root certificate",logData);
                logFunc(rbuff,logData);
            }
        }
        CertCloseStore(hStore,0);
    } while(--ii >= 0);
}
#endif          

int
meSslInit(meUByte *buff)
{
#if meSSL_USE_DLL
#ifdef _WIN32
    HINSTANCE libHandle ; 
#else
    void *libHandle ; 
#endif
#endif          
    SSL_METHOD const *meth;
#ifdef X509_V_FLAG_PARTIAL_CHAIN
    X509_VERIFY_PARAM *param;
#endif          
    if(meSslCtx != NULL)
        /* The SSL has already been initialized. */
        return 1;
    
#if meSSL_USE_DLL
    if(logFlags & meSSL_LOG_VERBOSE)
        logFunc((meUByte *) "About to get OpenSSL functions",logData);
#ifdef _WIN32
    libHandle = LoadLibrary(MESSL_STRINGIFY(_OPENSSLCNM));
#else
    libHandle = dlopen(MESSL_STRINGIFY(_OPENSSLCNM),RTLD_LOCAL|RTLD_LAZY);
#endif
    if(libHandle == NULL)
    {
        snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Failed to load " MESSL_STRINGIFY(_OPENSSLCNM) " library - OpenSSL installed? (%d)",meSslGetError());
        if(logFunc != NULL)
            logFunc(buff,logData);
        return -1;
    }
    if(((sslF_ASN1_OCTET_STRING_free = (meSSLF_ASN1_OCTET_STRING_free) meSslLibGetFunc(libHandle,"ASN1_OCTET_STRING_free")) == NULL) ||
       ((sslF_ASN1_STRING_cmp = (meSSLF_ASN1_STRING_cmp) meSslLibGetFunc(libHandle,"ASN1_STRING_cmp")) == NULL) ||
       ((sslF_ASN1_STRING_length = (meSSLF_ASN1_STRING_length) meSslLibGetFunc(libHandle,"ASN1_STRING_length")) == NULL) ||
       ((sslF_ASN1_STRING_to_UTF8 = (meSSLF_ASN1_STRING_to_UTF8) meSslLibGetFunc(libHandle,"ASN1_STRING_to_UTF8")) == NULL) ||
       ((sslF_BIO_free = (meSSLF_BIO_free) meSslLibGetFunc(libHandle,"BIO_free")) == NULL) ||
       ((sslF_BIO_new = (meSSLF_BIO_new) meSslLibGetFunc(libHandle,"BIO_new")) == NULL) ||
       ((sslF_BIO_number_written = (meSSLF_BIO_number_written) meSslLibGetFunc(libHandle,"BIO_number_written")) == NULL) ||
       ((sslF_BIO_read = (meSSLF_BIO_read) meSslLibGetFunc(libHandle,"BIO_read")) == NULL) ||
       ((sslF_BIO_s_mem = (meSSLF_BIO_s_mem) meSslLibGetFunc(libHandle,"BIO_s_mem")) == NULL) ||
       ((sslF_CONF_modules_load_file = (meSSLF_CONF_modules_load_file) meSslLibGetFunc(libHandle,"CONF_modules_load_file")) == NULL) ||
       ((sslF_CRYPTO_free = (meSSLF_CRYPTO_free) meSslLibGetFunc(libHandle,"CRYPTO_free")) == NULL) ||
       ((sslF_ENGINE_load_builtin_engines = (meSSLF_ENGINE_load_builtin_engines) meSslLibGetFunc(libHandle,"ENGINE_load_builtin_engines")) == NULL) ||
       ((sslF_ERR_clear_error = (meSSLF_ERR_clear_error) meSslLibGetFunc(libHandle,"ERR_clear_error")) == NULL) ||
       ((sslF_ERR_error_string = (meSSLF_ERR_error_string) meSslLibGetFunc(libHandle,"ERR_error_string")) == NULL) ||
       ((sslF_ERR_get_error = (meSSLF_ERR_get_error) meSslLibGetFunc(libHandle,"ERR_get_error")) == NULL) ||
       ((sslF_GENERAL_NAME_free = (meSSLF_GENERAL_NAME_free) meSslLibGetFunc(libHandle,"GENERAL_NAME_free")) == NULL) ||
       ((sslF_OPENSSL_load_builtin_modules = (meSSLF_OPENSSL_load_builtin_modules) meSslLibGetFunc(libHandle,"OPENSSL_load_builtin_modules")) == NULL) ||
       ((sslF_OPENSSL_sk_num = (meSSLF_OPENSSL_sk_num) meSslLibGetFunc(libHandle,"OPENSSL_sk_num")) == NULL) ||
       ((sslF_OPENSSL_sk_pop_free = (meSSLF_OPENSSL_sk_pop_free) meSslLibGetFunc(libHandle,"OPENSSL_sk_pop_free")) == NULL) ||
       ((sslF_OPENSSL_sk_value = (meSSLF_OPENSSL_sk_value) meSslLibGetFunc(libHandle,"OPENSSL_sk_value")) == NULL) ||
       ((sslF_RAND_file_name = (meSSLF_RAND_file_name) meSslLibGetFunc(libHandle,"RAND_file_name")) == NULL) ||
       ((sslF_RAND_load_file = (meSSLF_RAND_load_file) meSslLibGetFunc(libHandle,"RAND_load_file")) == NULL) ||
       ((sslF_RAND_poll = (meSSLF_RAND_poll) meSslLibGetFunc(libHandle,"RAND_poll")) == NULL) ||
       ((sslF_RAND_status = (meSSLF_RAND_status) meSslLibGetFunc(libHandle,"RAND_status")) == NULL) ||
       ((sslF_X509_NAME_ENTRY_get_data = (meSSLF_X509_NAME_ENTRY_get_data) meSslLibGetFunc(libHandle,"X509_NAME_ENTRY_get_data")) == NULL) ||
       ((sslF_X509_NAME_get_entry = (meSSLF_X509_NAME_get_entry) meSslLibGetFunc(libHandle,"X509_NAME_get_entry")) == NULL) ||
       ((sslF_X509_NAME_get_index_by_NID = (meSSLF_X509_NAME_get_index_by_NID) meSslLibGetFunc(libHandle,"X509_NAME_get_index_by_NID")) == NULL) ||
       ((sslF_X509_NAME_get_text_by_NID = (meSSLF_X509_NAME_get_text_by_NID) meSslLibGetFunc(libHandle,"X509_NAME_get_text_by_NID")) == NULL) ||
       ((sslF_X509_NAME_oneline = (meSSLF_X509_NAME_oneline) meSslLibGetFunc(libHandle,"X509_NAME_oneline")) == NULL) ||
       ((sslF_X509_NAME_print_ex = (meSSLF_X509_NAME_print_ex) meSslLibGetFunc(libHandle,"X509_NAME_print_ex")) == NULL) ||
       ((sslF_X509_NAME_print_ex_fp = (meSSLF_X509_NAME_print_ex_fp) meSslLibGetFunc(libHandle,"X509_NAME_print_ex_fp")) == NULL) ||
       ((sslF_X509_STORE_add_cert = (meSSLF_X509_STORE_add_cert) meSslLibGetFunc(libHandle,"X509_STORE_add_cert")) == NULL) ||
       ((sslF_X509_VERIFY_PARAM_free = (meSSLF_X509_VERIFY_PARAM_free) meSslLibGetFunc(libHandle,"X509_VERIFY_PARAM_free")) == NULL) ||
       ((sslF_X509_VERIFY_PARAM_new = (meSSLF_X509_VERIFY_PARAM_new) meSslLibGetFunc(libHandle,"X509_VERIFY_PARAM_new")) == NULL) ||
       ((sslF_X509_VERIFY_PARAM_set_flags = (meSSLF_X509_VERIFY_PARAM_set_flags) meSslLibGetFunc(libHandle,"X509_VERIFY_PARAM_set_flags")) == NULL) ||
       ((sslF_X509_free = (meSSLF_X509_free) meSslLibGetFunc(libHandle,"X509_free")) == NULL) ||
       ((sslF_X509_get_ext_d2i = (meSSLF_X509_get_ext_d2i) meSslLibGetFunc(libHandle,"X509_get_ext_d2i")) == NULL) ||
       ((sslF_X509_get_issuer_name = (meSSLF_X509_get_issuer_name) meSslLibGetFunc(libHandle,"X509_get_issuer_name")) == NULL) ||
       ((sslF_X509_get_subject_name = (meSSLF_X509_get_subject_name) meSslLibGetFunc(libHandle,"X509_get_subject_name")) == NULL) ||
       ((sslF_X509_verify_cert_error_string = (meSSLF_X509_verify_cert_error_string) meSslLibGetFunc(libHandle,"X509_verify_cert_error_string")) == NULL) ||
       ((sslF_a2i_IPADDRESS = (meSSLF_a2i_IPADDRESS) meSslLibGetFunc(libHandle,"a2i_IPADDRESS")) == NULL) ||
       ((sslF_d2i_X509 = (meSSLF_d2i_X509) meSslLibGetFunc(libHandle,"d2i_X509")) == NULL))
    {
        snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Failed to load libcrypto functions (%d)",meSslGetError());
        if(logFunc != NULL)
            logFunc(buff,logData);
        return -2;
    }
          
#ifdef _WIN32
    libHandle = LoadLibrary(MESSL_STRINGIFY(_OPENSSLLNM));
#else
    libHandle = dlopen(MESSL_STRINGIFY(_OPENSSLLNM),RTLD_LOCAL|RTLD_LAZY);
#endif
    if(libHandle == NULL)
    {
        snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Failed to load " MESSL_STRINGIFY(_OPENSSLLNM) " library - OpenSSL installed? (%d)",meSslGetError());
        if(logFunc != NULL)
            logFunc(buff,logData);
        return -3;
    }
    if(((sslF_OPENSSL_init_ssl = (meSSLF_OPENSSL_init_ssl) meSslLibGetFunc(libHandle,"OPENSSL_init_ssl")) == NULL) ||
       ((sslF_SSL_CIPHER_get_name = (meSSLF_SSL_CIPHER_get_name) meSslLibGetFunc(libHandle,"SSL_CIPHER_get_name")) == NULL) ||
       ((sslF_SSL_CTX_ctrl = (meSSLF_SSL_CTX_ctrl) meSslLibGetFunc(libHandle,"SSL_CTX_ctrl")) == NULL) ||
       ((sslF_SSL_CTX_free = (meSSLF_SSL_CTX_free) meSslLibGetFunc(libHandle,"SSL_CTX_free")) == NULL) ||
       ((sslF_SSL_CTX_get_cert_store = (meSSLF_SSL_CTX_get_cert_store) meSslLibGetFunc(libHandle,"SSL_CTX_get_cert_store")) == NULL) ||
       ((sslF_SSL_CTX_load_verify_locations = (meSSLF_SSL_CTX_load_verify_locations) meSslLibGetFunc(libHandle,"SSL_CTX_load_verify_locations")) == NULL) ||
       ((sslF_SSL_CTX_new = (meSSLF_SSL_CTX_new) meSslLibGetFunc(libHandle,"SSL_CTX_new")) == NULL) ||
       ((sslF_SSL_CTX_set1_param = (meSSLF_SSL_CTX_set1_param) meSslLibGetFunc(libHandle,"SSL_CTX_set1_param")) == NULL) ||
       ((sslF_SSL_CTX_set_default_verify_paths = (meSSLF_SSL_CTX_set_default_verify_paths) meSslLibGetFunc(libHandle,"SSL_CTX_set_default_verify_paths")) == NULL) ||
       ((sslF_SSL_CTX_set_verify = (meSSLF_SSL_CTX_set_verify) meSslLibGetFunc(libHandle,"SSL_CTX_set_verify")) == NULL) ||
       ((sslF_SSL_connect = (meSSLF_SSL_connect) meSslLibGetFunc(libHandle,"SSL_connect")) == NULL) ||
       ((sslF_SSL_ctrl = (meSSLF_SSL_ctrl) meSslLibGetFunc(libHandle,"SSL_ctrl")) == NULL) ||
       ((sslF_SSL_free = (meSSLF_SSL_free) meSslLibGetFunc(libHandle,"SSL_free")) == NULL) ||
       ((sslF_SSL_get_current_cipher = (meSSLF_SSL_get_current_cipher) meSslLibGetFunc(libHandle,"SSL_get_current_cipher")) == NULL) ||
       ((sslF_SSL_get_error = (meSSLF_SSL_get_error) meSslLibGetFunc(libHandle,"SSL_get_error")) == NULL) ||
       ((sslF_SSL_get_peer_certificate = (meSSLF_SSL_get_peer_certificate) meSslLibGetFunc(libHandle,"SSL_get_peer_certificate")) == NULL) ||
       ((sslF_SSL_get_verify_result = (meSSLF_SSL_get_verify_result) meSslLibGetFunc(libHandle,"SSL_get_verify_result")) == NULL) ||
       ((sslF_SSL_get_version = (meSSLF_SSL_get_version) meSslLibGetFunc(libHandle,"SSL_get_version")) == NULL) ||
       ((sslF_SSL_new = (meSSLF_SSL_new) meSslLibGetFunc(libHandle,"SSL_new")) == NULL) ||
       ((sslF_SSL_read = (meSSLF_SSL_read) meSslLibGetFunc(libHandle,"SSL_read")) == NULL) ||
       ((sslF_SSL_set_fd = (meSSLF_SSL_set_fd) meSslLibGetFunc(libHandle,"SSL_set_fd")) == NULL) ||
       ((sslF_SSL_shutdown = (meSSLF_SSL_shutdown) meSslLibGetFunc(libHandle,"SSL_shutdown")) == NULL) ||
       ((sslF_SSL_write = (meSSLF_SSL_write) meSslLibGetFunc(libHandle,"SSL_write")) == NULL) ||
       ((sslF_TLS_client_method = (meSSLF_TLS_client_method) meSslLibGetFunc(libHandle,"TLS_client_method")) == NULL) ||
       0)
    {
        snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Failed to load libssl functions (%d)",meSslGetError());
        if(logFunc != NULL)
            logFunc(buff,logData);
        return -4;
    }
#endif        
       
    /* Init the PRNG.  If that fails, bail out.  */
    if(meSslInitPrng(buff) < 0)
    {
        meSslGetSSLErrors(buff);
        return -7;
    }
    
    MESSLFunc(OPENSSL_load_builtin_modules)();
#ifndef OPENSSL_NO_ENGINE
    MESSLFunc(ENGINE_load_builtin_engines)();
#endif
    MESSLFunc(CONF_modules_load_file)(NULL,NULL,CONF_MFLAGS_DEFAULT_SECTION|CONF_MFLAGS_IGNORE_MISSING_FILE);
    MESSLFunc(OPENSSL_init_ssl)(0, NULL);
    
    meth = MESSLFunc(TLS_client_method)();
    meSslCtx = MESSLFunc(SSL_CTX_new)(meth);
    if(!meSslCtx)
    {
        meSslGetSSLErrors(buff);
        return -8;
    }
    
    MESSLFunc(SSL_CTX_set_default_verify_paths)(meSslCtx);
    MESSLFunc(SSL_CTX_load_verify_locations)(meSslCtx, NULL,NULL);
#ifdef _WIN32
    // load windows system ca certs
    meSslInitSystemCaCert(meSslCtx,buff);
#endif
#ifdef X509_V_FLAG_PARTIAL_CHAIN
    /* Set X509_V_FLAG_PARTIAL_CHAIN to allow the client to anchor trust in
     * a non-self-signed certificate. This defies RFC 4158 (Path Building)
     * which defines a trust anchor in terms of a self-signed certificate.
     * However, it substantially reduces attack surface by pruning the tree
     * of unneeded trust points. For example, the cross-certified
     * Let's Encrypt X3 CA, which protects gnu.org and appears as an
     * intermediate CA to clients, can be used as a trust anchor without
     * the entire IdentTrust PKI.
     */
    param = MESSLFunc(X509_VERIFY_PARAM_new)();
    if (param)
    {
        /* We only want X509_V_FLAG_PARTIAL_CHAIN, but the OpenSSL docs
         * say to use X509_V_FLAG_TRUSTED_FIRST also. It looks like
         * X509_V_FLAG_TRUSTED_FIRST applies to a collection of trust
         * anchors and not a single trust anchor.
         */
        (void) MESSLFunc(X509_VERIFY_PARAM_set_flags)(param, X509_V_FLAG_TRUSTED_FIRST | X509_V_FLAG_PARTIAL_CHAIN);
        if(MESSLFunc(SSL_CTX_set1_param)(meSslCtx, param) == 0)
        {
            if(logFlags & meSSL_LOG_WARNING)
                logFunc((meUByte *) "meSSL Warning: Failed set trust to partial chain",logData);
        }
        /* We continue on error */
        MESSLFunc(X509_VERIFY_PARAM_free)(param);
    }
    else if(logFlags & meSSL_LOG_WARNING)
    {
        logFunc((meUByte *) "meSSL Warning: Failed to allocate verification param",logData);
        /* We continue on error */
    }
#endif
    
    /* SSL_VERIFY_NONE instructs OpenSSL not to abort SSL_connect if the certificate is invalid. We
     * verify the certificate separately in meSslCheckCertificate, which provides much better
     * diagnostics than examining the error stack after a failed SSL_connect. */
    MESSLFunc(SSL_CTX_set_verify)(meSslCtx, SSL_VERIFY_NONE, NULL);
    
    
    /* The OpenSSL library can handle renegotiations automatically, so tell it to do so. */
    MESSLFunc(SSL_CTX_ctrl)(meSslCtx,SSL_CTRL_MODE,SSL_MODE_AUTO_RETRY,NULL);
    
    return 0;
}

#define ASTERISK_EXCLUDES_DOT   /* mandated by rfc2818 */

/* Return true is STRING (case-insensitively) matches PATTERN, false otherwise. The recognized
 * wildcard character is "*", which matches any character in STRING except ".". Any number of the
 * "*" wildcard may be present in the pattern.
 * 
 * This is used to match of hosts as indicated in rfc2818:
 * 
 *      Names may contain the wildcard character * which is considered to match any single domain
 *      name component or component fragment. E.g., *.a.com matches foo.a.com but not bar.foo.a.com.
 *      f*.com matches foo.com but not bar.com [or foo.bar.com].
 * 
 * If the pattern contain no wildcards, pattern_match(a, b) is equivalent to !strcasecmp(a, b).
 */

static int
meSslPatternMatch(const char *pattern, const char *string)
{
    const char *p = pattern, *n = string;
    char c;
    for (; (c = tolower(*p++)) != '\0'; n++)
    {
        if (c == '*')
        {
            for (c = tolower(*p); c == '*'; c = tolower(*++p))
                ;
            for (; *n != '\0'; n++)
            {
                if (tolower(*n) == c && meSslPatternMatch(p, n))
                    return 1;
#ifdef ASTERISK_EXCLUDES_DOT
                else if (*n == '.')
                    return 0;
#endif
            }
            return (c == '\0');
        }
        else if (c != tolower(*n))
            return 0;
    }
    return *n == '\0';
}

static char *
meSslGetRfc2253FormattedName(X509_NAME *name)
{
    int len;
    char *out=NULL;
    BIO* b;
    
    if((b = MESSLFunc(BIO_new)(MESSLFunc(BIO_s_mem)())))
    {
        if((MESSLFunc(X509_NAME_print_ex)(b,name,0,XN_FLAG_RFC2253) >= 0) && 
           ((len = (int) MESSLFunc(BIO_number_written)(b)) > 0))
        {
            out = malloc(len+1);
            MESSLFunc(BIO_read)(b,out,len);
            out[len] = 0;
        }
        MESSLFunc(BIO_free)(b);
    }
    
    return (out == NULL) ? strdup(""):out;
}

int
meSslCheckCertificate(SSL *conn, const char *sniHost, meUByte *rbuff)
{
    X509 *cert;
    GENERAL_NAMES *subjectAltNames;
    char common_name[256];
    long vresult;
    int ret, alt_name_checked = 0;
    const char *severityLbl;

#if 0
    int pinsuccess = opt.pinnedpubkey == NULL;
    /* The user explicitly said to not check for the certificate.  */
    if ((logFlags == meSSL_IGN_CRT_ERR) && pinsuccess)
        return success;
#else
    if(logFlags == meSSL_IGN_CRT_ERR)
        /* don't error and no logging - so stop now */
        return 1;
#endif
    severityLbl = (logFlags & meSSL_IGN_CRT_ERR) ? "Warning":"Error";
        
    if((cert = MESSLFunc(SSL_get_peer_certificate)(conn)) == NULL)
    {
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL %s: No certificate presented by %s",severityLbl,sniHost);
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        return (logFlags & meSSL_IGN_CRT_ERR) ? 1:0;
    }
    
    if(logFlags & meSSL_LOG_VERBOSE)
    {
        char *subject = meSslGetRfc2253FormattedName(MESSLFunc(X509_get_subject_name)(cert));
        char *issuer = meSslGetRfc2253FormattedName(MESSLFunc(X509_get_issuer_name)(cert));
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Debug - Certificate:\n  subject: %s\n  issuer:  %s",subject,issuer);
        logFunc(rbuff,logData);
        free(subject);
        free(issuer);
    }
    
    ret = 1;
    vresult = MESSLFunc(SSL_get_verify_result)(conn);
    if (vresult != X509_V_OK)
    {
        /* TODO only warn if option to disable cert check */
        char *issuer = meSslGetRfc2253FormattedName(MESSLFunc(X509_get_issuer_name)(cert));
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL %s: Cannot verify %s's certificate, issued by %s:",severityLbl,sniHost,issuer);
        free(issuer);
        if(logFunc != NULL)
        {
            logFunc(rbuff,logData);
            /* Try to print more user-friendly (and translated) messages for
               the frequent verification errors.  */
            switch (vresult)
            {
            case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
                logFunc((meUByte *) "  Unable to locally verify the issuer's authority",logData);
                break;
            case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
                logFunc((meUByte *) "  Self-signed certificate encountered",logData);
                break;
            case X509_V_ERR_CERT_NOT_YET_VALID:
                logFunc((meUByte *) "  Issued certificate not yet valid",logData);
                break;
            case X509_V_ERR_CERT_HAS_EXPIRED:
                logFunc((meUByte *) "  Issued certificate has expired",logData);
                break;
            default:
                /* For the less frequent error strings, simply provide the OpenSSL error message. */
                /* TODO - this erases the prefered return string */
                snprintf((char *) rbuff,meSSL_BUFF_SIZE,"  %s", MESSLFunc(X509_verify_cert_error_string)(vresult));
                logFunc(rbuff,logData);
            }
        }
        ret = 0;
        /* Fall through, so that the user is warned about *all* issues
           with the cert (important with --no-check-certificate.)  */
    }
    
    /* Check that HOST matches the common name in the certificate. The following remains to be
     * done:
     * 
     * - When matching against common names, it should loop over all common names and choose the
     *   most specific one, i.e. the last one, not the first one, which the current code picks.
     * 
     * - Ensure that ASN1 strings from the certificate are encoded as UTF-8 which can be
     *   meaningfully compared to HOST.
     */
    if((subjectAltNames = MESSLFunc(X509_get_ext_d2i)(cert,NID_subject_alt_name,NULL,NULL)) != NULL)
    {
        /* Test subject alternative names */
        /* Do we want to check for dNSNames or ipAddresses (see RFC 2818)?
         * Signal it by host_in_octet_string. */
        ASN1_OCTET_STRING *host_in_octet_string = MESSLFunc(a2i_IPADDRESS)(sniHost);
        
        int numaltnames = MESSLFunc(sk_GENERAL_NAME_num)(subjectAltNames);
        int ii;
        for(ii=0; ii<numaltnames; ii++)
        {
            const GENERAL_NAME *name = MESSLFunc(sk_GENERAL_NAME_value)(subjectAltNames,ii);
            if (name)
            {
                if (host_in_octet_string)
                {
                    if (name->type == GEN_IPADD)
                    {
                        /* Check for ipAddress */
                        /* TODO: Should we convert between IPv4-mapped IPv6 addresses and IPv4 addresses? */
                        alt_name_checked = 1;
                        if (!MESSLFunc(ASN1_STRING_cmp)(host_in_octet_string,name->d.iPAddress))
                            break;
                    }
                }
                else if (name->type == GEN_DNS)
                {
                    /* dNSName should be IA5String (i.e. ASCII), however who does trust CA? Convert
                     * it into UTF-8 for sure. */
                    unsigned char *name_in_utf8 = NULL;
                    
                    /* Check for dNSName */
                    alt_name_checked = 1;
                    
                    if (0 <= MESSLFunc(ASN1_STRING_to_UTF8)(&name_in_utf8, name->d.dNSName))
                    {
                        /* Compare and check for NULL attack in ASN1_STRING */
                        if (meSslPatternMatch((char *) name_in_utf8,sniHost) &&
                            (strlen((char *) name_in_utf8) == (size_t) MESSLFunc(ASN1_STRING_length)(name->d.dNSName)))
                        {
                            MESSLFunc(CRYPTO_free)(name_in_utf8,OPENSSL_FILE,OPENSSL_LINE);
                            break;
                        }
                        MESSLFunc(CRYPTO_free)(name_in_utf8,OPENSSL_FILE,OPENSSL_LINE);
                    }
                }
            }
        }
        MESSLFunc(sk_GENERAL_NAME_pop_free)(subjectAltNames,MESSLFunc(GENERAL_NAME_free));
        if(host_in_octet_string)
            MESSLFunc(ASN1_OCTET_STRING_free)(host_in_octet_string);
        
        if((alt_name_checked == 1) && (ii >= numaltnames))
        {
            snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL %s: No certificate subject alternative name matches requested host name %s",severityLbl,sniHost);
            if(logFunc != NULL)
                logFunc(rbuff,logData);
            ret = 0;
        }
    }
    
    if (alt_name_checked == 0)
    {
        /* Test commomName */
        X509_NAME *xname = MESSLFunc(X509_get_subject_name)(cert);
        common_name[0] = '\0';
        MESSLFunc(X509_NAME_get_text_by_NID)(xname,NID_commonName,common_name,sizeof(common_name));
        
        if (!meSslPatternMatch(common_name,sniHost))
        {
            snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL %s: Certificate common name %s doesn't match requested host name %s",severityLbl,common_name,sniHost);
            if(logFunc != NULL)
                logFunc(rbuff,logData);
            ret = 0;
        }
        else
        {
            /* We now determine the length of the ASN1 string. If it differs from common_name's
             * length, then there is a \0 before the string terminates. This can be an instance of a
             * null-prefix attack.
             * 
             * https://www.blackhat.com/html/bh-usa-09/bh-usa-09-archives.html#Marlinspike
             */
            int ii=-1, jj;
            X509_NAME_ENTRY *xentry;
            ASN1_STRING *sdata;
            
            if(xname)
            {
                for (;;)
                {
                    jj = MESSLFunc(X509_NAME_get_index_by_NID)(xname, NID_commonName, ii);
                    if(jj == -1)
                        break;
                    ii = jj;
                }
            }
            
            xentry = MESSLFunc(X509_NAME_get_entry)(xname,ii);
            sdata = MESSLFunc(X509_NAME_ENTRY_get_data)(xentry);
            if (strlen(common_name) != (size_t) MESSLFunc(ASN1_STRING_length)(sdata))
            {
                snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL %s: Certificate common name is invalid (contains a NUL character).\n  This may be an indication that the host is not who it claims to be\n  (that is, it is not the real %s).",severityLbl,sniHost);
                if(logFunc != NULL)
                    logFunc(rbuff,logData);
                ret = 0;
            }
        }
    }
#if 0
    // TODO - believe this is for server with a specific public key (i.e. like ssh)
    pinsuccess = pkp_pin_peer_pubkey (cert, opt.pinnedpubkey);
    if (!pinsuccess)
    {
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"The public key for %s does not match pinned public key",sniHost);
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        ret = 0;
    }
#endif    
    
    if(ret && (logFlags & meSSL_LOG_VERBOSE))
    {
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Debug: X509 certificate successfully verified and matches host %s",sniHost);
        logFunc(rbuff,logData);
    }
    MESSLFunc(X509_free)(cert);
    
    return (logFlags & meSSL_IGN_CRT_ERR) ? 1:ret;
}

static int
meSslReadLine(meSslFile *sFp, meUByte *buff)
{
    meUByte *dd=buff, cc ;
    meInt ret, err;
    for(;;)
    {
        if((ret = MESSLFunc(SSL_read)(sFp->ssl,dd,1)) <= 0)
        {
            err = MESSLFunc(SSL_get_error)(sFp->ssl,ret);
            snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Failed to read line - %d,%d",ret,err);
            if(logFunc != NULL)
                logFunc(buff,logData);
            meSslGetSSLErrors(buff);
            return -1;
        }
        if((cc=*dd) == '\n')
            break ;
        if(cc != '\r')
            dd++;
    }
    *dd = '\0';
    return 1;
}

int
meSslSetup(meInt flags, meSslLogger logger,void *data, meUByte *rbuff)
{
    int ret;
    logFlags = flags;
    if((logFunc = logger) == NULL)
        logFlags &= ~(meSSL_LOG_WARNING|meSSL_LOG_HEADERS|meSSL_LOG_VERBOSE);
    logData = data;
    if((meSslCtx == NULL) && ((ret = meSslInit(rbuff)) < 0))
        return ret;
    return 0;
}

void
meSslSetProxy(meUByte *host, meInt port)
{
    if(proxyHost != NULL)
    {
        if((host == NULL) || strcmp((char *) host,(char *) proxyHost))
        {
            free(proxyHost);
            proxyHost = NULL;
        }
        else
            host = NULL;
    }
    if(host != NULL)
        proxyHost = (meUByte *) strdup((char *) host);
    proxyPort = port;
}


int
meSslOpen(meSslFile *sFp, meUByte *host, meInt port, meUByte *user, meUByte *pass, meUByte *file, meUByte *cookie, meUByte *postFName, meUInt rwflag, meUByte *rbuff)
{
    meUByte *thost, *ss, buff[meSSLBUFSIZ], tbuff[1024];
    struct hostent *hp;
    struct sockaddr_in sa;
    FILE *pfp=NULL;
    meSOCKET sck;
    SSL *ssl;
    meInt tport, pfs, ret, err;
    
    sFp->sck = meBadSocket;
    sFp->ssl = NULL;
    sFp->length = -1;
    if((meSslCtx == NULL) && ((ret = meSslInit(rbuff)) < 0))
        return ret;
    MESSLFunc(ERR_clear_error)();
    
    if(proxyHost != NULL)
    {
        thost = proxyHost ;
        tport = proxyPort ;
    }
    else
    {
        thost = host ;
        tport = port ;
    }
    
    if(((hp = gethostbyname((char *) thost)) == NULL) ||
       (hp->h_length > ((int) sizeof(struct in_addr))) ||
       (hp->h_addrtype != AF_INET))
    {
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: Failed to get host [%s] %d",thost,(hp == NULL) ? meSocketGetError():-1);
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        return -11;
    }
    memset(&sa,0,sizeof(sa)) ;
    sa.sin_family=AF_INET;
    if(tport)
        sa.sin_port=htons(tport);
    else
        sa.sin_port=htons(443);
    memcpy(&sa.sin_addr.s_addr,hp->h_addr,hp->h_length) ;
    
    if((ssl=MESSLFunc(SSL_new)(meSslCtx)) == NULL)
    {
        strcpy((char *) rbuff,"meSSL Error: Error creating SSL structure");
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        return -12;
    }
    sFp->ssl = ssl;
    
    meSsslHostnameGetSni((char *) thost,(char *) tbuff);
    if(!mSslIsValidIpAddress((char *) tbuff))
    {
        // SSL_set_tlsext_host_name(ssl,tbuff);
        long rc = MESSLFunc(SSL_ctrl)(ssl,SSL_CTRL_SET_TLSEXT_HOSTNAME,TLSEXT_NAMETYPE_host_name,(void *) tbuff);
        if(rc == 0)
        {
            MESSLFunc(SSL_free)(ssl);
            strcpy((char *) rbuff,"meSSL Error: Failed to set TLS server-name indication");
            if(logFunc != NULL)
                logFunc(rbuff,logData);
            return -13;
        }
    }
    
#if 0
    // is used in sftp
    if (continue_session)
    {
        /* attempt to resume a previous SSL session */
        ctx = (struct openssl_transport_context *) fd_transport_context (*continue_session);
        if (!ctx || !ctx->sess || !SSL_set_session (conn, ctx->sess))
            goto error;
    }
#endif
    
    if((sck=socket(AF_INET,SOCK_STREAM,0)) == meBadSocket)
    {
        strcpy((char *) rbuff,"meSSL Error: Failed to create socket");
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        MESSLFunc(SSL_free)(ssl);
        return -14;
    }  
    sFp->sck = sck;
    
    /*Bind the socket to the SSL structure*/
    if((ret = MESSLFunc(SSL_set_fd)(ssl,sck)) < 1)
    {
        err = MESSLFunc(SSL_get_error)(ssl,ret);
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: SSL_set_fd error %d,%d",ret,err);
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        meSslGetSSLErrors(buff);
        meSslClose(sFp,0);
        return -15;
    }
    
    if(logFlags & meSSL_LOG_VERBOSE)
        logFunc((meUByte *) "meSSL Debug: About to connect socket",logData);
    /* Connect to the server, TCP/IP layer,*/
    if((err=connect(sck,(struct sockaddr*) &sa,sizeof(sa))) != 0)
    {
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: Socket returned error #%d",meSocketGetError());
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        meSslClose(sFp,0);
        return -16;
    }
    
    
    /*Connect to the server, SSL layer.*/
    if((ret=MESSLFunc(SSL_connect)(ssl)) < 1)
    {
        err = MESSLFunc(SSL_get_error)(ssl,ret);
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: SSL_connect error %d,%d",ret,err);
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        meSslGetSSLErrors(buff);
        meSslClose(sFp,0);
        return -17;
    }
    
    /*Print out connection details*/
    if(logFlags & meSSL_LOG_HEADERS)
    {
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL connected on socket %x, version: %s, cipher: %s",sck,MESSLFunc(SSL_get_version)(ssl),
                  MESSLFunc(SSL_CIPHER_get_name)(MESSLFunc(SSL_get_current_cipher)(ssl)));
        logFunc(rbuff,logData);
    }
    if(!meSslCheckCertificate(ssl,(char *) tbuff,rbuff))
    {
        meSslClose(sFp,0);
        return -18;
    }

    /*Send message to the server.*/
    ss = buff;
    if(postFName != NULL)
    {
        if((pfp=fopen((char *) postFName,"rb")) == NULL)
        {
            snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: Failed to open post file [%s]",postFName);
            if(logFunc != NULL)
                logFunc(rbuff,logData);
            meSslClose(sFp,0);
            return -19;
        }
        /* send POST message to http */
        strcpy((char *) ss,"POST ") ;
        ss += 5;
    }
    else
    {
        /* send GET message to http */
        strcpy((char *) ss,"GET ") ;
        ss += 4;
    }
    if(proxyHost == NULL)
    {
        strcpy((char *) ss,(char *) file);
        ss += strlen((char *) ss) ;
    }
    else if(port)
        ss += sprintf((char *) ss,"https://%s:%d/%s",host,port,file);
    else
        ss += sprintf((char *) ss,"https://%s/%s",host,file);
    ss += sprintf((char *) ss," HTTP/1.0\r\nConnection: keep-alive\r\nHost: %s",host);
    if(port)
        ss += sprintf((char *) ss,":%d",port);
    if(user != NULL)
    {
        /* password supplied, encode */
        strcpy((char *) ss,"\r\nAuthorization: Basic ") ;
        ss += 23 ;
        sprintf((char *) tbuff,"%s:%s",user,pass);
        ss = strBase64Encode(ss,tbuff) ;
    }
    if(pfp != NULL)
    {
        meInt ii, ll;
        meUByte *s1, *s2 ;
        fseek(pfp,0,SEEK_END);
        ii = ftell(pfp) ;
        fseek(pfp,0,SEEK_SET) ;
        if((s1 = (meUByte *) strrchr((char *) postFName,'/')) == NULL)
            s1 = postFName;
        if((s2 = (meUByte *) strrchr((char *) s1,'\\')) == NULL)
            s2 = s1;
        pfs = sprintf((char *) tbuff,"\r\n----5Iz6dTINmxNFw6S42Ryf98IBXX1NCe%x",(meUInt) clock());
        ll = sprintf((char *) tbuff+pfs,"\r\nContent-Disposition: form-data; name=\"file\"; filename=\"%s\"\r\nContent-Type: application/octet-stream\r\n\r\n",s2);
        ss += sprintf((char *) ss,"\r\nContent-Length: %d",pfs-2+ll+ii+pfs+4);
    }
    if(cookie != NULL)
    {
        strcpy((char *) ss,"\r\nCookie: ");
        ss += 10;
        strcpy((char *) ss,(char *) cookie);
        ss += strlen((char *) ss);
    }
    if(logFlags & meSSL_LOG_HEADERS)
    {
        ss[0] = '\n';
        ss[1] = '\0';
        logFunc((meUByte *) "meSSL Request header:",logData);
        logFunc(buff,logData);
    }
    strcpy((char *) ss,"\r\n\r\n");
    ss += 4;
    if((ret=MESSLFunc(SSL_write)(ssl,buff,(int) (ss-buff))) <= 0)
    {
        err = MESSLFunc(SSL_get_error)(ssl,ret);
        snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: Write error %d,%d",ret,err);
        if(logFunc != NULL)
            logFunc(rbuff,logData);
        meSslGetSSLErrors(buff);
        /* err == 6 means the server issued an SSL_shutdown. */
        meSslClose(sFp,(err == 6));
        if(pfp != NULL)
            fclose(pfp);
        return -20;
    }
    if(pfp != NULL)
    {
        meInt ii;
        if((ret=MESSLFunc(SSL_write)(ssl,tbuff+2,strlen((char *) tbuff+2))) > 0)
        {
            while(((ii=fread(buff,1,meSSLBUFSIZ,pfp)) > 0) &&
                  ((ret=MESSLFunc(SSL_write)(ssl,buff,ii)) > 0))
                ;
            fclose(pfp);
            if(ret > 0)
            {
                strcpy((char *) tbuff+pfs,"--\r\n");
                ret = MESSLFunc(SSL_write)(ssl,tbuff,pfs+4);
            }
        }
        else
            fclose(pfp);
        if(ret <=0)
        {
            err = MESSLFunc(SSL_get_error)(ssl,ret);
            snprintf((char *) rbuff,meSSL_BUFF_SIZE,"meSSL Error: Post file write error %d,%d",ret,err);
            if(logFunc != NULL)
                logFunc(rbuff,logData);
            meSslGetSSLErrors(buff);
            /* err == 6 means the server issued an SSL_shutdown. */
            meSslClose(sFp,(err == 6));
            return -22;
        }
    }
    /* must read header getting now ditch the header, read up to the first blank line */
    if(logFlags & meSSL_LOG_HEADERS)
        logFunc((meUByte *) "meSSL Response header:",logData);
    err = 1;
    while(meSslReadLine(sFp,buff) > 0)
    {
        if(logFlags & meSSL_LOG_HEADERS)
            logFunc(buff,logData);
        if(buff[0] == '\0')
            return err;
        if(((buff[0] == 'C') && !strncmp((char *) buff+1,"ontent-Length:",14)) ||
           ((buff[0] == 'c') && !strncmp((char *) buff+1,"ontent-length:",14)))
            sFp->length = atoi((char *) buff+15) ;
        else if(((buff[0] == 'L') || (buff[0] == 'l')) && !strncmp((char *) buff+1,"ocation:",8))
        {
            /* The requested file is not here, its at the given location */
            char cc;
            ss = buff+9 ;
            while(((cc=*ss) == ' ') || (cc == '\t'))
                ss++ ;
            if(*ss != '\0')
            {
                strcpy((char *) rbuff,(char *) ss);
                /* note: carry on reading to validate the rest of the header and get updated cookies etc. */
                err = 0;
            }
        }
    }
    sFp->length = 0;
    return err;
}

int
meSslRead(meSslFile *sFp, int sz, meUByte *buff)
{
    int err,ret = MESSLFunc(SSL_read)(sFp->ssl,buff,sz);
    if(ret > 0)
        return ret;
    
    err = MESSLFunc(SSL_get_error)(sFp->ssl,ret);
    if((sFp->length == -1) && ((err == SSL_ERROR_SYSCALL) || (err == SSL_ERROR_ZERO_RETURN)))
    {
        /* Conent-lenth not returned and we've likely hit the correct end of the data - return 0 */ 
        if(logFlags & meSSL_LOG_VERBOSE)
            logFunc((meUByte *) "meSSL Debug: Got data stream end",logData);
        sFp->length = -2;
        return 0;
    }
    snprintf((char *) buff,meSSL_BUFF_SIZE,"meSSL Error: Read failure - %d,%d",ret,err);
    if(logFunc != NULL)
        logFunc(buff,logData);
    meSslGetSSLErrors(buff);
    return -1;
}

void
meSslClose(meSslFile *sFp, int shutDown)
{
    if(sFp->ssl != NULL)
    {
        if(shutDown)
        {
            int err = MESSLFunc(SSL_shutdown)(sFp->ssl);
            if(err < 0)
            {
                if(logFunc != NULL)
                    logFunc((meUByte *) "meSSL Error: Error in shutdown",logData);
            }
            else if((err == 1) && (logFlags & meSSL_LOG_VERBOSE))
                logFunc((meUByte *) "meSSL Debug: Client exited gracefully",logData);
        }
        MESSLFunc(SSL_free)(sFp->ssl);
        sFp->ssl = NULL;
    }
    if(sFp->sck != meBadSocket)
    {
        meSocketClose(sFp->sck);
        sFp->sck = meBadSocket;
    }
}

void
meSslEnd()
{
    if(meSslCtx != NULL)
    {
        MESSLFunc(SSL_CTX_free)(meSslCtx);
        meSslCtx = NULL;
    }
}
